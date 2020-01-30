#include <cassert>
#include <set>

#include "dev_utils/dev_utils.h"
#include "xorsep/group.h"

void run(int n = 256 / 1.4, int m = 256) {
    int tot_cancles = 0;
    int tot_rounds = 100;
    for (int round = 0; round < tot_rounds; round++) {
        typedef MixFamily2<uint64_t, 10> HASH;

        std::vector<std::pair<uint64_t, bool>> kvs =
            generate_keyvalues(n, false, round);
        uint8_t *data = new uint8_t[256 / 8];

        // find a feasible seed
        int seed = HashGroup::build<uint64_t, HASH>(kvs, data, 256 / 8, false);
        assert(seed >= 0);

        int cancles = 0;
        bool success = HashGroup::build_expermients_<uint64_t, HASH>(
            kvs, data, 256 / 8, seed, cancles);
        tot_cancles += cancles;

        // make sure the build is correct
        assert(success);
        for (auto &kv : kvs) {
            bool r = HashGroup::query_group_size_256<uint64_t, HASH>(
                kv.first, data, seed);
            assert(r == kv.second);
        }

        delete[] data;
    }

    printf("n = %d, m = %d, #cancles in Gaussian Elimination = %.3lf\n", n, m,
           tot_cancles * 1.0 / tot_rounds);
}

int main() {
    for (int i = 150; i < 240; i++) {
        run(i, 256);
    }
    return 0;
}
