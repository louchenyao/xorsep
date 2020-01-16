#include <gtest/gtest.h>

#include <set>

#include "ssfehash/group.h"
#include "dev_utils/dev_utils.h"

TEST(HashGroup, BitManipulation) {
    uint8_t d = 16;
    set_bit(&d, 2, true);
    EXPECT_EQ(d, 20);
}

TEST(HashGroup, build_bitset_2_) {
    for (int round = 0; round < 2000; round++) {
        // prepare data
        std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(220, false, round);
        int data_size = 256 / 8;
        uint8_t *data = new uint8_t[data_size];
        int seed = 0;
        for (; seed < 256; seed++) {
            if (HashGroup::build_naive_<uint64_t, MixFamily2<uint64_t>>(kvs, data + 1, data_size - 1, seed)) {
                break;
            }
        }
        assert(seed < 256);
        memset(data, 0, data_size);

        // build
        bool succ = HashGroup::build_bitset_2_<uint64_t, MixFamily2<uint64_t>>(kvs, data + 1, data_size - 1, seed);
        EXPECT_EQ(succ, true);

        // verify
        data[0] = uint8_t(seed);
        for (auto &kv : kvs) {
            bool r = HashGroup::query<uint64_t, MixFamily2<uint64_t>>(kv.first, data, data_size);
            EXPECT_EQ(kv.second, r);
        }

        delete[] data;
    }
}

template <class HASH_FAMILY>
void group_test(std::string name, bool verify=true, bool store_index_into_group_memory=true) {
    printf("{");
    int tot_trails = 0;
    int tot_rounds = 2000;

    for (int round = 0; round < tot_rounds; round++) {
        // construct key-value pairs
        std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(220, false, round);

        // build the hash group
        uint8_t *data = new uint8_t[256 / 8];
        int family_index = HashGroup::build<uint64_t, HASH_FAMILY >(kvs, data, 256 / 8, store_index_into_group_memory);
        
        // if (family_index < 0) {
        //     printf("failed!\n");
        //     continue;
        // }
        assert(family_index >= 0);
        
        if (round < 10) {
            printf("%d, ", family_index + 1);
        } else if (round == 10) {
            printf("...");
        }
        
        
        tot_trails += family_index + 1;

        // verify
        if (verify) {
            for (auto &kv : kvs) {
                bool r;
                if (!store_index_into_group_memory) {
                    r = HashGroup::query_group_size_256<uint64_t, HASH_FAMILY >(kv.first, data, family_index);
                } else {
                    r = HashGroup::query<uint64_t, HASH_FAMILY >(kv.first, data, 256 / 8);
                }
                EXPECT_EQ(kv.second, r);
            }
        }
        delete[] data;
    }
    printf("} (avg: %.3lf) trails to find a hash index in %s! store_index_into_group_memory = %d \n", double(tot_trails)/tot_rounds, name.c_str(), (int)store_index_into_group_memory);
}

TEST(HashGroup, Basic) {
    group_test<Murmur3Family<uint64_t>>("Murmur3Family", true, true);
    group_test<CRC32Family<uint64_t>>("CRC32Family", true, true);
    group_test<FakeRandomFamily<uint64_t>>("FakeRandomFamily", false, true);
    group_test<MixFamily2_256<uint64_t>>("MixFamily2", true, true);

    group_test<Murmur3Family<uint64_t>>("Murmur3Family", true, false);
    group_test<MixFamily2_256<uint64_t>>("MixFamily2_256", true, false);
    group_test<FakeRandomFamily<uint64_t>>("FakeRandomFamily", false, false);

    // CRC32 does not work when the group size is exactly 256, due to its special property
    // It does work when store_index_into_group_memory=true because the group size is actully 256 - 8 bits
    group_test<CRC32Family<uint64_t>>("CRC32Family", true, false);
}