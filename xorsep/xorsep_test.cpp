#include <gtest/gtest.h>

#include "xorsep/xorsep.h"
#include "dev_utils/dev_utils.h"

TEST(XorSep, BuildWithTightSizes) {
    int group_num;
    int n = XorSep<uint64_t>::round_capacity(10*1000*1000, group_num);
    printf("n = %d group_num = %d utilization = %.2lf\n", n, group_num, group_num*256.0/n);
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(n);
    XorSep<uint64_t> xorsep(kvs.size());
    xorsep.build(kvs);
}

template<typename T>
void test_xorsep() {
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(100000);
    T xorsep(kvs.size());
    xorsep.build(kvs);

    // verify
    for (auto &kv : kvs) {
        EXPECT_EQ(kv.second, xorsep.query(kv.first));
    }

    // test batch query
    uint64_t keys[16];
    bool res[16];
    for (int i = 0; i < 16; ++i) {
        keys[i] = kvs[i].first;
    }
    xorsep.query_batch(keys, res, 16);
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(kvs[i].second, res[i]);
    }
}

TEST(XorSep, Basic) {
    test_xorsep<XorSep<uint64_t>>();
}

TEST(XorSepDyn, Basic) {
    test_xorsep<XorSepDyn<uint64_t>>();
}

TEST(XorSep, Update) {
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(100000);
    XorSep<uint64_t> xorsep(kvs.size());
    xorsep.build(kvs);

    // update
    for (auto &kv : kvs) {
        kv.second = bool(rand() % 2);
        xorsep.update(kv.first, kv.second);
    }

    // verify
    for (auto &kv : kvs) {
        EXPECT_EQ(kv.second, xorsep.query(kv.first));
    }
}