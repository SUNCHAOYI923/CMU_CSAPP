#include "prefetcher.h"

std::vector<uint64_t> NextLinePrefetcher::calculatePrefetch(uint64_t current_addr, bool miss) {
    std::vector<uint64_t> prefetches;

    // TODO: Task 3
    // 1. Align current_addr down to the current cache block.
    // 2. Prefetch the next sequential block.
    uint64_t current_block_addr = (current_addr / block_size) * block_size;
    uint64_t next_block_addr = current_block_addr + block_size;
    prefetches.push_back (next_block_addr);
    return prefetches;
}

std::vector<uint64_t> StridePrefetcher::calculatePrefetch(uint64_t current_addr, bool miss) {
    std::vector<uint64_t> prefetches;
    // TODO: Task 3
    // Suggested design:
    // 1. Track the previous accessed block.
    // 2. Compute the current stride in block units.
    // 3. If the same stride repeats enough times, prefetch the next block at that stride.
    // 4. Update last_block / last_stride / confidence.
    uint64_t current_block = current_addr / block_size;
    int64_t stride = (int64_t)current_block - (int64_t)last_block;
    if (stride == last_stride && stride > 0) prefetches.push_back ((current_block + stride) * block_size);
    last_block = current_block; last_stride = stride;
    return prefetches;
}

std::vector<uint64_t> CustomPrefetcher::calculatePrefetch(uint64_t current_addr, bool miss) {
    std::vector<uint64_t> prefetches;
    uint64_t current_block = current_addr / block_size;
    int64_t stride = (int64_t)current_block - (int64_t)last_block;
    if (stride != 0 && stride == last_stride) prefetches.push_back ((current_block + stride) * block_size);
    if (stride == 7 || stride == 1 || stride == 64) prefetches.push_back ((current_block + stride) * block_size);
    last_stride = stride; last_block = current_block;
    return prefetches;
}


Prefetcher* createPrefetcher(std::string name, uint32_t block_size) {
    if (name == "NextLine") return new NextLinePrefetcher(block_size);
    if (name == "Stride") return new StridePrefetcher(block_size);
    if (name == "Custom") return new CustomPrefetcher(block_size);
    return new NoPrefetcher();
}
