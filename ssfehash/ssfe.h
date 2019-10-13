#pragma once

#include <vector>

#include "ssfehash/group.h"

// static void print_space_utilization(const char *algo, int bytes, int max_capacity) {
//     printf("%s: %.3lf bits/key, capacity: %d\n", algo, bytes*8.0 / max_capacity, max_capacity);
// }

const int SSFE_GROUP_BITS = 256;

template <typename KEY_TYPE>
class SSFE {
   public:
    SSFE(int max_capacity): max_capacity_(max_capacity) {
        int max_load = SSFE_GROUP_BITS / 1.5; // TODO (Chenyao): Try to optimize these constants.
        group_num_ = max_capacity / max_load + 1;

        for (int i = 1; ; i *= 2) {
            if (i >= group_num_) {
                group_num_ = i;
                break;
            }
        }
        group_num_bitmask_ = group_num_ - 1;

        groups_.resize(group_num_);
        hash_index_ = new uint8_t[group_num_];

        int data_size = group_num_ * (SSFE_GROUP_BITS/8);
        data_ = new uint8_t[data_size];

        //print_space_utilization("SSFE", data_size, max_capacity);
    }
    ~SSFE() {
        delete[] data_;
        delete[] hash_index_;
    }

    void build(const std::vector<std::pair<KEY_TYPE, bool>> &kvs) {
        size_ = kvs.size();

        for (const auto &kv : kvs) {
            int g = h_.hash_once(kv.first) & group_num_bitmask_;
            groups_[g].push_back(kv);
        }

        for (int i = 0; i < group_num_; i++) {
            int index = HashGroup::build<KEY_TYPE, MixFamily<KEY_TYPE> >(groups_[i], data_ + i*(SSFE_GROUP_BITS/8), SSFE_GROUP_BITS / 8, false);
            hash_index_[i] = index;
            if (index < 0) {
                printf("i = %d\n", i);
                printf("group size: %d\n", (int)groups_[i].size());
                assert(false);
            }
        }
    }

    // update is not thread-safe
    void update(KEY_TYPE key, bool value) {
        int g = h_.hash_once(key) & group_num_bitmask_;

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
        int index = HashGroup::build<KEY_TYPE, MixFamily<KEY_TYPE> >(groups_[g], data_ + g*(SSFE_GROUP_BITS/8), SSFE_GROUP_BITS/8, false);
        hash_index_[g] = index;
        if (index < 0) {
            printf("g = %d\n", g);
            printf("group size: %d\n", (int)groups_[g].size());
            assert(false);
        }
    }

    // bool query_v0(KEY_TYPE key) {
    //     int g = h_.hash_once(key, group_num_);
    //     return HashGroup::query<KEY_TYPE, MixFamily<KEY_TYPE> >(key, data_ + g*(SSFE_GROUP_BITS/8), SSFE_GROUP_BITS/8);
    // }

    // In contrast to query_v0, this function fetch hash index from hash_index_, not from data_.
    // Thus, it can prefetch data_ when computing hash.
    bool query(KEY_TYPE key) {
        int g = h_.hash_once(key) & group_num_bitmask_;
        int offset = g*(SSFE_GROUP_BITS/8);
        __builtin_prefetch(data_ + offset);
        return HashGroup::query_group_size_256<KEY_TYPE, MixFamily<KEY_TYPE> >(key, data_ +offset, hash_index_[g]);
    }

    void query_batch(KEY_TYPE *keys, bool *res, int batch_size) {
        assert(batch_size <= 16);
        int g[16];
        for (int i = 0; i < batch_size; i++) {
            g[i] = h_.hash_once(keys[i]) & group_num_bitmask_;
            __builtin_prefetch(data_ + g[i]*(SSFE_GROUP_BITS/8));
        }
        for (int i = 0; i < batch_size; i++) {
            res[i] = HashGroup::query_group_size_256<KEY_TYPE, MixFamily<KEY_TYPE> >(keys[i], data_ + g[i]*(SSFE_GROUP_BITS/8), hash_index_[g[i]]);
        }
    }

   private:
    MixFamily<KEY_TYPE> h_;
    int group_num_;
    int group_num_bitmask_;
    int size_;
    int max_capacity_;
    std::vector<std::vector<std::pair<KEY_TYPE, bool>>> groups_;
    uint8_t* data_;
    uint8_t* hash_index_;
};

template <typename KEY_TYPE>
class SSFE_DONG {
   public:
    SSFE_DONG(int max_capacity) {
        int avg_load = 256;

        group_num_ = max_capacity / avg_load;    
        groups_ = new uint8_t*[group_num_];

        data_size_ = group_num_ * (avg_load*1.42/8 + 3);
        data_ = new uint8_t[data_size_];

        // print_space_utilization("SSFE_DONG", group_num_ + data_size_, max_capacity);
    }
    ~SSFE_DONG() {
        delete[] data_;
        delete[] groups_;
    }

    void build(const std::vector<std::pair<KEY_TYPE, bool>> &kvs) {
        std::vector<std::pair<KEY_TYPE, bool>> groups[group_num_];
        for (const auto &kv : kvs) {
            int g = h_.hash_once(kv.first, group_num_);
            groups[g].push_back(kv);
        }

        // One Group:
        // -------------------------------------------------------------
        // | len, 2 bytes | hash_index  1 bytes | data (len - 3) bytes |
        // -------------------------------------------------------------

        uint8_t *p = data_;
        for (int i = 0; i < group_num_; i++) {
            // setup the group start address
            groups_[i] = p;

            uint16_t len = 3 + (groups[i].size() * 1.40)/8 + 1; // 2 bytes for len, 1 bytes for hash index, (groups[i].size() * 1.4)/8 + 1 for x values
            assert(p + len <= data_ + data_size_);
            
            // copy len
            memcpy(p, &len, sizeof(uint16_t));

            // the build function setups the hash_index and data
            int hash_index = HashGroup::build<KEY_TYPE, MixFamily<KEY_TYPE> >(groups[i], p + 2, len - 2);
            if (hash_index < 0) {
                printf("i = %d\n", i);
                printf("group size: %d\n", (int)groups[i].size());
                assert(false);
            }

            p += len;
        }
    }

    bool query(KEY_TYPE key) {
        int g = h_.hash_once(key, group_num_);
        uint8_t *group = groups_[g];

        uint16_t len = 0;
        memcpy(&len, group, sizeof(uint16_t));
        return HashGroup::query<KEY_TYPE, MixFamily<KEY_TYPE> >(key, group + 2, len - 2);
    }

    void query_batch(KEY_TYPE *keys, bool *res, int batch_size) {
        assert(batch_size <= 16);
        int g[16];
        for (int i = 0; i < batch_size; i++) {
            g[i] = h_.hash_once(keys[i], group_num_);
            __builtin_prefetch(groups_ + g[i]);
        }

        // prefetch data
        for (int i = 0; i < batch_size; i++) {
            __builtin_prefetch(groups_[g[i]]);
        }

        // query
        for (int i = 0; i < batch_size; i++) {
            uint8_t *group = groups_[g[i]];
            uint16_t len = 0;
            memcpy(&len, group, sizeof(uint16_t));
            res[i] = HashGroup::query<KEY_TYPE, MixFamily<KEY_TYPE> >(keys[i], group + 2, len - 2);
        }
    }

   private:
    MixFamily<KEY_TYPE> h_;

    size_t data_size_;
    uint8_t* data_;

    int group_num_;
    uint8_t** groups_;
};