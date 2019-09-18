#include <gtest/gtest.h>

#include "ssfehash/hash_group.h"

TEST(HashGroup, Basic) {
    // construct key-value pairs
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> d(1,100000000);
    std::vector<std::pair<uint64_t, bool>> kvs;
    for (int i = 0; i < 256; i++) {
        uint64_t k = d(rng);
        bool v = d(rng) % 2;
        kvs.push_back(std::make_pair(k, v));
    }

    // build the hash group
    HashGroup<uint64_t> hg(kvs, 1.1);

    // verify
    for (auto &kv : kvs) {
        EXPECT_EQ(kv.second, hg.query(kv.first));
    }
}