cc_binary(
    name = "bench",
    srcs = glob(["ssfehash/*_bench.cpp"]),
    copts = ["-std=c++17", "-O2", "-march=native", "-Wall", "-Wextra", "-Werror"],
    deps = [
        "@benchmark//:benchmark", 
        "@benchmark//:benchmark_main",
        "//:ssfehash",
        "//:dev_utils",
    ]
)

cc_library(
    name = "ssfehash",
    hdrs = glob(["ssfehash/*.h"]),
)

cc_library(
    name = "dev_utils",
    hdrs = glob(["dev_utils/*.h"]),
)

cc_test(
    name = "test",
    srcs = glob(["ssfehash/*_test.cpp"]),
    # use O2 at there, because it will triger a gcc bug, which may caused by optimization: https://stackoverflow.com/questions/12984861/dont-understand-assuming-signed-overflow-warning
    copts = ["-std=c++17", "-O2", "-march=native", "-Wall", "-Wextra", "-Werror"],
    deps = [
        "//:ssfehash",
        "//:dev_utils",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ]
)