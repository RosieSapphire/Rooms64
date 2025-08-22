#include "t3d_ext.h"

#include "engine/aabb.h"

struct aabb aabb_make(const T3DVec3 *pos, const T3DVec3 *min,
                      const T3DVec3 *max)
{
        struct aabb bb;

        bb.pos_offset = *pos;
        bb.min = *min;
        bb.max = *max;

        return bb;
}

bool aabb_does_point_intersect(const struct aabb *bb, const T3DVec3 *p)
{
        int i;

        for (i = 0; i < 3; ++i) {
                if (p->v[i] < (bb->pos_offset.v[i] + bb->min.v[i]))
                        return false;

                if (p->v[i] > (bb->pos_offset.v[i] + bb->max.v[i]))
                        return false;
        }

        return true;
}

void aabb_render(__attribute__((unused))const struct aabb *bb)
{
        T3DVertPacked *v;
        T3DMat4FP *mtxfp;

        v = malloc_uncached(sizeof(*v) * 4);

        /* Bottom */
        v[0].posA[0] = -16.f;
        v[0].posA[1] = -16.f;
        v[0].posA[2] = 0.f;
        v[0].rgbaA = 0xFF0000FF;
        v[0].normA = 1;
        v[0].posB[0] = 16.f;
        v[0].posB[1] = -16.f;
        v[0].posB[2] = 0.f;
        v[0].rgbaB = 0x00FF00FF;
        v[0].normB = 1;

        v[1].posA[0] = 16.f;
        v[1].posA[1] = 16.f;
        v[1].posA[2] = 0.f;
        v[1].rgbaA = 0x0000FFFF;
        v[1].normA = 1;
        v[1].posB[0] = -16.f;
        v[1].posB[1] = 16.f;
        v[1].posB[2] = 0.f;
        v[1].rgbaB = 0xFF00FFFF;
        v[1].normB = 1;

        /* Top */
        v[2].posA[0] = -16.f;
        v[2].posA[1] = -16.f;
        v[2].posA[2] = 32.f;
        v[2].rgbaA = 0xFF0000FF;
        v[2].normA = 1;
        v[2].posB[0] = 16.f;
        v[2].posB[1] = -16.f;
        v[2].posB[2] = 32.f;
        v[2].rgbaB = 0x00FF00FF;
        v[2].normB = 1;

        v[3].posA[0] = 16.f;
        v[3].posA[1] = 16.f;
        v[3].posA[2] = 32.f;
        v[3].rgbaA = 0x0000FFFF;
        v[3].normA = 1;
        v[3].posB[0] = -16.f;
        v[3].posB[1] = 16.f;
        v[3].posB[2] = 32.f;
        v[3].rgbaB = 0xFF00FFFF;
        v[3].normB = 1;

        mtxfp = malloc_uncached(sizeof(*mtxfp));

        {
                T3DVec3 s, r, t;

                s = t3d_vec3_one();
                r = t3d_vec3_zero();
                t = t3d_vec3_zero();
                t3d_mat4fp_from_srt_euler(mtxfp, s.v, r.v, t.v);
        }

        rdpq_sync_pipe();
        rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH);
        t3d_matrix_push(mtxfp);
        t3d_vert_load(v, 0, 8);
        t3d_matrix_pop(1);
        t3d_tri_draw(0, 1, 2);
        t3d_tri_draw(2, 3, 0);
        t3d_tri_draw(4, 5, 6);
        t3d_tri_draw(6, 7, 4);
        t3d_tri_sync();

        free_uncached(mtxfp);
        free_uncached(v);
}
