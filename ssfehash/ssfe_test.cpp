#include <gtest/gtest.h>

#include "ssfehash/ssfe.h"
#include "dev_utils/dev_utils.h"

TEST(SSFE, BuildWithSpecificSizes) {
    // The total memory size of SSFE should be max_capacity*i. But it rounds up to 256*k, where k is the power of 2.
    // There we want to test if is the SSFE able to build when it doesn't round the group size up.
    for (double i = 1.3; i < 1.5; i += 0.01) {
        int n = double(1<<20) / i - 1;
        std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(n);
        SSFE<uint64_t> ssfe(kvs.size());
        ssfe.build(kvs);
    }
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