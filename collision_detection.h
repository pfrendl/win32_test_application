#pragma once

#include "datatypes.h"
#include <windows.h>


struct IndexPair {
    int a;
    int b;
};


struct IndexPairArray {
    IndexPair *pairs;
    int pair_count;
};


IndexPairArray sweep_and_prune(
    BoundingBox *bboxes,
    IndexPair *x_bound_idx_pairs,
    IndexPair *y_bound_idx_pairs,
    int bbox_count,
    Memory *memory
);





struct IndexedValue {
    double value;
    int index;
};


struct HashMap {
    int *indices;
    IndexPair *pairs;
    int size;
    int pair_count;
};

void insertion_sort(IndexedValue *ivs, int iv_count);
 
IndexPairArray inter_axis(IndexedValue *ivs, int bbox_count, Memory *memory);


constexpr uint64_t FNV_prime = 0x00000100000001b3;
uint64_t seed_hash(IndexPair *pair, int key_count, uint64_t seed);


HashMap hash_index_pair_array(IndexPairArray *idx_pairs, Memory *memory);


bool in_hash_map(IndexPair *pair, HashMap *hash_map);
