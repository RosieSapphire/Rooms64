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

        v = malloc_uncached(sizeof(*v) * 12);

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

        /* Front */
        v[4].posA[0] = -16.f;
        v[4].posA[1] = -16.f;
        v[4].posA[2] = 0.f;
        v[4].rgbaA = 0xFF0000FF;
        v[4].normA = 1;
        v[4].posB[0] = 16.f;
        v[4].posB[1] = -16.f;
        v[4].posB[2] = 0.f;
        v[4].rgbaB = 0x00FF00FF;
        v[4].normB = 1;

        v[5].posA[0] = 16.f;
        v[5].posA[1] = -16.f;
        v[5].posA[2] = 32.f;
        v[5].rgbaA = 0x0000FFFF;
        v[5].normA = 1;
        v[5].posB[0] = -16.f;
        v[5].posB[1] = -16.f;
        v[5].posB[2] = 32.f;
        v[5].rgbaB = 0xFF00FFFF;
        v[5].normB = 1;

        /* Back */
        v[6].posA[0] = -16.f;
        v[6].posA[1] = 16.f;
        v[6].posA[2] = 0.f;
        v[6].rgbaA = 0xFF0000FF;
        v[6].normA = 1;
        v[6].posB[0] = 16.f;
        v[6].posB[1] = 16.f;
        v[6].posB[2] = 0.f;
        v[6].rgbaB = 0x00FF00FF;
        v[6].normB = 1;

        v[7].posA[0] = 16.f;
        v[7].posA[1] = 16.f;
        v[7].posA[2] = 32.f;
        v[7].rgbaA = 0x0000FFFF;
        v[7].normA = 1;
        v[7].posB[0] = -16.f;
        v[7].posB[1] = 16.f;
        v[7].posB[2] = 32.f;
        v[7].rgbaB = 0xFF00FFFF;
        v[7].normB = 1;

        /* Left */
        v[8].posA[0] = 16.f;
        v[8].posA[1] = -16.f;
        v[8].posA[2] = 0.f;
        v[8].rgbaA = 0xFF0000FF;
        v[8].normA = 1;
        v[8].posB[0] = 16.f;
        v[8].posB[1] = 16.f;
        v[8].posB[2] = 0.f;
        v[8].rgbaB = 0x00FF00FF;
        v[8].normB = 1;

        v[9].posA[0] = 16.f;
        v[9].posA[1] = 16.f;
        v[9].posA[2] = 32.f;
        v[9].rgbaA = 0x0000FFFF;
        v[9].normA = 1;
        v[9].posB[0] = 16.f;
        v[9].posB[1] = -16.f;
        v[9].posB[2] = 32.f;
        v[9].rgbaB = 0xFF00FFFF;
        v[9].normB = 1;

        /* Right */
        v[10].posA[0] = -16.f;
        v[10].posA[1] = -16.f;
        v[10].posA[2] = 0.f;
        v[10].rgbaA = 0xFF0000FF;
        v[10].normA = 1;
        v[10].posB[0] = -16.f;
        v[10].posB[1] = 16.f;
        v[10].posB[2] = 0.f;
        v[10].rgbaB = 0x00FF00FF;
        v[10].normB = 1;

        v[11].posA[0] = -16.f;
        v[11].posA[1] = 16.f;
        v[11].posA[2] = 32.f;
        v[11].rgbaA = 0x0000FFFF;
        v[11].normA = 1;
        v[11].posB[0] = -16.f;
        v[11].posB[1] = -16.f;
        v[11].posB[2] = 32.f;
        v[11].rgbaB = 0xFF00FFFF;
        v[11].normB = 1;

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
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH |
                                T3D_FLAG_CULL_BACK);
        t3d_matrix_push(mtxfp);
        t3d_vert_load(v, 0, 24);
        t3d_matrix_pop(1);

        /* Bottom */
        t3d_tri_draw(0, 2, 1);
        t3d_tri_draw(2, 0, 3);

        /* Top */
        t3d_tri_draw(4, 5, 6);
        t3d_tri_draw(6, 7, 4);

        /* Front */
        t3d_tri_draw(8, 9, 10);
        t3d_tri_draw(10, 11, 8);

        /* Back */
        t3d_tri_draw(12, 14, 13);
        t3d_tri_draw(14, 12, 15);

        /* Left */
        t3d_tri_draw(16, 17, 18);
        t3d_tri_draw(18, 19, 16);

        /* Right */
        t3d_tri_draw(20, 22, 21);
        t3d_tri_draw(22, 20, 23);

        t3d_tri_sync();

        free_uncached(mtxfp);
        free_uncached(v);
}
