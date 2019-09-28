#include <gtest/gtest.h>

#include "ssfehash/ssfe.h"
#include "dev_utils/dev_utils.h"

template<typename SSFE_T>
void test_ssfe() {
    std::vector<std::pair<uint64_t, bool>> kvs = construct_keyvalues(100000);
    SSFE_T ssfe(kvs.size());
    ssfe.build(kvs);

    // verify
    for (auto &kv : kvs) {
        EXPECT_EQ(kv.second, ssfe.query(kv.first));
    }
}

TEST(SSFE, Basic) {
    test_ssfe<SSFE<uint64_t>>();
    test_ssfe<SSFE_DONG<uint64_t>>();
}
