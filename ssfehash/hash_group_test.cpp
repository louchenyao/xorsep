#include <gtest/gtest.h>

#include <set>

#include "ssfehash/hash_group.h"
#include "dev_utils/dev_utils.h"

TEST(HashGroup, BitManipulation) {
    uint8_t d = 16;
    set_bit(&d, 2, true);
    EXPECT_EQ(d, 20);
}

template <class HASH_FAMILY>
void hash_group_test(std::string name, bool verify=true) {
    printf("{");
    int tot_trails = 0;
    int tot_rounds = 2000;

    for (int round = 0; round < tot_rounds; round++) {
        // construct key-value pairs
        std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(220);

        // build the hash group
        uint8_t *data = new uint8_t[256 / 8];
        int family_index = HashGroup::build<uint64_t, HASH_FAMILY >(kvs, data, 256 / 8);
        assert(family_index >= 0);
        
        if (round < 30) {
            printf("%d, ", family_index + 1);
        } else if (round == 30) {
            printf("...");
        }
        
        
        tot_trails += family_index + 1;

        // verify
        if (verify) {
            for (auto &kv : kvs) {
                bool r = HashGroup::query<uint64_t, HASH_FAMILY >(kv.first, data, 256 / 8);
                EXPECT_EQ(kv.second, r);
            }
        }
        delete[] data;
    }
    printf("} (avg: %.3lf) trails to find a hash index in %s!\n", double(tot_trails)/tot_rounds, name.c_str());
}

TEST(HashGroup, Basic) {
    hash_group_test<MixFamily<uint64_t>>("MixFamily");
    hash_group_test<CRC32Family<uint64_t>>("CRC32Family");
    hash_group_test<FakeRandomFamily<uint64_t>>("FakeRandomFamily", false);
}