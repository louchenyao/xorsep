#include <benchmark/benchmark.h>
#include <cstdlib>
#include <string>

#include "ssfehash/ssfe.h"
#include "othello_wrapper.h"
#include "dev_utils/dev_utils.h"
#include "dev_utils/perf_event_helper.h"

#ifdef __linux__
#include "sepset.h"
#endif


static void BM_stdmap_query(benchmark::State& state) {
    // Setup
    auto kvs = generate_keyvalues(state.range(0));
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
            auto kvs = generate_keyvalues(state.range(0));
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
    auto kvs = generate_keyvalues(state.range(0));
    
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
// * Aarguments
// ************

static bool enough_memory() {
    return std::getenv("ENOUGH_MEMORY") != nullptr && std::string(std::getenv("ENOUGH_MEMORY")) == "1";
}

static void args_10m(benchmark::internal::Benchmark* b) {
    b->Arg(1000)->Arg(1000 * 1000)->Arg(10 * 1000 * 1000);
}

static void args_50m(benchmark::internal::Benchmark* b) {
    b->Apply(args_10m)->Arg(20 * 1000 * 1000)->Arg(50 * 1000 * 1000);
}
static void args_200m(benchmark::internal::Benchmark* b) {
    b->Apply(args_50m);
    if (enough_memory()) {
        b->Arg(100 * 1000 * 1000)->Arg(200 * 1000 * 1000);
    }
}

// ************
// * SSFE
// ************

static int ssfe_round_capacity(int cap) {
    int group_num = 0;
    return SSFE<uint64_t>::round_capacity(cap, group_num);
}

static void ssfe_args(benchmark::internal::Benchmark* b) {
    std::vector<int> small_args = {1000, 1000*1000, 10*1000*1000, 20*1000*1000, 40*1000*1000};
    std::vector<int> big_args = {80*1000*1000, 160*1000*1000};
    
    for (auto c: small_args) {
        b->Arg(ssfe_round_capacity(c));
    }

    if (enough_memory()) {
        for (auto c: big_args) {
            b->Arg(ssfe_round_capacity(c));
        }
    }
}

// ssfe build
BENCHMARK_TEMPLATE(BM_ssfe_build, SSFE<uint64_t>)->Apply(args_10m);

// ssfe query
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_query, SSFE<uint64_t>)(benchmark::State& state) {
    benchmark_query<SSFE<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_query)->Apply(ssfe_args);

// ssfe query batch
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_query_batch, SSFE<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<SSFE<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_query_batch)->Apply(ssfe_args);

// ssfe update
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_update, SSFE<uint64_t>)(benchmark::State& state) {
    benchmark_update<SSFE<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_update)->Apply(args_10m);

// ************
// * Othello
// ************

// othello query
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, othello_query, OthelloWrapper<uint64_t>)(benchmark::State& state) {
    benchmark_query<OthelloWrapper<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, othello_query)->Apply(args_200m);

// ************
// * SetSep
// ************

// sepset query
#ifdef __linux__
static void setsep_query_args(benchmark::internal::Benchmark *b) {
    // if the machine has enough memory, then run with 50m keys, otherwise 10m keys
    if (enough_memory()) {
        b->Apply(args_50m);
    } else {
        b->Apply(args_10m);
    }
}

BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, sepset_query, SepSet<uint64_t>)(benchmark::State& state) {
    benchmark_query<SepSet<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, sepset_query)->Apply(setsep_query_args);

// sepset query batch
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, sepset_query_batch, SepSet<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<SepSet<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, sepset_query_batch)->Apply(setsep_query_args);

// sepset update
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, sepset_update, SepSet<uint64_t>)(benchmark::State& state) {
    benchmark_update<SepSet<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, sepset_update)->Apply(args_10m);
//BENCHMARK_REGISTER_F(SSFEBuildFixture, sepset_update)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*8)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*128)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*512)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*1024)->Arg(EFD_TARGET_GROUP_NUM_RULES*64*4096);
#endif

// ************
// * SSFE Dong
// ************

// ssfe_dong query
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_dong_query, SSFE_DONG<uint64_t>)(benchmark::State& state) {
    benchmark_query<SSFE_DONG<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_dong_query)->Apply(args_50m);

// ssfe_dong query batch
BENCHMARK_TEMPLATE_DEFINE_F(SSFEBuildFixture, ssfe_dong_query_batch, SSFE_DONG<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<SSFE_DONG<uint64_t>>(ssfe, state);
}
BENCHMARK_REGISTER_F(SSFEBuildFixture, ssfe_dong_query_batch)->Apply(args_200m);

// ssfe_dong build
BENCHMARK_TEMPLATE(BM_ssfe_build, SSFE_DONG<uint64_t>)->Apply(args_10m);

// ************
// * stdmap
// ************

// stdmap query
BENCHMARK(BM_stdmap_query)->Arg(1000)->Arg(1000 * 1000);