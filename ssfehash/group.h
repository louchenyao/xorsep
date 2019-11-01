#pragma once

#include <cstring>
#include <random>
#include <vector>
#include <x86intrin.h>

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
        int row = i;
        bool found = false;
        for (; j < m; j++) {
            for (row = i; row < n; row++) {
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
        for (int k = row + 1; k < n; k++) {  // elimnate k-th row
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

// This function outputs debug informations for profiling.
template <typename KEY_TYPE, class HASH_FAMILY>
bool build_profile_(const std::vector<std::pair<KEY_TYPE, bool> > &kvs,
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

    // profiling counters
    assert(m == 256);
    int tot_swap = 0;
    int swap_for_jth_column = 0;

    // do gauess elimnation
    int j = 0;                     // the column with first non-zero entry
    for (int i = 0; i < n; i++) {  // i-th row
        // find a row s.t. a[row][j] = true, then swap it to i-th row
        int row = i;
        bool found = false;
        for (; j < m; j++) {
            for (row = i; row < n; row++) {
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
        swap_for_jth_column = 0;
        for (int k = row + 1; k < n; k++) {  // elimnate k-th row
            if (a[k][j]) {
                swap_for_jth_column += 1;
                //printf("j = %d, swap row %d with row %d", j, i, k);
                // set l < m + 1 to xor the answer
                for (int l = j; l < m + 1; l++) {
                    a[k][l] ^= a[i][l];
                }
            }
        }
        tot_swap += swap_for_jth_column;
        //printf("swap %d times for j = %d\n", swap_for_jth_column, j);
    }
    printf("swap %d times in total\n", tot_swap);

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
        int row = i;
        for (; j < m; j++) {
            for (row = i; row < n; row++) {
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
        for (int k = row + 1; k < n; k++) {  // elimnate k-th row
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
    uint64_t *d64 = (uint64_t *)data;
    for (int i = n - 1; i >= 0; i--) {
        // find the first non-zero column
        int j = 0;
        for (int k = 0; k < bitset_len; k++) {
            j = _tzcnt_u64(a[i][k]);
            if (j != 64) {
                j += k*64;
                break;
            }
        }

        bool d = false;
        for (int k = j/64; k < bitset_len; k++) {
            d ^= _popcnt64(a[i][k] & d64[k]) % 2;
        }
        set_bit(data, j, d^b[i]);
    }
    return true;
}


// len is the length of the data array
inline int tzcnt(uint64_t *data, int len) {
    for (int i = 0; i < len; i++) {
        if (data[i]) {
            return i*64 + _tzcnt_u64(data[i]);
        }
    }
    return len*64;
}

template <typename KEY_TYPE, class HASH_FAMILY>
bool build_bitset_2_(const std::vector<std::pair<KEY_TYPE, bool> > &kvs,
                 uint8_t *data, int data_size, int hash_family) {
    int n = kvs.size();
    int m = data_size * 8;
    HASH_FAMILY h;

    // asert the matrix size is n*256
    assert(n <= m);
    assert(m == 256);


    // build the hash matrix
    const int bitset_len = 256 / 64;
    uint64_t a[256][bitset_len];
    uint64_t fnz_a[256][bitset_len]; // First non-zero entry of a. It the transpose of a which only reserves the first non-zero entry.
    bool b[n];
    memset(a, 0, sizeof(a));
    memset(fnz_a, 0, sizeof(fnz_a));
    memset(b, 0, sizeof(b));

    for (int i = 0; i < n; i++) {
        auto [h1, h2, h3] = h.hash(kvs[i].first, hash_family, m);
        flip_bit(a[i], h1);
        flip_bit(a[i], h2);
        flip_bit(a[i], h3);
        set_bit(fnz_a[tzcnt(a[i], bitset_len)], i);
        b[i] = kvs[i].second;
    }

    // do gauss elimnation
    int rank = 0;
    int pivot_rows[256]; // pivot_rows[j] means the row which pivot is on j-th column
    for (int j = 0; j < 256; j++) { // j-th column
        int pivot_row = tzcnt(fnz_a[j], bitset_len);
        pivot_rows[j] = pivot_row;
        if (pivot_row >= 256) continue;
        //printf("pivot_row = %d\n", pivot_row);
        rank += 1;
        flip_bit(fnz_a[j], pivot_row);

        // elimnate other rows
        while (true) {
            int target_row = tzcnt(fnz_a[j], bitset_len);
            if (target_row >= 256) break;
            flip_bit(fnz_a[j], target_row);
            //printf("elimnating %d\n", target_row);

            // elimnate target_row
            for (int k = j/64; k < bitset_len; k++) {
                a[target_row][k] ^= a[pivot_row][k];
                //printf("a[target_row][k] = %lu\n", a[target_row][k]);
            }
            b[target_row] ^= b[pivot_row];

            // find the new first non-zero entry
            int new_fnz_column = tzcnt(a[target_row], bitset_len);
            if (new_fnz_column < 256) {
                flip_bit(fnz_a[new_fnz_column], target_row);
            } else {
                return false; // one row is just disappeared! The row rank won't be full anymore.
            }
        }
    }

    if (rank < n) return false;

    // substitute back
    memset(data, 0, bitset_len);
    uint64_t *d64 = (uint64_t *)data;
    for (int j = 255; j >= 0; j--) {
        int pivot_row = pivot_rows[j];
        if (pivot_row >= 256) continue;

        bool d = false;
        for (int k = j/64; k < bitset_len; k++) {
            d ^= _popcnt64(a[pivot_row][k] & d64[k]) % 2;
        }
        set_bit(data, j, d^b[pivot_row]);
        //printf("pivot_row = %d\n", pivot_row);
        //printf("d = %d, b[pivot_row] = %d\n", int(d), int(b[pivot_row]));
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