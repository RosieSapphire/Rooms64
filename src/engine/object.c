#include "engine/object.h"

#include "t3d_ext.h"

struct object object_create(const char *mdl_path, const T3DVec3 *scl,
                            const T3DVec3 *rot_eul, const T3DVec3 *pos,
                            void (*update_func)(struct object *, const float),
                            void *parent)
{
        struct object object;

        object.mdl = t3d_model_load(mdl_path);
        object.mtx = malloc_uncached(sizeof(*object.mtx));

        rspq_block_begin();
        t3d_matrix_push(object.mtx);
        t3d_model_draw(object.mdl);
        t3d_matrix_pop(1);
        object.dl = rspq_block_end();

        if (pos)
                object.position_a = *pos;
        else
                object.position_a = t3d_vec3_zero();
        object.position_b = object.position_a;

        if (rot_eul)
                object.rotation_euler_a = *rot_eul;
        else
                object.rotation_euler_a = t3d_vec3_zero();
        object.rotation_euler_b = object.rotation_euler_a;

        if (scl)
                object.scale_a = *scl;
        else
                object.scale_a = t3d_vec3_one();
        object.scale_b = object.scale_a;

        object.update_function = update_func;

        if (parent)
                object.parent = parent;

        return object;
}

void object_set_transforms(struct object *o, const T3DVec3 *scl,
                           const T3DVec3 *rot, const T3DVec3 *pos,
                           const uint8_t interpolate_flags)
{
        if (interpolate_flags & INTERP_POS)
                o->position_a = o->position_b;

        if (interpolate_flags & INTERP_ROT)
                o->rotation_euler_a = o->rotation_euler_b;

        if (interpolate_flags & INTERP_SCL)
                o->scale_a = o->scale_b;

        if (scl)
                o->scale_b = *scl;

        if (rot)
                o->rotation_euler_b = *rot;

        if (pos)
                o->position_b = *pos;

        if (!interpolate_flags)
                return;

        if (!(interpolate_flags & INTERP_SCL))
                o->scale_a = o->scale_b;

        if (!(interpolate_flags & INTERP_ROT))
                o->rotation_euler_a = o->rotation_euler_b;

        if (!(interpolate_flags & INTERP_POS))
                o->position_a = o->position_b;
}

void object_render(const struct object *obj, const float st)
{
        T3DVec3 scale, roteul, pos;

        t3d_vec3_lerp(&pos, &obj->position_a, &obj->position_b, st);
        t3d_vec3_scale(&pos, &pos, MODEL_SCALE);
        t3d_vec3_lerp(&roteul, &obj->rotation_euler_a,
                      &obj->rotation_euler_b, st);
        t3d_vec3_lerp(&scale, &obj->scale_a, &obj->scale_b, st);
        t3d_mat4fp_from_srt_euler(obj->mtx, scale.v, roteul.v, pos.v);
        rspq_block_run(obj->dl);
}

void object_destroy(struct object *obj)
{
        rspq_block_free(obj->dl);
        free_uncached(obj->mtx);
        t3d_model_free(obj->mdl);
}
