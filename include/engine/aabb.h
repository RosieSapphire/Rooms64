#ifndef ENGINE_AABB_H
#define ENGINE_AABB_H

#include <t3d/t3d.h>

struct aabb {
        T3DVec3 pos_offset;
        T3DVec3 min;
        T3DVec3 max;
};

bool aabb_does_point_intersect(const struct aabb *bb, const T3DVec3 *p);
void aabb_render(const struct aabb *bb, const uint32_t color);

#endif /* ENGINE_AABB_H */
