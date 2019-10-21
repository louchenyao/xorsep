#include <benchmark/benchmark.h>

#include "ssfehash/ssfe.h"
#include "dev_utils/dev_utils.h"

#include "PerfEvent.hpp"

// BM_stdmap_query benchmarks the time of acquiring the equery key.
// The result shows it consumes about 8 ns. I guess the overhead is from the extream long vector.
// Thus, we set keys to [0, n), then use the conuter as the key for each iteration.

static void BM_stdmap_query(benchmark::State& state) {
    // Setup
    auto kvs = generate_continous_keyvalues(state.range(0));
    std::map<uint64_t, bool> m;
    for (const auto&kv : kvs) {
        m[kv.first] = kv.second;
    }

    // Benchmark
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(m[i]);
        i += 1;
        if (i == state.range(0)) {
            i = 0;
        }
    }
}

template <class SSFE_T>
static void BM_ssfe_query(benchmark::State& state) {
    auto kvs = generate_continous_keyvalues(state.range(0));
    SSFE_T ssfe(kvs.size());
    ssfe.build(kvs);
    
    // Benchmark
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(ssfe.query(i));
        i = i + 1;
        if (i == state.range(0)) {
            i = 0;
        }
    }

    // start to run perf_event
    PerfEvent e;
    int normalizedConstant = state.iterations();
    e.startCounters();
    i = 0;
    for (int cnt = 0; cnt < normalizedConstant; cnt++) {
        benchmark::DoNotOptimize(ssfe.query(i));
        i = i + 1;
        if (i == state.range(0)) {
            i = 0;
        }
    }
    e.stopCounters();
    state.counters["cycles"] = e.getCounter("cycles") / normalizedConstant;
    state.counters["instructions"] = e.getCounter("instructions") / normalizedConstant;
    state.counters["L1-misses"] = e.getCounter("L1-misses") / normalizedConstant;
    state.counters["LLC-misses"] = e.getCounter("LLC-misses") / normalizedConstant;
    state.counters["branch-misses"] = e.getCounter("branch-misses") / normalizedConstant;
    state.counters["task-clock"] = e.getCounter("task-clock") / normalizedConstant;
    state.counters["IPC"] = state.counters["instructions"] / state.counters["cycles"];
    state.counters["CPUs"] = state.counters["task-clock"] / (e.getDuration()*1e9);
    state.counters["GHz"] = state.counters["cycles"] / state.counters["task-clock"];
}

template <class SSFE_T>
static void BM_ssfe_query_batch(benchmark::State& state) {
    auto kvs = generate_continous_keyvalues(state.range(0));
    SSFE_T ssfe(kvs.size());
    ssfe.build(kvs);
    
    // Benchmark
    int i = 0;
    for (auto _ : state) {
        uint64_t keys[16];
        bool res[16];
        for (int j = 0; j < 16; ++j) {
            keys[j] = i;
            i += 1;
        }
        if (i + 16 >= state.range(0)) {
            i = 0;
        }
        ssfe.query_batch(keys, res, 16);
        benchmark::DoNotOptimize(res[0]);
    }
}

template <class SSFE_T>
static void BM_ssfe_build(benchmark::State& state) {
    auto kvs = generate_continous_keyvalues(state.range(0));
    
    // Benchmark
    for (auto _ : state) {
        SSFE_T ssfe(kvs.size());
        ssfe.build(kvs);
        // ensure the compiler won't optimize the build function
        benchmark::DoNotOptimize(ssfe.query(kvs[0].first));
    }

    state.SetItemsProcessed(int64_t(state.iterations()) *
                            int64_t(state.range(0)));
}

static void BM_ssfe_update(benchmark::State& state) {
    auto kvs = generate_continous_keyvalues(state.range(0));
    SSFE<uint64_t> ssfe(kvs.size());
    ssfe.build(kvs);

    int i = 0;

    // Benchmark
    for (auto _ : state) {
        ssfe.update(i, i % 2);

        i = i + 1;
        if (i == state.range(0)) {
            i = 0;
        }
    }

    benchmark::DoNotOptimize(ssfe.query(0));
}

// SSFE benchmarks
BENCHMARK_TEMPLATE(BM_ssfe_query, SSFE<uint64_t>)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK_TEMPLATE(BM_ssfe_query_batch, SSFE<uint64_t>)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK_TEMPLATE(BM_ssfe_build, SSFE<uint64_t>)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK(BM_ssfe_update)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);

// SSFE_DONG benchmarks
BENCHMARK_TEMPLATE(BM_ssfe_query_batch, SSFE_DONG<uint64_t>)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK_TEMPLATE(BM_ssfe_build, SSFE_DONG<uint64_t>)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
BENCHMARK_TEMPLATE(BM_ssfe_query, SSFE_DONG<uint64_t>)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);

// stdmap benchmarks
BENCHMARK(BM_stdmap_query)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
