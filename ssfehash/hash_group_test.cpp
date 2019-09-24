#include <gtest/gtest.h>

#include <set>

#include "ssfehash/hash_group.h"
#include "dev_utils/dev_utils.h"

TEST(HashGroup, BitManipulation) {
    uint8_t d = 16;
    set_bit(&d, 2, true);
    EXPECT_EQ(d, 20);
}

TEST(HashGroup, Basic) {
    printf("{");
    for (int round = 0; round < 300; round++) {
        // construct key-value pairs
        std::vector<std::pair<uint64_t, bool>> kvs = construct_keyvalues(220);

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
    printf("} trails to find a hash index!\n");
}