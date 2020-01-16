#pragma once

#include <vector>

#include "ssfehash/prefetch.h"
#include "ssfehash/group.h"

// static void print_space_utilization(const char *algo, int bytes, int max_capacity) {
//     printf("%s: %.3lf bits/key, capacity: %d\n", algo, bytes*8.0 / max_capacity, max_capacity);
// }

const int SSFE_GROUP_BITS = 256;

template <typename KEY_TYPE>
class SSFE {
    typedef MixFamily3<KEY_TYPE, 8> HASH;
   public:
    SSFE() = default;
    SSFE(int max_capacity) {
        init(max_capacity);
    }
    ~SSFE() {
        delete[] data_;
        delete[] seed_;
    }

    // n is #bins (groups), m is #balls (keys)
    // see https://en.wikipedia.org/wiki/Balls_into_bins_problem
    static int epected_max_load(int n, int m) {
        return m/n + sqrt(m*log(n)/n) + 1;
    }

    static int round_capacity(int cap, int &group_num) {
        group_num = 1;
        while (epected_max_load(group_num, cap) > 232) {
            group_num *= 2;
        }

        //find the maximum capacity when keep the group_num unchanged 
        int l = cap, r = 1000*1000*1000;
        assert(l < r);
        while (l + 1 < r) {
            int mid = (l + r) / 2;
            if (epected_max_load(group_num, mid) <= 232) {
                l = mid;
            } else {
                r = mid;
            }
        }
        cap = l;

        assert(epected_max_load(cap, group_num) <= 232);
        return cap;
    }

    void init(int max_capacity) {
        assert(data_ == nullptr);

        max_capacity_ = round_capacity(max_capacity, group_num_);
        group_num_bitmask_ = group_num_ - 1;

        // the maintaince structure
        groups_.resize(group_num_);
        // reserve spaces to avoid frequent allocations
        for (auto &g: groups_) {
            g.reserve(256);
        }

        seed_ = new uint8_t[group_num_];
        assert(seed_ != nullptr);

        int data_size = group_num_ * (SSFE_GROUP_BITS/8);
        data_ = new uint8_t[data_size];
        assert(data_ != nullptr);

        //print_space_utilization("SSFE", data_size, max_capacity);
    }

    // get_space_usage returns a tuple (actual size, max capacity size)
    std::tuple<uint32_t, uint32_t> get_space_usage() {
        return std::make_tuple(uint32_t(group_num_ + group_num_ * (SSFE_GROUP_BITS/8)), uint32_t(max_capacity_ / 8));
    }

    void clear() {
        if (data_ != nullptr) {
            size_ = 0;
            max_capacity_ = 0;
            group_num_ = 0;
            group_num_bitmask_ = 0;
            groups_.resize(0);
            delete[] data_;
            delete[] seed_;
            data_ = nullptr;
            seed_ = nullptr;
        }
    }

    void build(const std::vector<std::pair<KEY_TYPE, bool>> &kvs) {
        size_ = kvs.size();

        for (const auto &kv : kvs) {
            int g = h_.hash1(kv.first) & group_num_bitmask_;
            groups_[g].push_back(kv);
        }

        for (int i = 0; i < group_num_; i++) {
            int seed = HashGroup::build<KEY_TYPE, HASH>(groups_[i], data_ + i*(SSFE_GROUP_BITS/8), SSFE_GROUP_BITS / 8, false);
            seed_[i] = seed;
            if (seed < 0) {
                printf("i = %d\n", i);
                printf("group size: %d\n", (int)groups_[i].size());
                fflush(stdout);
                assert(false);
            }
        }
    }

    // update is not thread-safe
    void update(KEY_TYPE key, bool value) {
        int g = h_.hash1(key) & group_num_bitmask_;

        // update value in the vector
        bool found = false;
        for (int i = 0; i < (int)groups_[g].size(); i++) {
            if (groups_[g][i].first == key) {
                groups_[g][i].second = value;
                found = true;
                break;
            }
        }

        // not found. insert it.
        if (!found) {
            assert(size_ < max_capacity_);
            size_ += 1;
            groups_[g].push_back(std::make_pair(key, value));
        }

        // rebuild the query structure
        int seed = HashGroup::build<KEY_TYPE, HASH>(groups_[g], data_ + g*(SSFE_GROUP_BITS/8), SSFE_GROUP_BITS/8, false);
        seed_[g] = seed;
        if (seed < 0) {
            printf("g = %d\n", g);
            printf("group size: %d\n", (int)groups_[g].size());
            assert(false);
        }
    }

    bool query(KEY_TYPE key) {
        int g = h_.hash1(key) & group_num_bitmask_;
        int offset = g*(SSFE_GROUP_BITS/8);
        uint8_t *d = data_ + offset; 
        prefetch0(seed_ + g);
        prefetch0(data_ + offset);
        return HashGroup::query_group_size_256<KEY_TYPE, HASH>(key, d, seed_[g]);
    }

    void query_batch(KEY_TYPE *keys, bool *res, int batch_size) {
        assert(batch_size <= 16);
        int g[16]; // group ids
        uint8_t hs[16][3];
        // compute the group ids
        for (int i = 0; i < batch_size; i++) {
            g[i] = h_.hash1(keys[i]) & group_num_bitmask_;
            // !!! imporant
            prefetch0(seed_ + g[i]);
        }
        // compute the indexes for each gorup
        for (int i = 0; i < batch_size; i++) {
            auto [h1, h2, h3] = h_.hash3(keys[i], seed_[g[i]]);
            hs[i][0] = h1 & 255;
            hs[i][1] = h2 & 255;
            hs[i][2] = h3 & 255;
            // !!! imporant
            prefetch0(data_ + g[i]*(SSFE_GROUP_BITS/8));
        }
        // compute the results
        for (int i = 0; i < batch_size; i++) {
            uint8_t *d = data_ + g[i]*(SSFE_GROUP_BITS/8);
            res[i] = get_bit(d, hs[i][0]) ^ get_bit(d, hs[i][1]) ^ get_bit(d, hs[i][2]);
        }
    }

   private:
    HASH h_;
    int group_num_;
    int group_num_bitmask_;
    int size_;
    int max_capacity_;
    std::vector<std::vector<std::pair<KEY_TYPE, bool>>> groups_;
    uint8_t* data_ = nullptr;
    uint8_t* seed_ = nullptr;
};

template <typename KEY_TYPE>
class SSFE_DONG {
    typedef MixFamily3<KEY_TYPE, 10> HASH;
   public:
    SSFE_DONG() = default;
    SSFE_DONG(int max_capacity) {
        init(max_capacity);
    }
    ~SSFE_DONG() {
        if (data_ != nullptr) {
            delete[] data_;
            delete[] groups_;
        }
    }

    void init(int max_capacity) {
        assert(data_ == nullptr);
        int avg_load = 256;

        // init groups related stuffs, groups_ ich is used to index the start postions of groups
        group_num_ = max_capacity / avg_load + 1;    
        groups_ = new uint8_t*[group_num_];

        // the maintaince struct, which stores kv pairs
        kv_groups_.resize(group_num_);
        for (auto &g: kv_groups_) {
            g.reserve(256);
        }

        // init data related stuffs, which is used as "bit array"-s of groups
        data_size_ = group_num_ * (avg_load*1.1/8 + 4);
        data_ = new uint8_t[data_size_];

        // print_space_utilization("SSFE_DONG", group_num_ + data_size_, max_capacity);
    }

    void clear() {
        if (data_ != nullptr) {
            data_size_ = 0;
            group_num_ = 0;
            delete[] data_;
            delete[] groups_;
            kv_groups_.resize(0);
            data_ = nullptr;
            groups_ = nullptr;
        }
    }

    void build(const std::vector<std::pair<KEY_TYPE, bool>> &kvs) {
        for (const auto &kv : kvs) {
            int g = h_.hash1(kv.first) % group_num_;
            kv_groups_[g].push_back(kv);
        }

        // The Group Format:
        // -------------------------------------------------------------
        // | len, 2 bytes | seed  1 bytes | data (len - 3) bytes |
        // -------------------------------------------------------------

        uint8_t *p = data_;
        for (int i = 0; i < group_num_; i++) {
            // setup the group start address
            groups_[i] = p;

            uint16_t len = 3 + (kv_groups_[i].size() * 1.1)/8 + 1; // 2 bytes for len, 1 bytes for seed, (groups[i].size() * 1.4)/8 + 1 for x values
            assert(p + len <= data_ + data_size_);
            
            // copy len
            memcpy(p, &len, sizeof(uint16_t));

            // the build function setups the seed and data
            int seed = HashGroup::build<KEY_TYPE, HASH>(kv_groups_[i], p + 2, len - 2);
            if (seed < 0) {
                printf("i = %d\n", i);
                printf("group size: %d\n", (int)kv_groups_[i].size());
                assert(false);
            }

            p += len;
        }
    }

    bool query(KEY_TYPE key) {
        int g = h_.hash1(key) % group_num_;
        uint8_t *group = groups_[g];

        uint16_t len = 0;
        memcpy(&len, group, sizeof(uint16_t));
        return HashGroup::query<KEY_TYPE, HASH>(key, group + 2, len - 2);
    }

    void query_batch(KEY_TYPE *keys, bool *res, int batch_size) {
        assert(batch_size <= 16);
        uint8_t *g[16];
        for (int i = 0; i < batch_size; i++) {
            g[i] = groups_[h_.hash1(keys[i]) % group_num_];
        }

        // prefetch data
        for (int i = 0; i < batch_size; i++) {
            prefetch0(g[i]);
        }

        // query
        for (int i = 0; i < batch_size; i++) {
            uint16_t len = 0;
            memcpy(&len, g[i], sizeof(uint16_t));
            res[i] = HashGroup::query<KEY_TYPE, HASH>(keys[i], g[i] + 2, len - 2);
        }
    }

   private:
    HASH h_;
    std::vector<std::vector<std::pair<KEY_TYPE, bool>>> kv_groups_;

    size_t data_size_;
    uint8_t* data_ = nullptr;

    int group_num_;
    uint8_t** groups_ = nullptr;
};