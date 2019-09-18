cc_binary(
    name = "bench",
    srcs = glob(["benchmarks/*.cpp"]),
    copts = ["-std=c++17", "-O3", "-march=native", "-Wall", "-Wextra", "-Werror"],
    deps = [
        "@benchmark//:benchmark", 
        "@benchmark//:benchmark_main",
    ]
)