#include <gtest/gtest.h>

#include "ssfehash/ssfe.h"
#include "dev_utils/dev_utils.h"

TEST(SSFE, Basic) {
    std::vector<std::pair<uint64_t, bool>> kvs = construct_keyvalues(100000);
    SSFE<uint64_t> ssfe(kvs.size());
    ssfe.build(kvs);

    // verify
    for (auto &kv : kvs) {
        EXPECT_EQ(kv.second, ssfe.query(kv.first));
    }
}