#pragma once

#include <vector>

#include "ssfehash/hash_group.h"

template <typename KEY_TYPE>
class SSFE {
   public:
    SSFE(int max_capacity) {
        int avg_load= (256-8) / 1.1 / 1.5; // TODO (Chenyao): Try to optimize these constants.
        group_num_ = max_capacity / avg_load + 1;
        data_ = new uint8_t[group_num_ * 256];
    }
    ~SSFE() {
        delete[] data_;
    }

    void build(const std::vector<std::pair<KEY_TYPE, bool>> &kvs) {
        std::vector<std::pair<KEY_TYPE, bool>> groups[group_num_];
        for (const auto &kv : kvs) {
            int g = h_.hash_once(kv.first, group_num_);
            groups[g].push_back(kv);
        }

        for (int i = 0; i < group_num_; i++) {
            int hash_family = HashGroup::build<KEY_TYPE>(groups[i], data_ + i*256, 256 / 8);
            if (hash_family < 0) {
                printf("group size: %d\n", (int)groups[i].size());
                assert(false);
            }
        }
    }

    bool query(KEY_TYPE key) {
        int g = h_.hash_once(key, group_num_);
        return HashGroup::query<KEY_TYPE>(key, data_ + g*256, 256 / 8);
    }

   private:
    HashFamily<KEY_TYPE> h_;
    int group_num_;
    uint8_t* data_;
};