#include "collision_detection.h"
#include <stdio.h>


void insertion_sort(IndexedValue *ivs, int iv_count) {
    for(int i = 0; i < iv_count; ++i) {
        IndexedValue tmp = ivs[i];
        int j = i;
        
        while(j > 0 && ivs[j - 1].value > tmp.value) {
            IndexedValue read_first = ivs[j - 1];
            ivs[j] = read_first;
            --j;
        }
        
        ivs[j] = tmp;
    }
}


int sorted_search(int elem, int *array, int len) {
    int start = 0;
    int end = len;
    int middle = (end + start) / 2;
    while(start < end) {
        int middle_elem = array[middle];
        if(middle_elem > elem) {
            end = middle;
        }
        else if(middle_elem < elem) {
            start = middle + 1;
        }
        else return middle;
        middle = (end + start) / 2;
    }
    return middle;
}

 
IndexPairArray inter_axis(IndexedValue *ivs, int bbox_count, Memory *memory) {
    int *open = (int *)m_alloc(memory, sizeof(int) * bbox_count);
    int open_count = 0;
    
    IndexPair *inters = (IndexPair *)m_alloc(memory, 0);
    int inter_count = 0;
    
    int iv_count = 2 * bbox_count;
    for(int i = 0; i < iv_count; ++i) {
        int idx = ivs[i].index;
        int found = sorted_search(idx, open, open_count);
        if(found < open_count && open[found] == idx) {
            for(int i = found; i < open_count; ++i) {
                open[i] = open[i + 1];
            }
            --open_count;
        }
        else {
            for(int i = 0; i < open_count; ++i) {
                m_alloc(memory, sizeof(IndexPairArray));
                int open_idx = open[i];
                if(open_idx < idx) {
                    inters[inter_count++] = {open_idx, idx};
                }
                else {
                    inters[inter_count++] = {idx, open_idx};
                }
            }
            for(int j = open_count; j > found; --j) {
                open[j] = open[j - 1];
            }
            open[found] = idx;
            ++open_count;
        }
    }
    
    return {inters, inter_count};
}


uint64_t seed_hash(char* key, int key_size, int key_count, uint64_t seed) {
	const char* end = key + key_size;
	while(key != end) {
		seed = (seed * FNV_prime) ^ *key++;
	}
	return seed % key_count;
}


HashMap hash_index_pair_array(IndexPairArray *idx_pairs, Memory *memory) {
    int hash_table_size = 4 * idx_pairs->pair_count;
    int *hash_indices = (int *)m_alloc(memory, sizeof(int) * hash_table_size);
    IndexPair *hash_pairs = (IndexPair *) m_alloc(memory, sizeof(IndexPair) * hash_table_size);
    int table_ptr = 0;
    for(int i = 0; i < hash_table_size; ++i) {
        hash_indices[i] = -1;
    }
    for(int i = 0; i < idx_pairs->pair_count; ++i) {
        hash_pairs[table_ptr] = idx_pairs->pairs[i];
        uint64_t seed = 1;
        do {
            uint64_t idx = seed_hash((char *)(idx_pairs->pairs + i), sizeof(IndexPair), hash_table_size, seed);
            if(hash_indices[idx] == -1) {
                hash_indices[idx] = table_ptr++;
                break;
            }
            ++seed;
        } while(true);
    }
    return {hash_indices, hash_pairs, hash_table_size, table_ptr};
}


bool in_hash_map(IndexPair *pair, HashMap *hash_map) {
    if(hash_map->pair_count == 0) {
        return false;
    }
    uint64_t seed = 1;
    do {
        uint64_t idx = seed_hash((char *)pair, sizeof(IndexPair), hash_map->size, seed);
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
    IndexPair *x_bound_idx_pairs,
    IndexPair *y_bound_idx_pairs,
    int bbox_count,
    Memory *memory
) {
    char buff[128];
    
    //sprintf(buff, "memory->ptr A: %zd\n", memory->ptr);
    //OutputDebugStringA(buff);
    
    int iv_count = 2 * bbox_count;
    IndexedValue *xs = (IndexedValue *)m_alloc(memory, sizeof(IndexedValue) * iv_count);
    IndexedValue *ys = (IndexedValue *)m_alloc(memory, sizeof(IndexedValue) * iv_count);
    for(int i = 0; i < bbox_count; ++i) {
        BoundingBox *bbox = bboxes + i;
        IndexPair *x_bound_idx_pair = x_bound_idx_pairs + i;
        IndexPair *y_bound_idx_pair = y_bound_idx_pairs + i;
        xs[x_bound_idx_pair->a] = {bbox->min_point.v[0], i};
        xs[x_bound_idx_pair->b] = {bbox->max_point.v[0], i};
        ys[y_bound_idx_pair->a] = {bbox->min_point.v[1], i};
        ys[y_bound_idx_pair->b] = {bbox->max_point.v[1], i};
        x_bound_idx_pair->a = -1;
        y_bound_idx_pair->a = -1;
    }
    insertion_sort(xs, iv_count);
    insertion_sort(ys, iv_count);
    for(int i = 0; i < iv_count; ++i) {
        int idx = xs[i].index;
        if(x_bound_idx_pairs[idx].a  == -1) {
            x_bound_idx_pairs[idx].a = i;
        }
        else {
            x_bound_idx_pairs[idx].b = i;
        }
    }
    for(int i = 0; i < iv_count; ++i) {
        int idx = ys[i].index;
        if(y_bound_idx_pairs[idx].a == -1) {
            y_bound_idx_pairs[idx].a = i;
        }
        else {
            y_bound_idx_pairs[idx].b = i;
        }
    }
    
    //sprintf(buff, "memory->ptr B: %zd\n", memory->ptr);
    //OutputDebugStringA(buff);
    
    IndexPairArray x_inters = inter_axis(xs, bbox_count, memory);
    
    //sprintf(buff, "memory->ptr BB: %zd\n", memory->ptr);
    //OutputDebugStringA(buff);
    
    IndexPairArray y_inters = inter_axis(ys, bbox_count, memory);
    
    IndexPairArray *a_inters;
    IndexPairArray *b_inters;
    if(x_inters.pair_count < y_inters.pair_count) {
        a_inters = &x_inters;
        b_inters = &y_inters;
    }
    else {
        a_inters = &y_inters;
        b_inters = &x_inters;
    }
    
    //sprintf(buff, "memory->ptr C: %zd\n", memory->ptr);
    //OutputDebugStringA(buff);
    
    HashMap a_inters_hash_map = hash_index_pair_array(a_inters, memory);
    a_inters->pair_count = 0;
    for(int i = 0; i < b_inters->pair_count; ++i) {
        IndexPair *idx_pair = b_inters->pairs + i;
        if(in_hash_map(idx_pair, &a_inters_hash_map)) {
            a_inters->pairs[a_inters->pair_count++] = *idx_pair;
        }
    }
    
    //sprintf(buff, "memory->ptr D: %zd\n", memory->ptr);
    //OutputDebugStringA(buff);
    
    return *a_inters;
}
