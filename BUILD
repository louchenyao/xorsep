load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

SETSEP_COPTS = ["-Wno-error=register"]
SETSEP_LINKOPTS = ["-pthread", "-lnuma", "-ldl"]

cc_binary(
    name = "bench",
    srcs = glob(["ssfehash/*_bench.cpp"]),
    # use -Wno-error=strict-overflow there because O3 trigers a gcc bug
    # https://stackoverflow.com/questions/12984861/dont-understand-assuming-signed-overflow-warning
    copts = ["-std=c++17", "-O3", "-march=native", "-Wall", "-Wextra", "-Werror", "-Wno-error=strict-overflow"] + SETSEP_COPTS,
    linkopts = select({
        "@bazel_tools//src/conditions:linux_x86_64": SETSEP_LINKOPTS,
        "//conditions:default": [],
    }),
    defines = ["NO_OUTPUT"], # for annoying othello
    deps = [
        "@benchmark//:benchmark", 
        "@benchmark//:benchmark_main",
        "@perfevent//:all",
        "@othello//:othello_wrapper",
        "//:ssfehash",
        "//:dev_utils",
    ] + select({
        "@bazel_tools//src/conditions:linux_x86_64": ["@dpdk//:setsep"],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "ssfehash",
    hdrs = glob(["ssfehash/*.h"]),
    deps = [
        "@smhasher//:murmurhash3",
        "@xxhash//:xxhash",
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
        "@xxhash//:xxhash",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ]
)

cc_binary(
    name = "group_exp_cancles",
    srcs = glob(["ssfehash/group_exp_cancles.cpp"]),
    copts = ["-std=c++17", "-O3", "-march=native", "-Wall", "-Wextra", "-Werror", "-Wno-error=strict-overflow"],
    deps = [
        "//:ssfehash",
        "//:dev_utils",
        "@xxhash//:xxhash",
    ]
)