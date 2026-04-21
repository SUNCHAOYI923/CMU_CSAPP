#include "memory_hierarchy.h"
#include "prefetcher.h"
#include "repl_policy.h"
#include <cmath>
#include <iomanip>
#include <iostream>

using namespace std;

MainMemory::MainMemory(int lat) : latency(lat) {}

int MainMemory::access(uint64_t addr, char type, uint64_t cycle) {
    (void)addr;
    (void)type;
    (void)cycle;
    access_count++;
    return latency;
}

void MainMemory::printStats() {
    cout << "  [Main Memory] Total Accesses: " << access_count << endl;
}

CacheLevel::CacheLevel(string name, CacheConfig cfg, MemoryObject* next)
    : level_name(name), config(cfg), next_level(next) {
    policy = createReplacementPolicy(config.policy_name);
    prefetcher = createPrefetcher(config.prefetcher, config.block_size);

    uint64_t total_bytes = (uint64_t)config.size_kb * 1024;
    num_sets = total_bytes / (config.block_size * config.associativity);

    offset_bits = log2(config.block_size);
    index_bits = log2(num_sets);

    sets.resize(num_sets, vector<CacheLine>(config.associativity));

    cout << "Constructed " << level_name << ": "
         << config.size_kb << "KB, " << config.associativity << "-way, "
         << config.latency << "cyc, "
         << "[" << config.policy_name << " + " << prefetcher->getName() << "]" << endl;
}

CacheLevel::~CacheLevel() {
    delete policy;
    delete prefetcher;
}

uint64_t CacheLevel::get_index(uint64_t addr) {
    // TODO: Task 1
    // Compute the set index from the address.
    // Hint: remove block offset bits first, then keep only the index bits.
    return (addr >> offset_bits) & (num_sets - 1);
}

uint64_t CacheLevel::get_tag(uint64_t addr) {
    // TODO: Task 1
    // Compute the tag from the address.
    // Hint: shift away both block offset bits and set index bits.
    return addr >> (offset_bits + index_bits);
}

uint64_t CacheLevel::reconstruct_addr(uint64_t tag, uint64_t index) {
    // TODO: Task 1 / Task 2
    // Rebuild a block-aligned address from a tag and set index.
    // This helper is useful when writing back an evicted dirty line.
    return (tag << (offset_bits + index_bits)) | (index << offset_bits);
}

void CacheLevel::write_back_victim(const CacheLine& line, uint64_t index, uint64_t cycle) {
    // TODO: Task 1 / Task 2
    // Move dirty write-back logic into this helper.
    // Suggested steps:
    // 1. If the victim is not dirty, return immediately.
    // 2. If there is no next level, return immediately.
    // 3. Increment the write-back counter.
    // 4. Reconstruct the evicted block address from tag + index.
    // 5. Send a write access to the next level.
    if (!line.dirty || next_level == NULL) return;
    ++write_backs;
    uint64_t new_addr = reconstruct_addr (line.tag, index);
    next_level->access (new_addr, 'w', cycle);
}

int CacheLevel::access(uint64_t addr, char type, uint64_t cycle) {
    int lat = config.latency;

    // TODO: Task 1
    // 1. Derive the address fields for the current cache geometry:
    //    - block offset bits
    //    - set index bits
    //    - tag bits
    // 2. Use the address to compute index/tag and select the set.
    // 3. Search all ways for a valid tag match.
    // 4. On hit:
    //    - increment hits
    //    - call policy->onHit(...)
    //    - update dirty bit for writes
    //    - clear is_prefetched if a prefetched line is consumed
    // 5. On miss:
    //    - increment misses
    //    - find an invalid line or select a victim with policy->getVictim(...)
    //    - call write_back_victim(...) if the chosen victim is dirty
    //    - fetch the requested block from next_level and add that latency to lat
    //    - install the new cache line and call policy->onMiss(...)
    // 6. Your code should work correctly even if cache size, associativity,
    //    number of sets, or cache line size changes.
    // 7. Task 3: after demand access logic works, call the prefetcher here and
    //    install returned blocks through install_prefetch(...).

    uint64_t index = get_index (addr), tag = get_tag (addr);
    int hit_pos = -1, invalid_pos = -1;
    bool is_miss = true;
    for (uint32_t i = 0;i < config.associativity;++i)
    {
        if (sets[index][i].valid && sets[index][i].tag == tag) hit_pos = i, is_miss = false;
        if (!sets[index][i].valid && invalid_pos == -1) invalid_pos = i;
    }
    if (~hit_pos)
    {
        ++hits;
        if (type == 'w') sets[index][hit_pos].dirty = true;
        if (sets[index][hit_pos].is_prefetched) sets[index][hit_pos].is_prefetched = false;
        policy->onHit (sets[index], hit_pos, cycle);
    }
    else
    {
        ++misses;
        int victim = invalid_pos != -1 ? invalid_pos : policy->getVictim (sets[index]);
        if (sets[index][victim].dirty) write_back_victim (sets[index][victim], index, cycle);
        if (next_level) lat += next_level->access (addr, 'r', cycle);
        sets[index][victim].valid = true;
        sets[index][victim].tag = tag;
        sets[index][victim].dirty = type == 'w';
        sets[index][victim].is_prefetched = false;
        policy->onMiss (sets[index], victim, cycle);
    }
    if (prefetcher)
    {
        vector <uint64_t> pre_addrs = prefetcher->calculatePrefetch (addr,is_miss);
        for (uint64_t pre_addr : pre_addrs)
        {
            install_prefetch (pre_addr, cycle);
            ++prefetch_issued;
        }
    }
    return lat;
}

void CacheLevel::install_prefetch(uint64_t addr, uint64_t cycle) {
    // TODO: Task 3
    // Implement a prefetch fill path similar to the miss path in access(), but
    // treat prefetched lines as clean and mark is_prefetched = true.
    // If you evict a dirty victim during prefetch installation, reuse
    // write_back_victim(...) instead of duplicating that logic.

    uint64_t index = get_index (addr), tag = get_tag (addr);
    int invalid_pos = -1;
    for (uint32_t i = 0;i < config.associativity;++i)
    {
        if (sets[index][i].valid && sets[index][i].tag == tag) return;
        if (!sets[index][i].valid && invalid_pos == -1) invalid_pos = i;
    }
    int victim = invalid_pos != -1 ? invalid_pos : policy->getVictim (sets[index]);
    if (sets[index][victim].dirty) write_back_victim (sets[index][victim], index, cycle);
    if (next_level) next_level->access (addr, 'r', cycle);
    sets[index][victim].valid = true;
    sets[index][victim].tag = tag;
    sets[index][victim].dirty = false;
    sets[index][victim].is_prefetched = true;
    policy->onMiss (sets[index], victim, cycle);
}   

void CacheLevel::printStats() {
    uint64_t total = hits + misses;
    cout << "  [" << level_name << "] "
         << "Hit Rate: " << fixed << setprecision(2) << (total ? (double)hits / total * 100.0 : 0) << "% "
         << "(Access: " << total << ", Miss: " << misses << ", WB: " << write_backs << ")" << endl;
    cout << "      Prefetches Issued: " << prefetch_issued << endl;
}
