#ifndef ENGINE_OBJECT_H
#define ENGINE_OBJECT_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

enum {
        INTERP_POS = (1 << 0),
        INTERP_ROT = (1 << 1),
        INTERP_SCL = (1 << 2),
        INTERP_ALL = (INTERP_POS | INTERP_ROT | INTERP_SCL)
};

struct object {
        T3DModel *mdl;
        T3DMat4FP *mtx;
        rspq_block_t *dl;
        T3DVec3 position_a;
        T3DVec3 position_b;
        T3DVec3 rotation_euler_a;
        T3DVec3 rotation_euler_b;
        T3DVec3 scale_a;
        T3DVec3 scale_b;
        void (*update_function)(struct object *, const float);
        uint16_t sub_obj_index;
        void *parent;
};

struct object object_create(const char *mdl_path, const T3DVec3 *pos,
                            const T3DVec3 *rot_eul, const T3DVec3 *scl,
                            void (*update_func)(struct object *, const float),
                            void *parent);
void object_set_transforms(struct object *o, const T3DVec3 *scl,
                           const T3DVec3 *rot, const T3DVec3 *pos,
                           const uint8_t interpolate_flags);
void object_render(const struct object *obj, const float st);
void object_destroy(struct object *obj);

#endif /* ENGINE_OBJECT_H */
