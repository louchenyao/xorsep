#include <benchmark/benchmark.h>

#include "ssfehash/hash_family.h"
#include "dev_utils/dev_utils.h"

template <class HASH>
static void BM_hash3(benchmark::State& state) {
    std::vector<int> families;
    for (int i = 0; i < HASH_FAMILY_NUM; i++) {
        families.push_back(i);
    }
    shuffle_vector<int>(families);

    auto h = HASH();
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(h.hash3(42414242424u, families[i]));
        i = (i + 1) % HASH_FAMILY_NUM;
    }
}

template <class HASH>
static void BM_hash1(benchmark::State& state) {
    auto h = HASH();
    int i = 23423431;
    for (auto _ : state) {
        i++;
        benchmark::DoNotOptimize(h.hash1(i));
    }
}

BENCHMARK_TEMPLATE(BM_hash3, CRC32Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash3, MixFamily<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash3, Murmur3Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash3, XXH32Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash3, MixFamily256<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash3, MixFamily2_256<uint64_t>);

BENCHMARK_TEMPLATE(BM_hash1, CRC32Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash1, MixFamily<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash1, Murmur3Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash1, XXH32Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash1, MixFamily256<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash1, MixFamily2_256<uint64_t>);
