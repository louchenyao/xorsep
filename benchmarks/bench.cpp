#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdlib>
#include <random>
#include <map>
#include <vector>

#include "ssfehash/hash_family.h"
#include "ssfehash/ssfe.h"
#include "dev_utils/dev_utils.h"

static void BM_stdmap_query(benchmark::State& state) {
    // Setup
    auto kvs = construct_keyvalues(state.range(0));
    std::map<uint64_t, bool> m;
    for (const auto&kv : kvs) {
        m[kv.first] = kv.second;
    }

    shuffle_vector<std::pair<uint64_t, bool> >(kvs);
    
    // Benchmark
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(m[kvs[i].first]);
        i = (i + 1) % kvs.size();
    }
}

static void BM_ssfe_query(benchmark::State& state) {
    auto kvs = construct_keyvalues(state.range(0));
    SSFE<uint64_t> ssfe(kvs.size());
    ssfe.build(kvs);
    shuffle_vector<std::pair<uint64_t, bool> >(kvs);
    
    // Benchmark
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(ssfe.query(kvs[i].first));
        i = (i + 1) % kvs.size();
    }
}

static void BM_hash(benchmark::State& state) {
    std::vector<int> families;
    for (int i = 0; i < HASH_FAMILY_NUM; i++) {
        families.push_back(i);
    }
    shuffle_vector<int>(families);

    if (state.range(0) == 64) {
        auto h = HashFamily<uint64_t>();
        int i = 0;
        for (auto _ : state) {
            benchmark::DoNotOptimize(h.hash(42414242424u, families[i], 12345));
            i = (i + 1) % HASH_FAMILY_NUM;
        }
    } else if (state.range(0) == 128) {
        auto h = HashFamily<long double>();
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

BENCHMARK(BM_hash)->Arg(64)->Arg(128);
BENCHMARK(BM_ssfe_query)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK(BM_stdmap_query)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
