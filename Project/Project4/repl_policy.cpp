#include "repl_policy.h"

// =========================================================
// TODO: Task 1 / Task 3 replacement policies
// Implement LRU first, then extend with SRRIP / BIP.
// =========================================================

void LRUPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    // TODO: mark the hit line as most recently used.
    set[way].last_access = cycle;
}

void LRUPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    // TODO: initialize a newly inserted line as MRU.
    set[way].last_access = cycle;
}

int LRUPolicy::getVictim(std::vector<CacheLine>& set) {
    // TODO: return the least recenuint times = 0x3f3f3f3f;
    int victim = 0;
    uint64_t time = set[0].last_access;
    for (size_t i = 1;i < set.size ();++i)
        if (set[i].last_access < time) time = set[i].last_access, victim = i;
    return victim;
}

void SRRIPPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    // TODO: typically promote the line to RRPV=0.
    set[way].rrpv = 0;
}

void SRRIPPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    // TODO: insert with a long re-reference interval, e.g. RRPV=2.
    set[way].rrpv = 2;
}

int SRRIPPolicy::getVictim(std::vector<CacheLine>& set) {
    // TODO: search for RRPV==3, otherwise age all lines and retry.
    for (uint32_t i = 0;i < set.size ();++i)
        if (set[i].rrpv == 3) return i;
    for (size_t i = 0;i < set.size ();++i) ++set[i].rrpv;
    return getVictim (set);
}

void BIPPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    // TODO: hits still become MRU.
    set[way].last_access = cycle;
}

void BIPPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    // TODO: mostly insert at LRU position, but occasionally insert at MRU.
    // Hint: use insertion_counter and throttle.
    ++insertion_counter;
    set[way].last_access = insertion_counter % throttle ? 0 : cycle;
}

int BIPPolicy::getVictim(std::vector<CacheLine>& set) {
    // TODO: BIP usually uses the same victim selection as LRU.
    int victim = 0;
    uint64_t time = set[0].last_access;
    for (size_t i = 1;i < set.size ();++i)
        if (set[i].last_access < time) time = set[i].last_access, victim = i;
    return victim;
}

ReplacementPolicy* createReplacementPolicy(std::string name) {
    if (name == "SRRIP") return new SRRIPPolicy();
    if (name == "BIP") return new BIPPolicy();
    return new LRUPolicy();
}
