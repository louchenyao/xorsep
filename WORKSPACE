load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "gtest",
    remote = "https://github.com/google/googletest",
    tag = "release-1.8.1",
)

git_repository(
    name = "benchmark",
    remote = "https://github.com/google/benchmark",
    tag = "v1.5.0",
)

new_git_repository(
    name = "perfevent",
    remote = "https://github.com/viktorleis/perfevent.git",
    commit = "f34b43af69e45521b973b4065e25504c3ec43d93",
    build_file_content = """
cc_library(
    name = "all",
    hdrs = ["PerfEvent.hpp"],
    visibility = ["//visibility:public"],
)""",
)