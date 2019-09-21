#include <gtest/gtest.h>

#include <set>

#include "ssfehash/ssfe.h"

TEST(SSFE, Basic) {
    // construct key-value pairs
    std::vector<std::pair<uint64_t, bool>> kvs;
    std::set<uint64_t> key_set;
    for (int i = 0; i < 100000; i++) {
        uint64_t k = rand();
        bool v = rand() % 2;
        if (key_set.find(k) != key_set.end()) {
            i--;
            continue;
        }

        key_set.insert(k);
        kvs.push_back(std::make_pair(k, v));
    }

    SSFE<uint64_t> ssfe(kvs.size());
    ssfe.build(kvs);

    // verify
    for (auto &kv : kvs) {
        EXPECT_EQ(kv.second, ssfe.query(kv.first));
    }
}