#include <gtest/gtest.h>

#include <set>

#include "ssfehash/group.h"
#include "dev_utils/dev_utils.h"

TEST(GroupProfile, Basic) {
    typedef MixFamily<uint64_t> HASH;
    std::vector<std::pair<uint64_t, bool>> kvs = generate_keyvalues(230);
    uint8_t *data = new uint8_t[256 / 8];
    int hash_index = HashGroup::build<uint64_t, HASH>(kvs, data, 256 / 8, false);
    assert(hash_index >= 0);
    
    memset(data, 0, 256 / 8);
    HashGroup::build_profile_<uint64_t, HASH>(kvs, data, 256 / 8, hash_index);

    for (auto &kv : kvs) {
        bool r = HashGroup::query_group_size_256<uint64_t, HASH> (kv.first, data, hash_index);
        EXPECT_EQ(r, kv.second);
    }
    
    delete[] data;
}
