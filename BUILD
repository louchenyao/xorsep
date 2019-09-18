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
    copts = ["-std=c++17", "-O3", "-march=native", "-Wall", "-Wextra", "-Werror"],
    deps = [
        "//:ssfehash",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ]
)