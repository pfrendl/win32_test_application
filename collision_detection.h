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
    int *x_bound_idxs,
    int *y_bound_idxs,
    int bbox_count,
    Memory *memory
);

void narrow_phase(Vec2 *positions, double *radii, IndexPairArray *collisions);





struct IndexedInterval {
    double minimum;
    double maximum;
    int index;
};


struct HashMap {
    int *indices;
    IndexPair *pairs;
    int size;
    int pair_count;
};

void insertion_sort(IndexedInterval *ivs, int iv_count);
 
IndexPairArray inter_axis(IndexedInterval *ivs, int bbox_count, Memory *memory);


constexpr uint64_t FNV_prime = 0x00000100000001b3;
uint64_t seed_hash(IndexPair *pair, int key_count, uint64_t seed);


HashMap hash_index_pair_array(IndexPairArray *idx_pairs, Memory *memory);


bool in_hash_map(IndexPair *pair, HashMap *hash_map);
