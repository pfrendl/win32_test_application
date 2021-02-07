#include <stdlib.h>

#include "datatypes.h"


Memory m_create(size_t size) {
    Memory memory;
    memory.data = malloc(size);
    memory.ptr = 0;
    memory.size = size;
    return memory;
}


void m_destroy(Memory *memory) {
    free(memory->data);
}


void *m_alloc(Memory *memory, size_t size) {
    size_t ptr = memory->ptr + size;
    void *result;
    if (ptr <= memory->size) {
        result = (void *)((char *)(memory->data) + memory->ptr);
        memory->ptr = ptr;
    }
    else {
        result = 0;
    }
    return result;
}


void m_free(Memory *memory) {
    memory->ptr = 0;
}


Mat3x3 matmul(Mat3x3 *a, Mat3x3 *b) {
    Mat3x3 result;
    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < 3; ++j) {
            result.v[i][j] = 0;
            for(int k = 0; k < 3; ++k) {
                result.v[i][j] += a->v[i][k] * b->v[k][j];
            }
        }
    }
    return result;
}
