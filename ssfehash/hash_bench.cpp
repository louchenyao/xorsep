#include <benchmark/benchmark.h>

#include "ssfehash/hash_family.h"
#include "dev_utils/dev_utils.h"

template <class HASH>
static void BM_hash(benchmark::State& state) {
    std::vector<int> families;
    for (int i = 0; i < HASH_FAMILY_NUM; i++) {
        families.push_back(i);
    }
    shuffle_vector<int>(families);

    auto h = HASH();
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(h.hash(42414242424u, families[i], 12345));
        i = (i + 1) % HASH_FAMILY_NUM;
    }
}

template <class HASH>
static void BM_hash_once(benchmark::State& state) {
    auto h = HASH();
    int i = 23423431;
    for (auto _ : state) {
        i++;
        benchmark::DoNotOptimize(h.hash_once(i, 12345));
    }
}

BENCHMARK_TEMPLATE(BM_hash, CRC32Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash, MixFamily<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash, Murmur3Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash, XXH32Family<uint64_t>);

BENCHMARK_TEMPLATE(BM_hash_once, CRC32Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash_once, MixFamily<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash_once, Murmur3Family<uint64_t>);
BENCHMARK_TEMPLATE(BM_hash_once, XXH32Family<uint64_t>);