#include <benchmark/benchmark.h>
#include <cstdlib>
#include <string>

#include "xorsep/xorsep.h"
#include "othello_wrapper.h"
#include "dev_utils/dev_utils.h"
#include "dev_utils/perf_event_helper.h"

#ifdef __linux__
#include "setsep.h"
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

template <class T>
// BuildFixture generate a separator instance with the given capacity
class BuildFixture : public benchmark::Fixture {
   public:
    void SetUp(const ::benchmark::State& state) {
        if (n != state.range(0)) {
            n = state.range(0);
            sep.clear();
            sep.init(state.range(0));
            kvs = generate_keyvalues(state.range(0));
            sep.build(kvs);
        }
    }
    T sep;
    std::vector<std::pair<uint64_t, bool>> kvs;
    int n = 0;
};

template <typename T>
void benchmark_query(T &sep, std::vector<std::pair<uint64_t, bool>> &kvs, benchmark::State& state) {
    // Benchmark
    int i = 0;
    {
        PerfEventBenchamrkWrapper e(state);
        for (auto _ : state) {
            benchmark::DoNotOptimize(sep.query(kvs[i].first));
            i = i + 1;
            if (i == state.range(0)) {
                i = 0;
            }
        }
    }
}

template <class T>
void benchmark_query_batch(T &sep, std::vector<std::pair<uint64_t, bool>> &kvs, benchmark::State& state) {
    // Benchmark
    int i = 0;
    {
        PerfEventBenchamrkWrapper e(state);
        while (state.KeepRunningBatch(16)) {
            uint64_t keys[16];
            bool res[16];
            for (int j = 0; j < 16; ++j) {
                keys[j] = kvs[i].first;
                i += 1;
            }
            if (i + 16 >= state.range(0)) {
                i = 0;
            }
            sep.query_batch(keys, res, 16);
            benchmark::DoNotOptimize(res[0]);
        }
    }
}

template <class T>
static void BM_build(benchmark::State& state) {
    auto kvs = generate_keyvalues(state.range(0));
    
    // Benchmark
    for (auto _ : state) {
        T sep(kvs.size());
        sep.build(kvs);
        // ensure the compiler won't optimize the build function
        benchmark::DoNotOptimize(sep.query(kvs[0].first));
    }

    state.SetItemsProcessed(int64_t(state.iterations()) *
                            int64_t(state.range(0)));
}

template <class T>
void benchmark_update(T &sep, benchmark::State& state) {
    int i = 0;
    // Benchmark
    for (auto _ : state) {
        sep.update(i, i % 2);

        i = i + 1;
        if (i == state.range(0)) {
            i = 0;
        }
    }

    benchmark::DoNotOptimize(sep.query(0));
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
// * XorSep
// ************

static int xorsep_round_capacity(int cap) {
    int group_num = 0;
    return XorSep<uint64_t>::round_capacity(cap, group_num);
}

static void xorsep_args(benchmark::internal::Benchmark* b) {
    std::vector<int> small_args = {1000, 1000*1000, 10*1000*1000, 20*1000*1000, 40*1000*1000};
    std::vector<int> big_args = {80*1000*1000, 160*1000*1000};
    
    for (auto c: small_args) {
        b->Arg(xorsep_round_capacity(c));
    }

    if (enough_memory()) {
        for (auto c: big_args) {
            b->Arg(xorsep_round_capacity(c));
        }
    }
}

// xorsep build
BENCHMARK_TEMPLATE(BM_build, XorSep<uint64_t>)->Apply(args_10m);

// xorsep query
BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, xorsep_query, XorSep<uint64_t>)(benchmark::State& state) {
    benchmark_query<XorSep<uint64_t>>(sep, kvs, state);
}
BENCHMARK_REGISTER_F(BuildFixture, xorsep_query)->Apply(xorsep_args);

// xorsep query batch
BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, xorsep_query_batch, XorSep<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<XorSep<uint64_t>>(sep, kvs, state);
}
BENCHMARK_REGISTER_F(BuildFixture, xorsep_query_batch)->Apply(xorsep_args);

// xorsep update
BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, xorsep_update, XorSep<uint64_t>)(benchmark::State& state) {
    benchmark_update<XorSep<uint64_t>>(sep, state);
}
BENCHMARK_REGISTER_F(BuildFixture, xorsep_update)->Apply(args_10m);

// ************
// * Othello
// ************

static int othello_round_capacity(int cap) {
    // the following round rules are from othello.h:157
    //int hl1 = 8; //start from ma=64
    //int hl2 = 7; //start from mb=64
    //while ((1UL<<hl2) < keycount * 1) hl2++;
    //while ((1UL<<hl1) < keycount * 1.333334) hl1++;

    // when `keycount * 1.333334` is the power of 2, it has the best utilization rate
    int hl1 = 8;
    while ((1<<hl1) < cap*1.333334) hl1++;
    cap = (1<<hl1)/1.333334;
    assert(!((1<<hl1) < cap*1.333334));
    return cap;
}

static void othello_args(benchmark::internal::Benchmark* b) {
    std::vector<int> small_args = {1000, 1000*1000, 10*1000*1000, 20*1000*1000, 40*1000*1000};
    std::vector<int> big_args = {80*1000*1000, 160*1000*1000};
    
    for (auto c: small_args) {
        b->Arg(othello_round_capacity(c));
    }

    if (enough_memory()) {
        for (auto c: big_args) {
            b->Arg(othello_round_capacity(c));
        }
    }
}

// othello query
BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, othello_query, OthelloWrapper<uint64_t>)(benchmark::State& state) {
    benchmark_query<OthelloWrapper<uint64_t>>(sep, kvs, state);
}
BENCHMARK_REGISTER_F(BuildFixture, othello_query)->Apply(othello_args);

// ************
// * SetSep
// ************

// setsep query
#ifdef __linux__

static int setsep_round_capacity(int cap) {
    // the following round rules are from rte_efd.c:529
    // num_chunks = rte_align32pow2(max_num_rules/EFD_TARGET_CHUNK_NUM_RULES);

    int EFD_TARGET_CHUNK_NUM_RULES  = (EFD_TARGET_GROUP_NUM_RULES * 64);
    int c = EFD_TARGET_CHUNK_NUM_RULES;
    while (c < cap) c *= 2;

    int num_chunks = c/EFD_TARGET_CHUNK_NUM_RULES;
    // check num_chunks is the power of 2
    assert((num_chunks & (num_chunks - 1))==0);
    return c;
}
static void setsep_args(benchmark::internal::Benchmark *b) {
    std::vector<int> small_args = {1000, 1000*1000, 10*1000*1000, 20*1000*1000};
    std::vector<int> big_args = {40*1000*1000, 80*1000*1000};

    for (auto c: small_args) {
        b->Arg(setsep_round_capacity(c));
    }

    if (enough_memory()) {
        for (auto c: big_args) {
            b->Arg(setsep_round_capacity(c));
        }
    }
}

BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, setsep_query, SetSep<uint64_t>)(benchmark::State& state) {
    benchmark_query<SetSep<uint64_t>>(sep, kvs, state);
}
BENCHMARK_REGISTER_F(BuildFixture, setsep_query)->Apply(setsep_args);

// setsep query batch
BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, setsep_query_batch, SetSep<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<SetSep<uint64_t>>(sep, kvs, state);
}
BENCHMARK_REGISTER_F(BuildFixture, setsep_query_batch)->Apply(setsep_args);

// setsep update
BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, setsep_update, SetSep<uint64_t>)(benchmark::State& state) {
    benchmark_update<SetSep<uint64_t>>(sep, state);
}
BENCHMARK_REGISTER_F(BuildFixture, setsep_update)->Apply(args_10m);
#endif

// ************
// * XorSepDyn
// ************

// xorsepdyn query
BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, xorsepdyn_query, XorSepDyn<uint64_t>)(benchmark::State& state) {
    benchmark_query<XorSepDyn<uint64_t>>(sep, kvs, state);
}
BENCHMARK_REGISTER_F(BuildFixture, xorsepdyn_query)->Apply(args_50m);

// xorsepdyn query batch
BENCHMARK_TEMPLATE_DEFINE_F(BuildFixture, xorsepdyn_query_batch, XorSepDyn<uint64_t>)(benchmark::State& state) {
    benchmark_query_batch<XorSepDyn<uint64_t>>(sep, kvs, state);
}
BENCHMARK_REGISTER_F(BuildFixture, xorsepdyn_query_batch)->Apply(args_200m);

// xorsepdyn build
BENCHMARK_TEMPLATE(BM_build, XorSepDyn<uint64_t>)->Apply(args_10m);

// ************
// * stdmap
// ************

// stdmap query
BENCHMARK(BM_stdmap_query)->Arg(1000)->Arg(1000 * 1000);
