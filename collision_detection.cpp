#include "collision_detection.h"


int partition(IndexedInterval *ivs, int low, int high) {
    IndexedInterval pivot = ivs[low];
    int leftwall = low;
    
    for(int i = low + 1; i < high; ++i) {
        if(ivs[i].minimum < pivot.minimum) {
            ++leftwall;
            IndexedInterval tmp = ivs[i];
            ivs[i] = ivs[leftwall];
            ivs[leftwall] = tmp;
        }
    }
    
    ivs[low] = ivs[leftwall];
    ivs[leftwall] = pivot;
    
    return leftwall;
}


void quicksort(IndexedInterval *ivs, int low, int high) {
    if(low < high) {
        int pivot_location = partition(ivs, low, high);
        quicksort(ivs, low, pivot_location);
        quicksort(ivs, pivot_location + 1, high);
    }
}


void insertion_sort(IndexedInterval *ivs, int iv_count) {
    for(int i = 0; i < iv_count; ++i) {
        IndexedInterval tmp = ivs[i];
        int j = i;
        
        while(j > 0 && ivs[j - 1].minimum > tmp.minimum) {
            ivs[j] = ivs[j - 1];
            --j;
        }
        
        ivs[j] = tmp;
    }
}

 
IndexPairArray inter_axis(IndexedInterval *ivs, int bbox_count, Memory *memory) {
    IndexPair *inters = (IndexPair *)m_alloc(memory, 0);
    int inter_count = 0;
    
    m_alloc(memory, sizeof(IndexPair) * bbox_count);
    for(int i = 0; i < bbox_count; ++i) {
        double bound = ivs[i].maximum;
        int idx_i = ivs[i].index;
        int j = i + 1;
        for(; j < bbox_count && ivs[j].minimum < bound; ++j) {
            int idx_j = ivs[j].index;
            int left = idx_i < idx_j;
            int right = 1 - left;
            inters[inter_count++] = {left * idx_i + right * idx_j, left * idx_j + right * idx_i};
        }
        m_alloc(memory, sizeof(IndexPair) * (j - i - 1));
    }
    
    return {inters, inter_count};
}


inline uint64_t seed_hash(IndexPair pair, int key_count, uint64_t seed) {
    seed = (seed * FNV_prime) ^ pair.a;
    seed = (seed * FNV_prime) ^ pair.b;
    return seed % key_count;
}


HashMap hash_index_pair_array(IndexPairArray *idx_pairs, Memory *memory) {
    IndexPair *pairs = idx_pairs->pairs;
    int pair_count = idx_pairs->pair_count; 
    int hash_table_size = 4 * pair_count;
    int *hash_indices = (int *)m_alloc(memory, sizeof(int) * hash_table_size);
    IndexPair *hash_pairs = (IndexPair *) m_alloc(memory, sizeof(IndexPair) * hash_table_size);
    int table_ptr = 0;
    for(int i = 0; i < hash_table_size; ++i) {
        hash_indices[i] = -1;
    }
    for(int i = 0; i < pair_count; ++i) {
        IndexPair pair = pairs[i];
        uint64_t seed = 1;
        uint64_t idx;
        while(true) {
            idx = seed_hash(pair, hash_table_size, seed);
            if(hash_indices[idx] == -1) {
                break;
            }
            ++seed;
        }
        hash_indices[idx] = table_ptr;
        hash_pairs[table_ptr++] = pair;
    }
    return {hash_indices, hash_pairs, hash_table_size, table_ptr};
}


bool in_hash_map(IndexPair *pair, HashMap *hash_map) {
    if(hash_map->pair_count == 0) {
        return false;
    }
    uint64_t seed = 1;
    do {
        uint64_t idx = seed_hash(*pair, hash_map->size, seed);
        int pair_idx = hash_map->indices[idx];
        if(pair_idx == -1) {
            return false;
        }
        else {
            IndexPair *idxed_pair = hash_map->pairs + pair_idx;
            if(idxed_pair->a == pair->a && idxed_pair->b == pair->b) {
                return true;
            }
        }
        ++seed;
    } while(true);
}


IndexPairArray sweep_and_prune(
    BoundingBox *bboxes,
    int *x_bound_idxs,
    int *y_bound_idxs,
    int bbox_count,
    Memory *memory
) {
    
    IndexedInterval *xs = (IndexedInterval *)m_alloc(memory, sizeof(IndexedInterval) * bbox_count);
    IndexedInterval *ys = (IndexedInterval *)m_alloc(memory, sizeof(IndexedInterval) * bbox_count);
    
    for(int i = 0; i < bbox_count; ++i) {
        BoundingBox bbox = bboxes[i];
        int x_bound_idx = x_bound_idxs[i];
        int y_bound_idx = y_bound_idxs[i];
        xs[x_bound_idx] = {bbox.min_point.x, bbox.max_point.x, i};
        ys[y_bound_idx] = {bbox.min_point.y, bbox.max_point.y, i};
    }
    
    insertion_sort(xs, bbox_count);
    insertion_sort(ys, bbox_count);
    
    for(int i = 0; i < bbox_count; ++i) {
        x_bound_idxs[xs[i].index] = i;
    }
    for(int i = 0; i < bbox_count; ++i) {
        y_bound_idxs[ys[i].index] = i;
    }
    
    IndexPairArray x_inters = inter_axis(xs, bbox_count, memory);
    IndexPairArray y_inters = inter_axis(ys, bbox_count, memory);
    
    HashMap y_inters_hash_map = hash_index_pair_array(&y_inters, memory);
    
    int pair_count = 0;
    for(int i = 0; i < x_inters.pair_count; ++i) {
        IndexPair *idx_pair = x_inters.pairs + i;
        x_inters.pairs[pair_count] = *idx_pair;
        pair_count += in_hash_map(idx_pair, &y_inters_hash_map);
    }
    x_inters.pair_count = pair_count;
    
    return x_inters;
}


void narrow_phase(Vec2 *positions, double *radii, IndexPairArray *collisions) {
    int write_idx = 0;
    for(int i = 0; i < collisions->pair_count; ++i) {
        IndexPair pair = collisions->pairs[i];
        Vec2 pos_a = positions[pair.a];
        Vec2 pos_b = positions[pair.b];
        double dx = pos_a.x - pos_b.x;
        double dy = pos_a.y - pos_b.y;
        double touch_dist = radii[pair.a] + radii[pair.b];
        if(dx * dx + dy * dy < touch_dist * touch_dist) {
            collisions->pairs[write_idx++] = pair;
        }
    }
    collisions->pair_count = write_idx;
}
