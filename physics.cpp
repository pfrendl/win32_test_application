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
        
        double offset_len = sqrt(pos.v[0] * pos.v[0] + pos.v[1] * pos.v[1]);
        double dist_to_origin = offset_len + radii[i];
        double out_of_bounds_depth = map_radius - dist_to_origin;
        out_of_bounds_depth *= out_of_bounds_depth < 0;
        double force_len = joint_firmness * out_of_bounds_depth;
        
        offset_len += offset_len == 0;
        Vec2 offset_dir = {pos.v[0] / offset_len, pos.v[1] / offset_len};
        
        forces[i].v[0] = force_len * offset_dir.v[0];
        forces[i].v[1] = force_len * offset_dir.v[1];
    }
    
    for(int i = 0; i < collisions->pair_count; ++i) {
        IndexPair pair = collisions->pairs[i];
        Vec2 pos_a = positions[pair.a];
        Vec2 pos_b = positions[pair.b];
        Vec2 ab_vec = {pos_b.v[0] - pos_a.v[0], pos_b.v[1] - pos_a.v[1]};
        
        double dist = sqrt(ab_vec.v[0] * ab_vec.v[0] + ab_vec.v[1] * ab_vec.v[1]);
        double touch_dist = radii[pair.a] + radii[pair.b];
        double inter_depth = touch_dist - dist;
        double force_len = joint_firmness *  inter_depth;
        
        dist += dist == 0;
        Vec2 ab_dir = {ab_vec.v[0] / dist, ab_vec.v[1] / dist};
        Vec2 force = {force_len * ab_vec.v[0], force_len * ab_vec.v[1]};
        
        forces[pair.a].v[0] -= force.v[0];
        forces[pair.a].v[1] -= force.v[1];
        forces[pair.b].v[0] += force.v[0];
        forces[pair.b].v[1] += force.v[1];
    }
    
    return forces;
}

void apply_forces(Vec2 *positions, Vec2 *velocities, double *weights, Vec2 *forces, int circle_count, double velocity_dampening, double delta_t) {
    for(int i = 0; i < circle_count; ++i) {
        velocities[i].v[0] += delta_t * (forces[i].v[0] / weights[i] - velocity_dampening * velocities[i].v[0]);
        velocities[i].v[1] += delta_t * (forces[i].v[1] / weights[i] - velocity_dampening * velocities[i].v[1]);
        positions[i].v[0] += delta_t * velocities[i].v[0];
        positions[i].v[1] += delta_t * velocities[i].v[1];
    }
}
