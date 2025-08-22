#pragma once

#include <t3d/t3d.h>

struct aabb {
        T3DVec3 pos_offset;
        T3DVec3 min;
        T3DVec3 max;
};

struct aabb aabb_make(const T3DVec3 *pos, const T3DVec3 *min,
                      const T3DVec3 *max);
bool aabb_does_point_intersect(const struct aabb *bb, const T3DVec3 *p);
void aabb_render(__attribute__((unused))const struct aabb *bb);
