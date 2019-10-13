#pragma once

#include <cstring>
#include <random>
#include <vector>

#include "ssfehash/hash_family.h"

template <typename T>
inline bool get_bit(T *data, int i) {
    const int s = sizeof(T) * 8;
    return data[i / s] & (T(1) << (i % s));
}

template <typename T>
inline void set_bit(T *data, int i, bool val = true) {
    const int s = sizeof(T) * 8;
    data[i / s] = (data[i / s] & ~(T(1) << (i % s))) | (T(val) << (i % s));
}

template <typename T>
inline void flip_bit(T *data, int i, bool cond = true) {
    const int s = sizeof(T) * 8;
    data[i / s] ^= (T(cond) << (i % s));
}

namespace HashGroup {

template <typename KEY_TYPE, class HASH_FAMILY>
bool build_naive_(const std::vector<std::pair<KEY_TYPE, bool> > &kvs,
                 uint8_t *data, size_t data_size, int hash_family) {
    int n = kvs.size();
    int m = data_size * 8;
    HASH_FAMILY h;

    // build the hash matrix
    bool a[n][m + 1];
    memset(a, 0, sizeof(a));
    for (int i = 0; i < n; i++) {
        auto [h1, h2, h3] = h.hash(kvs[i].first, hash_family, data_size * 8);
        a[i][h1] ^= true;
        a[i][h2] ^= true;
        a[i][h3] ^= true;
        a[i][m] = kvs[i].second;
    }

    // do gauess elimnation
    int j = 0;                     // the column with first non-zero entry
    for (int i = 0; i < n; i++) {  // i-th row
        // find a row s.t. a[row][j] = true, then swap it to i-th row
        bool found = false;
        for (; j < m; j++) {
            for (int row = i; row < n; row++) {
                if (a[row][j]) {
                    // swap a[row] and a[i]
                    for (int k = j; k < m + 1; k++) {
                        std::swap(a[i][k], a[row][k]);
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
        // printf("i = %d, j = %d\n", i, j);
        if (!found) return false;

        // elimnate other rows which j-th column elements are true
        for (int k = i + 1; k < n; k++) {  // elimnate k-th row
            if (a[k][j]) {
                // set l < m + 1 to xor the answer
                for (int l = j; l < m + 1; l++) {
                    a[k][l] ^= a[i][l];
                }
            }
        }
    }

    // calculate result
    memset(data, 0, data_size);
    for (int i = n - 1; i >= 0; i--) {
        // find the first non-zero column
        for (j = 0; j < m && a[i][j] == false; j++)
            ;
        assert(j < m);

        set_bit(data, j, a[i][m]);
        for (int k = j + 1; k < m; k++) {
            flip_bit(data, j, a[i][k] & get_bit(data, k));
        }
        // printf("i = %d, x = %d\n", i, int(x_[i]));
    }

    return true;
}

template <typename KEY_TYPE, class HASH_FAMILY>
bool build_bitset_(const std::vector<std::pair<KEY_TYPE, bool> > &kvs,
                 uint8_t *data, int data_size, int hash_family) {
    int n = kvs.size();
    int m = data_size * 8;
    HASH_FAMILY h;

    // build the hash matrix
    int bitset_len = (m+63)/64;
    uint64_t a[n][bitset_len];
    bool b[n];
    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));

    for (int i = 0; i < n; i++) {
        auto [h1, h2, h3] = h.hash(kvs[i].first, hash_family, m);
        flip_bit(a[i], h1);
        flip_bit(a[i], h2);
        flip_bit(a[i], h3);
        b[i] = kvs[i].second;
    }

    // do gauss elimnation
    int j = 0;                     // the column with first non-zero entry
    for (int i = 0; i < n; i++) {  // i-th row
        // find a row s.t. a[row][j] = true, then swap it to i-th row
        bool found = false;
        for (; j < m; j++) {
            for (int row = i; row < n; row++) {
                if (get_bit(a[row], j)) {
                    // swap a[row] and a[i]
                    for (int k = j/64; k < bitset_len; k++) {
                        std::swap(a[i][k], a[row][k]);
                    }
                    std::swap(b[i], b[row]);

                    found = true;
                    break;
                }
            }

            if (found) {
                break;
            }
        }
        // printf("i = %d, j = %d\n", i, j);
        if (!found) return false;

        // elimnate other rows which j-th column elements are true
        for (int k = i + 1; k < n; k++) {  // elimnate k-th row
            if (get_bit(a[k], j)) {
                // set l < m + 1 to xor the answer
                for (int l = j/64; l < bitset_len; l++) {
                    a[k][l] ^= a[i][l];
                }
                b[k] ^= b[i];
            }
        }
    }

    // calculate result
    memset(data, 0, data_size);
    for (int i = n - 1; i >= 0; i--) {
        // find the first non-zero column
        for (j = 0; j < m && get_bit(a[i], j) == false; j++)
            ;
        assert(j < m);

        set_bit(data, j, b[i]);
        for (int k = j + 1; k < m; k++) {
            flip_bit(data, j, get_bit(a[i], k) & get_bit(data, k));
        }
    }
    return true;
}

template <typename KEY_TYPE, class HASH_FAMILY>
int build(const std::vector<std::pair<KEY_TYPE, bool> > &kvs, uint8_t *data,
          size_t data_size, bool store_index_to_data = true) {
    int hash_family_index_size = 1;
    assert(HASH_FAMILY_NUM <= 256);
    if (!store_index_to_data) {
        hash_family_index_size = 0;
    }

    // ensure the data size is large enough
    assert((data_size - hash_family_index_size) * 8 >= kvs.size());

    // try to construct with all hash families, and return the first successed
    // one.
    for (int i = 0; i < HASH_FAMILY_NUM; ++i) {
        if (build_bitset_<KEY_TYPE, HASH_FAMILY>(
                kvs, data + hash_family_index_size,
                data_size - hash_family_index_size, i)) {
            
            if (store_index_to_data) {
                data[0] = (uint8_t)i;
            }
            return i;
        }
    }

    return -1;
}

template <typename KEY_TYPE, class HASH_FAMILY>
bool query(KEY_TYPE k, uint8_t *data, int data_size) {
    HASH_FAMILY h;
    auto [h1, h2, h3] = h.hash(k, data[0], (data_size - 1) * 8);
    return get_bit(data + 1, h1) ^ get_bit(data + 1, h2) ^
           get_bit(data + 1, h3);
}

template <typename KEY_TYPE, class HASH_FAMILY>
bool query_group_size_256(KEY_TYPE k, uint8_t *data, int hash_index) {
    HASH_FAMILY h;
    auto [h1, h2, h3] = h.hash(k, hash_index);
    return get_bit(data, h1 & 255) ^ get_bit(data, h2 & 255) ^
           get_bit(data, h3 & 255);
}

}  // namespace HashGroup