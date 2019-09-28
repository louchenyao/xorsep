#pragma once

#include <algorithm>
#include <random>
#include <set>
#include <vector>

inline std::vector<std::pair<uint64_t, bool>> generate_keyvalues(int n) {
    // construct key-value pairs
    std::vector<std::pair<uint64_t, bool>> kvs;
    std::set<uint64_t> key_set;
    for (int i = 0; i < n; i++) {
        uint64_t k = uint64_t(uint64_t(rand()) << 32) ^ rand();
        bool v = rand() % 2;
        if (key_set.find(k) != key_set.end()) {
            i--;
            continue;
        }

        key_set.insert(k);
        kvs.push_back(std::make_pair(k, v));
    }

    return kvs;
}

inline std::vector<std::pair<uint64_t, bool>> generate_continous_keyvalues(int n) {
    std::vector<std::pair<uint64_t, bool>> kvs;
    for (int i = 0; i < n; ++i) {
        kvs.push_back(std::make_pair(uint64_t(i), rand() % 2));
    }
    return kvs;
}

template <typename T>
inline void shuffle_vector(std::vector<T> &v) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(v.begin(), v.end(), g);
}
