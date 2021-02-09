#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "collision_detection.h"
#include "random.h"


int main() {
    // insertion_sort
    {
        IndexedInterval inputs[] = {{0.1, 1.1, 0}, {0.05, 0.07, 1}, {0.5, 1.0, 2}, {0.4, 0.5, 3}, {1.4, 3.0, 4}};
        IndexedInterval expected_results[] = {inputs[1], inputs[0], inputs[3], inputs[2], inputs[4]};
        int circle_count = sizeof(inputs) / sizeof(IndexedInterval);
        insertion_sort(inputs, circle_count);
        for(int i = 0; i < circle_count; ++i) {
            IndexedInterval input = inputs[i];
            IndexedInterval expected_result = expected_results[i];
            assert(input.minimum == expected_result.minimum);
            assert(input.maximum == expected_result.maximum);
            assert(input.index == expected_result.index);
        }
    }
    
    // insertion_sort
    {
        constexpr int input_count = 10000;
        IndexedInterval inputs[input_count];
        for(int i = 0; i < input_count; ++i) {
            inputs[i] = {randu(), randu(), i};
        }
        
        clock_t start = clock();
        
        insertion_sort(inputs, input_count);
        
        clock_t end = clock();
        double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("insertion_sort time: %f\n", cpu_time_used);
        
        for(int i = 0; i < input_count - 1; ++i) {
            IndexedInterval iv = inputs[i];
            assert(iv.minimum <= inputs[i + 1].minimum);
        }
    }
    
    // quicksort
    {
        constexpr int input_count = 10000;
        IndexedInterval inputs[input_count];
        for(int i = 0; i < input_count; ++i) {
            inputs[i] = {randu(), randu(), i};
        }
        
        clock_t start = clock();
        
        quicksort(inputs, 0, input_count);
        
        clock_t end = clock();
        double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("quicksort time: %f\n", cpu_time_used);
        
        for(int i = 0; i < input_count - 1; ++i) {
            IndexedInterval iv = inputs[i];
            assert(iv.minimum <= inputs[i + 1].minimum);
        }
    }
    
    // inter_axis
    {
        IndexedInterval ivs[] = {{0.05, 0.4, 0}, {0.08, 0.7, 2}, {0.1, 0.5, 1}, {0.6, 2.0, 5}};
        IndexPair idx_pairs[] = {{0, 2}, {0, 1}, {1, 2}, {2, 5}};
        IndexPairArray expected_results = {idx_pairs, sizeof(idx_pairs) / sizeof(IndexPair)};
        int bbox_count = sizeof(ivs) / sizeof(IndexedInterval);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = inter_axis(ivs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == expected_results.pair_count);
        for(int i = 0; i < idx_pair_arr.pair_count; ++i) {
            IndexPair *input = idx_pair_arr.pairs + i;
            IndexPair *expected_result = expected_results.pairs + i;
            assert(input->a == expected_result->a && input->b == expected_result->b);
        }
        m_destroy(&memory);
    }
    
    // hash_index_pair_array
    {
        IndexPair idx_pairs[] = {{7, 8}, {0, 2}, {0, 1}, {1, 2}, {2, 5}};
        int pair_count = sizeof(idx_pairs) / sizeof(IndexPair);
        IndexPairArray idx_pair_arr = {idx_pairs, pair_count};
        Memory memory = m_create(1024);
        HashMap hash_map = hash_index_pair_array(&idx_pair_arr, &memory);
        assert(hash_map.size == 4 * pair_count);
        assert(hash_map.pair_count == pair_count);
        for(int i = 0; i < pair_count; ++i) {
            IndexPair pair_i = idx_pairs[i];
            bool found = false;
            for(int j = 0; j < pair_count; ++j) {
                IndexPair pair_j = hash_map.pairs[j];
                if(pair_i.a == pair_j.a && pair_i.b == pair_j.b) {
                    found = true;
                    break;
                }
            }
            assert(found);
        }
        m_destroy(&memory);
    }
    
    // in_hash_map
    {
        IndexPair idx_pairs[] = {{7, 8}, {0, 2}, {0, 1}, {1, 2}, {2, 5}};
        int pair_count = sizeof(idx_pairs) / sizeof(IndexPair);
        IndexPairArray idx_pair_arr = {idx_pairs, pair_count};
        Memory memory = m_create(1024);
        HashMap hash_map = hash_index_pair_array(&idx_pair_arr, &memory);
        assert(hash_map.size == 4 * pair_count);
        assert(hash_map.pair_count == pair_count);
        for(int i = 0; i < pair_count; ++i) {
            assert(in_hash_map(idx_pairs + i, &hash_map));
        }
        IndexPair excluded_pair = {10, 15};
        assert(!in_hash_map(&excluded_pair, &hash_map));
        
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0, 0}, {1, 1}},
            {{2, 0}, {3, 1}},
        };
        int x_bound_idxs[] = {0, 1};
        int y_bound_idxs[] = {0, 1};
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == 0);
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0, 0}, {1, 1}},
            {{0, 2}, {1, 3}},
        };
        int x_bound_idxs[] = {0, 1};
        int y_bound_idxs[] = {0, 1};
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == 0);
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0, 0}, {1, 1}},
            {{0, 0}, {1, 1}},
        };
        int x_bound_idxs[] = {0, 1};
        int y_bound_idxs[] = {0, 1};
        IndexPair expected_results[] = {{0, 1}};
        int pair_count = sizeof(expected_results) / sizeof(IndexPair);
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == pair_count);
        for(int i = 0; i < idx_pair_arr.pair_count; ++i) {
            IndexPair a = idx_pair_arr.pairs[i];
            IndexPair b = expected_results[i];
            assert(a.a == b.a && a.b == b.b);
        }
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0, 0}, {1, 1}},
            {{0.5, 0.5}, {1.5, 1.5}},
        };
        int x_bound_idxs[] = {0, 1};
        int y_bound_idxs[] = {0, 1};
        IndexPair expected_results[] = {{0, 1}};
        int pair_count = sizeof(expected_results) / sizeof(IndexPair);
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == pair_count);
        for(int i = 0; i < idx_pair_arr.pair_count; ++i) {
            IndexPair a = idx_pair_arr.pairs[i];
            IndexPair b = expected_results[i];
            assert(a.a == b.a && a.b == b.b);
        }
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0.5, 0.5}, {1.5, 1.5}},
            {{0, 0}, {1, 1}},
        };
        int x_bound_idxs[] = {0, 1};
        int y_bound_idxs[] = {0, 1};
        IndexPair expected_results[] = {{0, 1}};
        int pair_count = sizeof(expected_results) / sizeof(IndexPair);
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == pair_count);
        for(int i = 0; i < idx_pair_arr.pair_count; ++i) {
            IndexPair a = idx_pair_arr.pairs[i];
            IndexPair b = expected_results[i];
            assert(a.a == b.a && a.b == b.b);
        }
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0.5, 0.5}, {1.5, 1.5}},
            {{0, 0}, {1, 1}},
            {{5, 0}, {5, 1}},
        };
        int x_bound_idxs[] = {0, 1, 2};
        int y_bound_idxs[] = {0, 1, 2};
        IndexPair expected_results[] = {{0, 1}};
        int pair_count = sizeof(expected_results) / sizeof(IndexPair);
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == pair_count);
        for(int i = 0; i < idx_pair_arr.pair_count; ++i) {
            IndexPair a = idx_pair_arr.pairs[i];
            IndexPair b = expected_results[i];
            assert(a.a == b.a && a.b == b.b);
        }
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0.5, 0.5}, {1.5, 1.5}},
            {{0.7, 0.5}, {1.7, 1.5}},
            {{1.5, 0.5}, {2.5, 1.5}},
        };
        int x_bound_idxs[] = {0, 1, 2};
        int y_bound_idxs[] = {0, 1, 2};
        IndexPair expected_results[] = {{0, 1}, {1, 2}};
        int pair_count = sizeof(expected_results) / sizeof(IndexPair);
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == pair_count);
        for(int i = 0; i < idx_pair_arr.pair_count; ++i) {
            IndexPair a = idx_pair_arr.pairs[i];
            IndexPair b = expected_results[i];
            assert(a.a == b.a && a.b == b.b);
        }
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0.5, 0.5}, {1.5, 1.5}},
            {{1.5, 0.5}, {2.5, 1.5}},
            {{0.7, 0.5}, {1.7, 1.5}},
        };
        int x_bound_idxs[] = {0, 1, 2};
        int y_bound_idxs[] = {0, 1, 2};
        IndexPair expected_results[] = {{0, 2}, {1, 2}};
        int pair_count = sizeof(expected_results) / sizeof(IndexPair);
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        assert(idx_pair_arr.pair_count == pair_count);
        for(int i = 0; i < idx_pair_arr.pair_count; ++i) {
            IndexPair a = idx_pair_arr.pairs[i];
            IndexPair b = expected_results[i];
            assert(a.a == b.a && a.b == b.b);
        }
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0.5, 0.4}, {1.5, 1.2}},
            {{1.4, 0.6}, {2.5, 1.6}},
            {{0.7, 0.5}, {1.7, 1.5}},
        };
        int x_bound_idxs[] = {0, 1, 2};
        int y_bound_idxs[] = {0, 1, 2};
        int expected_results_x[] = {0, 2, 1};
        int expected_results_y[] = {0, 2, 1};
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        for(int i = 0; i < bbox_count; ++i) {
            assert(x_bound_idxs[i] == expected_results_x[i]);
            assert(y_bound_idxs[i] == expected_results_y[i]);
        }
        m_destroy(&memory);
    }
    
    // sweep_and_prune
    {
        BoundingBox bboxes[] = {
            {{0.5, 0.7}, {2.5, 1.2}},
            {{1.4, 0.5}, {1.5, 1.6}},
            {{0.7, 0.6}, {1.7, 1.5}},
        };
        int bbox_count = sizeof(bboxes) / sizeof(BoundingBox);
        int x_bound_idxs[] = {0, 1, 2};
        int y_bound_idxs[] = {0, 1, 2};
        int expected_results_x[] = {0, 2, 1};
        int expected_results_y[] = {2, 0, 1};
        
        Memory memory = m_create(1024);
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, bbox_count, &memory);
        for(int i = 0; i < bbox_count; ++i) {
            assert(x_bound_idxs[i] == expected_results_x[i]);
            assert(y_bound_idxs[i] == expected_results_y[i]);
        }
        m_destroy(&memory);
    }
    
    {
        constexpr int circle_count = 1000;
        Vec2 positions[circle_count];
        double radii[circle_count];
        for(int i = 0; i < circle_count; ++i) {
            positions[i] = random_normal({0, 0}, 0.2);
            radii[i] = random_uniform(0.003, 0.01);
        }
        int x_bound_idxs[circle_count];
        int y_bound_idxs[circle_count];
        for(int i = 0; i < circle_count; ++i) {
            x_bound_idxs[i] = i;
            y_bound_idxs[i] = i;
        }
        
        Memory memory = m_create(1024 * 1024 * 10);
        BoundingBox *bboxes = (BoundingBox *)m_alloc(&memory, sizeof(BoundingBox) * circle_count);
        for(int i = 0; i < circle_count; ++i) {
            Vec2 *origin = positions + i;
            double radius = radii[i];
            bboxes[i] = {{origin->x - radius, origin->y - radius}, {origin->x + radius, origin->y + radius}};
        }
        
        IndexPairArray idx_pair_arr = sweep_and_prune(bboxes, x_bound_idxs, y_bound_idxs, circle_count, &memory);
        
        IndexedInterval *xs = (IndexedInterval *)m_alloc(&memory, sizeof(IndexedInterval) * circle_count);
        IndexedInterval *ys = (IndexedInterval *)m_alloc(&memory, sizeof(IndexedInterval) * circle_count);
        for(int i = 0; i < circle_count; ++i) {
            BoundingBox *bbox = bboxes + i;
            int x_bound_idx = x_bound_idxs[i];
            int y_bound_idx = y_bound_idxs[i];
            xs[x_bound_idx] = {bbox->min_point.x, bbox->max_point.x, i};
            ys[y_bound_idx] = {bbox->min_point.y, bbox->max_point.y, i};
        }
        
        for(int i = 0; i < circle_count - 1; ++i) {
            assert(xs[i].minimum < xs[i + 1].minimum);
            assert(ys[i].minimum < ys[i + 1].minimum);
        }
        m_destroy(&memory);
    }
}
