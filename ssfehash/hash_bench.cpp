#include <benchmark/benchmark.h>

#include "ssfehash/hash_family.h"
#include "dev_utils/dev_utils.h"

static void BM_hash(benchmark::State& state) {
    std::vector<int> families;
    for (int i = 0; i < HASH_FAMILY_NUM; i++) {
        families.push_back(i);
    }
    shuffle_vector<int>(families);

    if (state.range(0) == 64) {
        auto h = MixFamily<uint64_t>();
        int i = 0;
        for (auto _ : state) {
            benchmark::DoNotOptimize(h.hash(42414242424u, families[i], 12345));
            i = (i + 1) % HASH_FAMILY_NUM;
        }
    } else if (state.range(0) == 128) {
        auto h = MixFamily<long double>();
        assert(sizeof(long double) == 128/8);
        int i = 0;
        for (auto _ : state) {
            benchmark::DoNotOptimize(h.hash(42414242424u, families[i], 12345));
            i = (i + 1) % HASH_FAMILY_NUM;
        }
    } else {
        assert(false);
    }
}

static void BM_hash_once(benchmark::State& state) {
    if (state.range(0) == 64) {
        auto h = MixFamily<uint64_t>();
        int i = 23423431;
        for (auto _ : state) {
            i++;
            benchmark::DoNotOptimize(h.hash_once(i, 12345));
        }
    } else if (state.range(0) == 128) {
        auto h = MixFamily<long double>();
        for (auto _ : state) {
            benchmark::DoNotOptimize(h.hash_once(42414242424u, 12345));
        }
    } else {
        assert(false);
    }
}

BENCHMARK(BM_hash)->Arg(64)->Arg(128);
BENCHMARK(BM_hash_once)->Arg(64)->Arg(128);