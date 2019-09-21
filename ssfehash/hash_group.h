#pragma once

#include <random>
#include <vector>

#include "ssfehash/hash_family.h"

template <typename KEY_TYPE>
class HashGroup {
   public:
    HashGroup(const std::vector<std::pair<KEY_TYPE, bool> > &kvs,
              double x_size_factor = 1.1) {
        // try to construct at most 100 times
        for (int i = 0; i < 100; ++i) {
            if (construct(kvs, x_size_factor, i)) {
                printf("Constructed %d times to find a proper set of hash functions!\n", i+1);
                hash_family_ = i;
                return;
            }
        }
        assert(false);
    }
    ~HashGroup() { delete[] x_; }

    bool construct(const std::vector<std::pair<KEY_TYPE, bool> > &kvs,
                   double x_size_factor, int hash_family) {
        int n = kvs.size();
        int m = n * x_size_factor;
        m_ = m;
        assert(n >= 3);

        // build the hash matrix
        bool a[n][m + 1];
        memset(a, 0, sizeof(a));
        for (int i = 0; i < n; i++) {
            auto [h1, h2, h3] = h.hash(kvs[i].first, hash_family, m_);
            a[i][h1] ^= true;
            a[i][h2] ^= true;
            a[i][h3] ^= true;
            a[i][m] = kvs[i].second;
            // printf("h1 = %d, h2 = %d, h3 = %d, k = %llu, v = %d\n", h1, h2,
            // h3, k, bool(kvs[i].second));
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

        // calculate x
        x_ = new bool[m];
        memset(x_, 0, m * sizeof(bool));
        for (int i = n - 1; i >= 0; i--) {
            // find the first non-zero column
            for (j = 0; j < m && a[i][j] == false; j++)
                ;
            assert(j < m);

            x_[j] = a[i][m];
            for (int k = j + 1; k < m; k++) {
                x_[j] ^= a[i][k] & x_[k];
            }
            // printf("i = %d, x = %d\n", i, int(x_[i]));
        }

        return true;
    }

    bool query(KEY_TYPE k) {
        auto [h1, h2, h3] = h.hash(k, hash_family_, m_);
        return x_[h1] ^ x_[h2] ^ x_[h3];
    }

   private:
    int hash_family_;
    bool *x_;
    int m_;  // x_ size

    HashFamily<KEY_TYPE> h;
};