#include "graphics.h"
#include "math.h"


void render_circle(win32_offscreen_buffer *buffer, Vec2 *cam_pos, double zoom, Vec2 *origin, double radius, uint32_t color) {
    Vec2 principal_point = {buffer->width / 2.0, buffer->height / 2.0};
    
    double img_radius = zoom * radius;
    
    int ox = zoom * (origin->v[0] - cam_pos->v[0]) + principal_point.v[0];
    int oy = zoom * (origin->v[1] - cam_pos->v[1]) + principal_point.v[1];
    
    int min_i = max(oy - int(img_radius), 0) - oy;
    int max_i = min(oy + int(img_radius) + 1, buffer->height) - oy;
    
    
    for(int i = min_i; i < max_i; ++i) {
        double sqrt_val = sqrt(img_radius * img_radius - i * i);
        int min_j = max(int(-sqrt_val) + ox, 0);
        int max_j = min(int(sqrt_val) + ox + 1, buffer->width);
        uint32_t *row = (uint32_t *)(buffer->memory) + (oy + i) * buffer->width;
        for(int j = min_j; j < max_j; ++j) {
            row[j] = color;
        }
    }
}


void render_scene(win32_offscreen_buffer *buffer, Vec2 *cam_pos, double zoom, Vec2 *positions, double *radii, int circle_count, IndexPairArray *collisions, Memory *memory) {
    
    for(int i = 0; i < buffer->height; ++i) {
        uint32_t *row = (uint32_t *)(buffer->memory) + i * buffer->width;
        for(int j = 0; j < buffer->width; ++j) {
             row[j] = 0x004d4d4d;
        }
    }
    
    Vec2 origin = {0, 0};
    double map_radius = 1.0;
    render_circle(buffer, cam_pos, zoom, &origin, map_radius, 0x00ffffff);
    
    bool *paint_red = (bool *)m_alloc(memory, sizeof(bool) * circle_count);
    for(int i = 0; i < circle_count; ++i) {
        paint_red[i] = false;
    }
    for(int i = 0; i < collisions->pair_count; ++i) {
        IndexPair *pair = collisions->pairs + i;
        paint_red[pair->a] = true;
        paint_red[pair->b] = true;
    }
    
    for(int i = 0; i < circle_count; ++i) {
        uint32_t color = paint_red[i] ? 0x00ff0000 : 0x00000000;
        render_circle(buffer, cam_pos, zoom, positions + i, radii[i], color);
    }
}
