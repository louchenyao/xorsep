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
        std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(220);
        int data_size = 256 / 8;
        uint8_t *data = new uint8_t[data_size];
        int hash_index = 0;
        for (; hash_index < 256; hash_index++) {
            if (HashGroup::build_naive_<uint64_t, MixFamily<uint64_t>>(kvs, data + 1, data_size - 1, hash_index)) {
                break;
            }
        }
        assert(hash_index < 256);
        memset(data, 0, data_size);

        // build
        bool succ = HashGroup::build_bitset_2_<uint64_t, MixFamily<uint64_t>>(kvs, data + 1, data_size - 1, hash_index);
        EXPECT_EQ(succ, true);

        // verify
        data[0] = uint8_t(hash_index);
        for (auto &kv : kvs) {
            bool r = HashGroup::query<uint64_t, MixFamily<uint64_t>>(kv.first, data, data_size);
            EXPECT_EQ(kv.second, r);
        }

        delete[] data;
    }
}

TEST(HashGroup, WithoutStoringHashIndexInData) {
    printf("{");
    int tot_trails = 0;
    int tot_rounds = 2000;

    for (int round = 0; round < tot_rounds; round++) {
        // construct key-value pairs
        std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(220);

        // build the hash group
        uint8_t *data = new uint8_t[256 / 8];
        int hash_index = HashGroup::build<uint64_t, MixFamily<uint64_t>>(kvs, data, 256 / 8, false);
        assert(hash_index >= 0);
        
        if (round < 30) {
            printf("%d, ", hash_index + 1);
        } else if (round == 30) {
            printf("...");
        }
        
        
        tot_trails += hash_index + 1;

        // verify
        for (auto &kv : kvs) {
            bool r = HashGroup::query_group_size_256<uint64_t, MixFamily<uint64_t>>(kv.first, data, hash_index);
            EXPECT_EQ(kv.second, r);
        }
        delete[] data;
    }
    printf("} (avg: %.3lf) trails to find a hash index in MixFamily(WithoutStroingIndex)!\n", double(tot_trails)/tot_rounds);
}

template <class HASH_FAMILY>
void group_test(std::string name, bool verify=true) {
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
    group_test<Murmur3Family<uint64_t>>("Murmur3Family");
    group_test<MixFamily<uint64_t>>("MixFamily");
    group_test<CRC32Family<uint64_t>>("CRC32Family");
    group_test<FakeRandomFamily<uint64_t>>("FakeRandomFamily", false);
}