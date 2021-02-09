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
