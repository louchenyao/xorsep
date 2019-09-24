#include <benchmark/benchmark.h>

#include "ssfehash/hash_group.h"
#include "dev_utils/dev_utils.h"


static void BM_build_naive_(benchmark::State& state) {
    std::vector<std::pair<uint64_t, bool>> kvs = construct_keyvalues((256 - 8) / 1.1 / 1.5);
    uint8_t *data = new uint8_t[256 / 8];

    // find a feasible hash function, use that to benchmark the equations solver
    int hash_family = HashGroup::build<uint64_t, MixFamily<uint64_t>>(kvs, data, 256 / 8);
    assert(hash_family >= 0);

    for (auto _ : state) {
        benchmark::DoNotOptimize(HashGroup::build_naive_<uint64_t, MixFamily<uint64_t>>(kvs, data, 256/8 - 1, hash_family));
    }

    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(kvs.size()));
    delete[] data;
}

BENCHMARK(BM_build_naive_);