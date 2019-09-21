#include <gtest/gtest.h>

#include <set>

#include "ssfehash/hash_group.h"

TEST(HashGroup, BitManipulation) {
    uint8_t d = 16;
    set_bit(&d, 2, true);
    EXPECT_EQ(d, 20);
}

TEST(HashGroup, Basic) {
    printf("{");
    for (int round = 0; round < 300; round++) {
        // construct key-value pairs
        std::vector<std::pair<uint64_t, bool>> kvs;
        std::set<uint64_t> key_set;
        for (int i = 0; i < 220; i++) {
            uint64_t k = rand();
            bool v = rand() % 2;
            if (key_set.find(k) != key_set.end()) {
                i--;
                continue;
            }

            key_set.insert(k);
            kvs.push_back(std::make_pair(k, v));
        }

        // build the hash group
        uint8_t *data = new uint8_t[256 / 8];
        int hash_family = HashGroup::build<uint64_t>(kvs, data, 256 / 8);
        assert(hash_family >= 0);
        printf("%d, ", hash_family + 1);

        // verify
        for (auto &kv : kvs) {
            EXPECT_EQ(kv.second, HashGroup::query(kv.first, data, 256 / 8));
        }

        delete[] data;
    }
    printf("} trails to find a hash family!\n");
}