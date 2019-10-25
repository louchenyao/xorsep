#include <gtest/gtest.h>

#include "ssfehash/hash_family.h"
#include "xxhash.h"

template <class HASH_FAMILY>
void test_hash_family() {
    HASH_FAMILY h;

    int a = 123;
    uint32_t b = 123;
    int64_t c = 123;
    int d = 432;

    int mod = 9999999;

    EXPECT_EQ(h.hash(a, 42, mod), h.hash(b, 42, mod));
    EXPECT_EQ(h.hash(b, 42, mod), h.hash(c, 42, mod));
    EXPECT_NE(h.hash(c, 42, mod), h.hash(d, 42, mod));
}

TEST(CRC32Family, Basic) {
    test_hash_family<Murmur3Family<uint64_t> >();
    test_hash_family<MixFamily<long double> >();
    test_hash_family<CRC32Family<long double> >();
    test_hash_family<MixFamily<uint64_t> >();
    test_hash_family<CRC32Family<uint64_t> >();
}

TEST(xxHash, Basic) {
    uint64_t a = 233333333333333u;
    uint64_t b = 123;
    XXH32_hash_t h1 = XXH32(&a, sizeof(a), 0);
    XXH32_hash_t h2 = XXH32(&b, sizeof(b), 0);
    //printf("%d %d\n", h1, h2);
    EXPECT_NE(h1, h2);
}