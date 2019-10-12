#include <benchmark/benchmark.h>

#include <tuple>

#include "ssfehash/group.h"
#include "dev_utils/dev_utils.h"


std::tuple<std::vector<std::pair<uint64_t, bool>>, int> prepare() {
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues((256 - 8) / 1.1 / 1.5);
    
    uint8_t *data = new uint8_t[256 / 8];
    // find a feasible hash function, use that to benchmark the equations solver
    int hash_index = HashGroup::build<uint64_t, MixFamily<uint64_t>>(kvs, data, 256 / 8);
    assert(hash_index >= 0);
    delete[] data;

    return std::make_tuple(kvs, hash_index);
}

static void BM_build_naive_(benchmark::State& state) {
    auto [kvs, hash_index] = prepare();
    uint8_t *data = new uint8_t[256 / 8];
    for (auto _ : state) {
        benchmark::DoNotOptimize(HashGroup::build_naive_<uint64_t, MixFamily<uint64_t>>(kvs, data, 256/8 - 1, hash_index));
    }
    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(kvs.size()));
    delete[] data;
}

static void BM_build_bitset_(benchmark::State& state) {
    auto [kvs, hash_index] = prepare();
    uint8_t *data = new uint8_t[256 / 8];
    for (auto _ : state) {
        benchmark::DoNotOptimize(HashGroup::build_bitset_<uint64_t, MixFamily<uint64_t>>(kvs, data, 256/8 - 1, hash_index));
    }
    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(kvs.size()));
    delete[] data;
}

BENCHMARK(BM_build_naive_);
BENCHMARK(BM_build_bitset_);