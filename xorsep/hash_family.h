#pragma once

#include <iostream> 
#include <tuple>
#include <random>
#include <MurmurHash3.h>
#include <nmmintrin.h>
#include "xxhash.h"

const int HASH_FAMILY_NUM = 256;

template <int k>
inline std::tuple<uint16_t, uint16_t, uint16_t> split_into_uintk(uint32_t h) {
    const uint32_t mask = (1<<k) - 1;
    return std::make_tuple<uint16_t, uint16_t, uint16_t>(h & mask, (h >> k) & mask, (h >> (2*k)) & mask);
}

template <typename KEY_TYPE>
class CRC32Family {
   public:
    CRC32Family() {
        // require the size of KEY_TYPE is the times of 64 bits
        assert(sizeof(KEY_TYPE) % 8 == 0);
    }
    std::tuple<uint16_t, uint16_t, uint16_t> hash3(KEY_TYPE key, int seed) {
        uint64_t h = seed;
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h = _mm_crc32_u64(h, t[i]);
        }
        return split_into_uintk<10>(h);
    }
    uint32_t hash1(KEY_TYPE key) {
        uint64_t h = 12191410945815747277u;
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h ^= _mm_crc32_u64(h, t[i]);
        }
        return h;
    }
};

template <typename KEY_TYPE, int k>
class MixFamily2 {
   public:
    MixFamily2() {
        // require the size of KEY_TYPE is the times of 64 bits
        assert(sizeof(KEY_TYPE) % 8 == 0);
    }

    std::tuple<uint16_t, uint16_t, uint16_t> hash3(KEY_TYPE key, int seed) {
        uint32_t h = XXH32((void *)&key, sizeof(KEY_TYPE), seed);
        return split_into_uintk<k>(h);
    }

    uint32_t hash1(KEY_TYPE key) {
        uint64_t h = 12191410945815747277u;
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h ^= _mm_crc32_u64(h, t[i]);
        }
        return h;
    }
};

template <typename KEY_TYPE, int k>
class MixFamily3 {
   public:
    MixFamily3() {
        // require the size of KEY_TYPE is the times of 64 bits
        assert(sizeof(KEY_TYPE) % 8 == 0);
    }
    std::tuple<uint16_t, uint16_t, uint16_t> hash3(KEY_TYPE key, int seed) {
        uint64_t h = seed;
        if (seed == 0) {
            uint64_t *t = (uint64_t *)&key;
            for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
                h = _mm_crc32_u64(h, t[i]);
            }
        } else {
            h = XXH32((void *)&key, sizeof(KEY_TYPE), seed);
        }
        return split_into_uintk<k>(h);
    }
    uint32_t hash1(KEY_TYPE key) {
        uint64_t h = 12191410945815747277u;
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h ^= _mm_crc32_u64(h, t[i]);
        }
        return h;
    }
};

template <typename KEY_TYPE>
class Murmur3Family {
   public:
    Murmur3Family() {
        // require the size of KEY_TYPE is the times of 32 bits
        assert(sizeof(KEY_TYPE) % 4 == 0);
    }
    
    std::tuple<uint16_t, uint16_t, uint16_t> hash3(KEY_TYPE key, int seed) {
        uint32_t h;
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), seed, &h);
        return split_into_uintk<10>(h);
    }

    uint32_t hash1(KEY_TYPE key) {
        uint32_t h;
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), 1445563897, &h);
        return h;
    }
};

template <typename KEY_TYPE>
class XXH32Family {
   public:
    XXH32Family() {
        // require the size of KEY_TYPE is the times of 32 bits
        assert(sizeof(KEY_TYPE) % 4 == 0);
    }

    std::tuple<uint16_t, uint16_t, uint16_t> hash3(KEY_TYPE key, int seed) {
        uint32_t h = XXH32((void *)&key, sizeof(KEY_TYPE), seed);
        return split_into_uintk<10>(h);
    }

    uint32_t hash1(KEY_TYPE key) {
        uint32_t h = XXH32((void *)&key, sizeof(KEY_TYPE), 1445563897);
        return h;
    }
};


template <typename KEY_TYPE>
class FakeRandomFamily {
   public:
    std::tuple<uint16_t, uint16_t, uint16_t> hash3(KEY_TYPE key, int seed) {
        (void)key;
        (void)seed;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dist(0, 0xffffffff);
        uint32_t h = dist(gen);
        return split_into_uintk<10>(h);
    }

    uint32_t hash1(KEY_TYPE key) {
        (void)key;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dist(0, 0xffffffff);
        uint32_t h = dist(gen);
        return h;
    }
};
