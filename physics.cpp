#include "physics.h"


Vec2 *calculate_forces(
    Vec2 *positions,
    double  *radii,
    int circle_count,
    IndexPairArray *collisions,
    double firmness,
    double map_radius,
    Memory *memory
) {
    Vec2 *forces = (Vec2 *)m_alloc(memory, sizeof(Vec2) * circle_count);
    
    double joint_firmness = firmness * firmness / (firmness + firmness);
    
    for(int i = 0; i < circle_count; ++i) {
        Vec2 pos = positions[i];
        
        double offset_len = sqrt(pos.x * pos.x + pos.y * pos.y);
        double dist_to_origin = offset_len + radii[i];
        double out_of_bounds_depth = map_radius - dist_to_origin;
        out_of_bounds_depth *= out_of_bounds_depth < 0;
        double force_len = joint_firmness * out_of_bounds_depth;
        
        offset_len += offset_len == 0;
        Vec2 offset_dir = {pos.x / offset_len, pos.y / offset_len};
        
        forces[i].x = force_len * offset_dir.x;
        forces[i].y = force_len * offset_dir.y;
    }
    
    for(int i = 0; i < collisions->pair_count; ++i) {
        IndexPair pair = collisions->pairs[i];
        Vec2 pos_a = positions[pair.a];
        Vec2 pos_b = positions[pair.b];
        Vec2 ab_vec = {pos_b.x - pos_a.x, pos_b.y - pos_a.y};
        
        double dist = sqrt(ab_vec.x * ab_vec.x + ab_vec.y * ab_vec.y);
        double touch_dist = radii[pair.a] + radii[pair.b];
        double inter_depth = touch_dist - dist;
        double force_len = joint_firmness *  inter_depth;
        
        dist += dist == 0;
        Vec2 ab_dir = {ab_vec.x / dist, ab_vec.y / dist};
        Vec2 force = {force_len * ab_vec.x, force_len * ab_vec.y};
        
        forces[pair.a].x -= force.x;
        forces[pair.a].y -= force.y;
        forces[pair.b].x += force.x;
        forces[pair.b].y += force.y;
    }
    
    return forces;
}

void apply_forces(Vec2 *positions, Vec2 *velocities, double *weights, Vec2 *forces, int circle_count, double velocity_dampening, double delta_t) {
    for(int i = 0; i < circle_count; ++i) {
        velocities[i].x += delta_t * (forces[i].x / weights[i] - velocity_dampening * velocities[i].x);
        velocities[i].y += delta_t * (forces[i].y / weights[i] - velocity_dampening * velocities[i].y);
        positions[i].x += delta_t * velocities[i].x;
        positions[i].y += delta_t * velocities[i].y;
    }
}
