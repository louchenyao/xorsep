#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdlib>
#include <random>
#include <map>
#include <vector>

#include "ssfehash/hash_family.h"

static void BM_stdmap(benchmark::State& state) {
    // Setup
    std::vector<uint64_t> keys;
    std::map<uint64_t, bool> m;
    for (int i = 0; i < state.range(0); i++) {
        uint64_t k = std::rand();
        bool v = std::rand() % 2;
        m[k] = v;
        keys.push_back(k);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(keys.begin(), keys.end(), g);

    // Benchmark
    int i = 0;
    int keys_size = keys.size();
    for (auto _ : state) {
        benchmark::DoNotOptimize(m[i]);
        i = (i + 1) % keys_size;
    }
}

static void BM_hash_family(benchmark::State& state) {
    std::vector<int> families;
    for (int i = 0; i < HASH_FAMILY_NUM; i++) {
        families.push_back(i);
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(families.begin(), families.end(), g);

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

BENCHMARK(BM_hash_family)->Arg(64)->Arg(128);
BENCHMARK(BM_stdmap)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);

// Run the benchmark
BENCHMARK_MAIN();