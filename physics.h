#pragma once

#include "collision_detection.h"


Vec2 *calculate_forces(
    Vec2 *positions,
    double  *radii,
    int circle_count,
    IndexPairArray *collisions,
    double firmness,
    double map_radius,
    Memory *memory
);

void apply_forces(Vec2 *positions, Vec2 *velocities, double *weights, Vec2 *forces, int circle_count, double velocity_dampening, double delta_t);
