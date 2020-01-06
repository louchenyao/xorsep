#include <benchmark/benchmark.h>

#include "ssfehash/ssfe.h"
#include "othello_wrapper.h"
#include "dev_utils/dev_utils.h"
#include "dev_utils/perf_event_helper.h"

#ifdef __linux__
#include "sepset.h"
#endif


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
// SSFEBuildFixture generate a ssfe instance with given capacity
class SSFEBuildFixture : public benchmark::Fixture {
   public:
    void SetUp(const ::benchmark::State& state) {
        if (n != state.range(0)) {
            n = state.range(0);
            ssfe.clear();
            ssfe.init(state.range(0));
            auto kvs = generate_continous_keyvalues(state.range(0));
            ssfe.build(kvs);
        }
    }
    SSFE_T ssfe;
    int n = 0;
};

template <typename SSFE_T>
void benchmark_query(SSFE_T &ssfe, benchmark::State& state) {
    // Benchmark
    int i = 0;
    {
        PerfEventBenchamrkWrapper e(state);
        for (auto _ : state) {
            benchmark::DoNotOptimize(ssfe.query(i));
            i = i + 1;
            if (i == state.range(0)) {
                i = 0;
            }
        }
    }
}

template <class SSFE_T>
void benchmark_query_batch(SSFE_T &ssfe, benchmark::State& state) {
    // Benchmark
    int i = 0;
    while (state.KeepRunningBatch(16)) {
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

template <class SSFE_T>
void benchmark_update(SSFE_T &ssfe, benchmark::State& state) {
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

// ************
// * SSFE
// ************

// ssfe build
BENCHMARK_TEMPLATE(BM_ssfe_build, SSFE<uint64_t>)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000);

// ssfe query
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_query, SSFE<uint64_t>)(benchmark::State& state) {
    benchmark_query<SSFE<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_query)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000)->Arg(20 * 1000 * 1000)->Arg(50 * 1000 * 1000)->Arg(100 * 1000 * 1000)->Arg(200 * 1000 * 1000);

// ssfe query batch
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_query_batch, SSFE<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<SSFE<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_query_batch)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000)->Arg(20 * 1000 * 1000)->Arg(50 * 1000 * 1000)->Arg(100 * 1000 * 1000)->Arg(200 * 1000 * 1000);

// ssfe update
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_update, SSFE<uint64_t>)(benchmark::State& state) {
    benchmark_update<SSFE<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_update)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000);

// ************
// * Othello
// ************

// othello query
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, othello_query, OthelloWrapper<uint64_t>)(benchmark::State& state) {
    benchmark_query<OthelloWrapper<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, othello_query)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000)->Arg(20 * 1000 * 1000)->Arg(50 * 1000 * 1000)->Arg(100 * 1000 * 1000)->Arg(200 * 1000 * 1000);

// ************
// * SetSep
// ************

// sepset query
#ifdef __linux__
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, sepset_query, SepSet<uint64_t>)(benchmark::State& state) {
    benchmark_query<SepSet<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, sepset_query)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000)->Arg(20 * 1000 * 1000)->Arg(50 * 1000 * 1000)->Arg(100 * 1000 * 1000);


// sepset query batch
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, sepset_query_batch, SepSet<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<SepSet<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, sepset_query_batch)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000)->Arg(20 * 1000 * 1000)->Arg(50 * 1000 * 1000)->Arg(100 * 1000 * 1000)->Arg(200 * 1000 * 1000);


// sepset update
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, sepset_update, SepSet<uint64_t>)(benchmark::State& state) {
    benchmark_update<SepSet<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, sepset_update)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000);
// Error: void SepSet<KEY_TYPE>::init(int) [with KEY_TYPE = long unsigned int]: Assertion `table_ != nullptr' failed.
//BENCHMARK_REGISTER_F(SSFEBuildFixture, sepset_update)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*8)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*128)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*512)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*1024)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*4096);
#endif

// ************
// * SSFE Dong
// ************

// ssfe_dong query
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_dong_query, SSFE_DONG<uint64_t>)(benchmark::State& state) {
    benchmark_query<SSFE_DONG<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_dong_query)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000)->Arg(20 * 1000 * 1000);

// ssfe_dong query batch
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_dong_query_batch, SSFE_DONG<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<SSFE_DONG<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_dong_query_batch)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000)->Arg(20 * 1000 * 1000)->Arg(50 * 1000 * 1000)->Arg(100 * 1000 * 1000)->Arg(200 * 1000 * 1000);

// ssfe_dong build
BENCHMARK_TEMPLATE(BM_ssfe_build, SSFE_DONG<uint64_t>)->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000);

// ************
// * stdmap
// ************

// stdmap query
BENCHMARK(BM_stdmap_query)->Arg(1000)->Arg(1000 * 1000);