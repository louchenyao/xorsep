#include <cassert>
#include <set>

#include "ssfehash/group.h"
#include "dev_utils/dev_utils.h"

void run(int n = 256 / 1.4) {
    printf("n = %d\n", n);
    typedef MixFamily<uint64_t> HASH;
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(256/1.4);
    uint8_t *data = new uint8_t[256 / 8];
    int hash_index = HashGroup::build<uint64_t, HASH>(kvs, data, 256 / 8, false);
    assert(hash_index >= 0);
    printf("hash_index = %d\n", hash_index);
    
    memset(data, 0, 256 / 8);
    HashGroup::build_profile_<uint64_t, HASH>(kvs, data, 256 / 8, hash_index);
    printf("=====\n");

    for (auto &kv : kvs) {
        bool r = HashGroup::query_group_size_256<uint64_t, HASH> (kv.first, data, hash_index);
        assert(r == kv.second);
    }
    
    delete[] data;
}

int main() {
    for (int i = 0; i < 10; i++) {
        run(256 / 1.4);
        run(256 / 1.1);
    }
    return 0;
}
