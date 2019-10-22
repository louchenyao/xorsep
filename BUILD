SEPSET_COPTS = ["-Wno-error=register"]
SEPSET_LINKOPTS = ["-pthread", "-lnuma", "-ldl"]

cc_binary(
    name = "bench",
    srcs = glob(["ssfehash/*_bench.cpp"]),
    # use -Wno-error=strict-overflow there, because O3 trigers a gcc bug https://stackoverflow.com/questions/12984861/dont-understand-assuming-signed-overflow-warning
    copts = ["-std=c++17", "-O3", "-march=native", "-Wall", "-Wextra", "-Werror", "-Wno-error=strict-overflow"] + SEPSET_COPTS,
    linkopts = SEPSET_LINKOPTS,
    deps = [
        "@benchmark//:benchmark", 
        "@benchmark//:benchmark_main",
        "@perfevent//:all",
        "@dpdk//:sepset",
        "//:ssfehash",
        "//:dev_utils",
    ]
)

cc_library(
    name = "ssfehash",
    hdrs = glob(["ssfehash/*.h"]),
    deps = [
        "@smhasher//:murmurhash3",
    ]
)

cc_library(
    name = "dev_utils",
    hdrs = glob(["dev_utils/*.h"]),
    deps = [
        "@benchmark//:benchmark",
        "@perfevent//:all",
    ]
)

cc_test(
    name = "test",
    srcs = glob(["ssfehash/*_test.cpp"]),
    copts = ["-std=c++17", "-O3", "-march=native", "-Wall", "-Wextra", "-Werror", "-Wno-error=strict-overflow"],
    deps = [
        "//:ssfehash",
        "//:dev_utils",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ]
)
