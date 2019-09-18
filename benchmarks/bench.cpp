#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdlib>
#include <random>
#include <map>
#include <vector>

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

// Register the function as a benchmark
BENCHMARK(BM_stdmap)->Arg(10 * 1000)->Arg(100 * 1000)->Arg(1000 * 1000)->Arg(2 * 1000 * 1000);
// Run the benchmark
BENCHMARK_MAIN();