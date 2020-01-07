#pragma once

#include <algorithm>
#include <random>
#include <set>
#include <vector>

// generates key value pairs.
// If `dense` is true, then the keys range is [0, n)
inline std::vector<std::pair<uint64_t, bool>> generate_keyvalues(int n, bool dense=true, int seed=20200108) {
    std::mt19937_64 g(seed);
    std::vector<std::pair<uint64_t, bool>> kvs;
    for (int i = 0; i < n; ++i) {
        uint64_t key = i;
        if (!dense) {
            key = g();
        }
        kvs.push_back(std::make_pair(key, g() % 2));
    }
    std::shuffle(kvs.begin(), kvs.end(), g);
    return kvs;
}
