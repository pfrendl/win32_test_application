#pragma once

#include <stdint.h>
#include <math.h>


struct Vec2 {
    double v[2];
};


struct Vec3 {
    double v[3];
};


struct Mat3x3 {
    double v[3][3];
};


struct BoundingBox {
    Vec2 min_point;
    Vec2 max_point;
};


struct Memory {
    void *data;
    size_t ptr;
    size_t size;
};


Memory m_create(size_t size);
void m_destroy(Memory *memory);
void *m_alloc(Memory *memory, size_t size);
void m_free(Memory *memory);


Mat3x3 matmul(Mat3x3 *a, Mat3x3 *b);
