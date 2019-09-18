cc_binary(
    name = "bench",
    srcs = glob(["benchmarks/*.cpp"]),
    copts = ["-std=c++17", "-O3", "-march=native", "-Wall", "-Wextra", "-Werror"],
    deps = [
        "@benchmark//:benchmark", 
        "@benchmark//:benchmark_main",
    ]
)

cc_library(
    name = "ssfehash",
    hdrs = ["ssfehash/hash_group.h"],
)

cc_test(
    name = "test",
    srcs = glob(["ssfehash/*_test.cpp"]),
    # use O2 at there, because it will triger a gcc bug, which may caused by optimization: https://stackoverflow.com/questions/12984861/dont-understand-assuming-signed-overflow-warning
    copts = ["-std=c++17", "-O2", "-march=native", "-Wall", "-Wextra", "-Werror"],
    deps = [
        "//:ssfehash",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ]
)