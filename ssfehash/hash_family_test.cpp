#include <gtest/gtest.h>

#include "ssfehash/hash_family.h"

template <typename KEY_TYPE>
void test_hash_family() {
    HashFamily<KEY_TYPE> h;

    int a = 123;
    uint32_t b = 123;
    int64_t c = 123;
    int d = 432;

    int mod = 9999999;

    EXPECT_EQ(h.hash(a, 42, mod), h.hash(b, 42, mod));
    EXPECT_EQ(h.hash(b, 42, mod), h.hash(c, 42, mod));
    EXPECT_NE(h.hash(c, 42, mod), h.hash(d, 42, mod));
}

TEST(HashFamily, Basic) {
    test_hash_family<long double>();
    test_hash_family<uint64_t>();
}