#include <benchmark/benchmark.h>

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

static void BM_ssfe_build(benchmark::State& state) {
    auto kvs = construct_keyvalues(state.range(0));
    SSFE<uint64_t> ssfe(kvs.size());
    shuffle_vector<std::pair<uint64_t, bool> >(kvs);
    
    // Benchmark
    for (auto _ : state) {
        ssfe.build(kvs);
        // ensure the compiler won't optimize the build function
        benchmark::DoNotOptimize(ssfe.query(kvs[0].first));
    }

    // HACK, it is keys per second
    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(state.range(0)));
}

static void BM_query_prepare(benchmark::State& state) {
    auto kvs = construct_keyvalues(state.range(0));
    shuffle_vector<std::pair<uint64_t, bool> >(kvs);
    
    // Benchmark
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(kvs[i].first);
        i = (i + 1) % kvs.size();
    }
}

BENCHMARK(BM_ssfe_build)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK(BM_query_prepare)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK(BM_ssfe_query)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK(BM_stdmap_query)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
