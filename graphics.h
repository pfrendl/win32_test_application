#pragma once

#include "win32_definitions.h"
#include "collision_detection.h"


void render_scene(win32_offscreen_buffer *buffer, Vec2 *cam_pos, double zoom, Vec2 *positions, double *radii, int circle_count, IndexPairArray *collisions, Memory *memory);
