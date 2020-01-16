#include <gtest/gtest.h>

#include "ssfehash/ssfe.h"
#include "dev_utils/dev_utils.h"

TEST(SSFE, BuildWithTightSizes) {
    int group_num;
    int n = SSFE<uint64_t>::round_capacity(10*1000*1000, group_num);
    printf("n = %d group_num = %d utilization = %.2lf\n", n, group_num, group_num*256.0/n);
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(n);
    SSFE<uint64_t> ssfe(kvs.size());
    ssfe.build(kvs);
}

template<typename SSFE_T>
void test_ssfe() {
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(100000);
    SSFE_T ssfe(kvs.size());
    ssfe.build(kvs);

    // verify
    for (auto &kv : kvs) {
        EXPECT_EQ(kv.second, ssfe.query(kv.first));
    }

    // test batch query
    uint64_t keys[16];
    bool res[16];
    for (int i = 0; i < 16; ++i) {
        keys[i] = kvs[i].first;
    }
    ssfe.query_batch(keys, res, 16);
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(kvs[i].second, res[i]);
    }
}

TEST(SSFE, Basic) {
    test_ssfe<SSFE<uint64_t>>();
}

TEST(SSFE_DONG, Basic) {
    test_ssfe<SSFE_DONG<uint64_t>>();
}

TEST(SSFE, Update) {
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(100000);
    SSFE<uint64_t> ssfe(kvs.size());
    ssfe.build(kvs);

    // update
    for (auto &kv : kvs) {
        kv.second = bool(rand() % 2);
        ssfe.update(kv.first, kv.second);
    }

    // verify
    for (auto &kv : kvs) {
        EXPECT_EQ(kv.second, ssfe.query(kv.first));
    }
}