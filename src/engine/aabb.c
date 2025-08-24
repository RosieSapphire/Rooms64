#include "t3d_ext.h"

#include "engine/aabb.h"

static sprite_t *aabb_spr = NULL;

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

#ifdef AABB_RENDER
void aabb_render(const struct aabb *bb, const uint32_t color)
{
        T3DVertPacked *v;
        T3DMat4FP *mtxfp;
        int i;

        if (!aabb_spr)
                aabb_spr = sprite_load("rom:/aabb_border.ia4.sprite");

        v = malloc_uncached(sizeof(*v) * 12);

        /* Bottom A */
        v[0].posA[0] = bb->min.v[0] * MODEL_SCALE;
        v[0].posA[1] = bb->min.v[1] * MODEL_SCALE;
        v[0].posA[2] = bb->min.v[2] * MODEL_SCALE;

        v[0].posB[0] = bb->max.v[0] * MODEL_SCALE;
        v[0].posB[1] = bb->min.v[1] * MODEL_SCALE;
        v[0].posB[2] = bb->min.v[2] * MODEL_SCALE;

        /* Bottom B */
        v[1].posA[0] = bb->max.v[0] * MODEL_SCALE;
        v[1].posA[1] = bb->max.v[1] * MODEL_SCALE;
        v[1].posA[2] = bb->min.v[2] * MODEL_SCALE;

        v[1].posB[0] = bb->min.v[0] * MODEL_SCALE;
        v[1].posB[1] = bb->max.v[1] * MODEL_SCALE;
        v[1].posB[2] = bb->min.v[2] * MODEL_SCALE;

        /* Top A */
        v[2].posA[0] = bb->min.v[0] * MODEL_SCALE;
        v[2].posA[1] = bb->min.v[1] * MODEL_SCALE;
        v[2].posA[2] = bb->max.v[2] * MODEL_SCALE;

        v[2].posB[0] = bb->max.v[0] * MODEL_SCALE;
        v[2].posB[1] = bb->min.v[1] * MODEL_SCALE;
        v[2].posB[2] = bb->max.v[2] * MODEL_SCALE;

        /* Top B */
        v[3].posA[0] = bb->max.v[0] * MODEL_SCALE;
        v[3].posA[1] = bb->max.v[1] * MODEL_SCALE;
        v[3].posA[2] = bb->max.v[2] * MODEL_SCALE;

        v[3].posB[0] = bb->min.v[0] * MODEL_SCALE;
        v[3].posB[1] = bb->max.v[1] * MODEL_SCALE;
        v[3].posB[2] = bb->max.v[2] * MODEL_SCALE;

        /* Front A */
        v[4].posA[0] = bb->min.v[0] * MODEL_SCALE;
        v[4].posA[1] = bb->min.v[1] * MODEL_SCALE;
        v[4].posA[2] = bb->min.v[2] * MODEL_SCALE;

        v[4].posB[0] = bb->max.v[0] * MODEL_SCALE;
        v[4].posB[1] = bb->min.v[1] * MODEL_SCALE;
        v[4].posB[2] = bb->min.v[2] * MODEL_SCALE;

        /* Front B */
        v[5].posA[0] = bb->max.v[0] * MODEL_SCALE;
        v[5].posA[1] = bb->min.v[1] * MODEL_SCALE;
        v[5].posA[2] = bb->max.v[2] * MODEL_SCALE;

        v[5].posB[0] = bb->min.v[0] * MODEL_SCALE;
        v[5].posB[1] = bb->min.v[1] * MODEL_SCALE;
        v[5].posB[2] = bb->max.v[2] * MODEL_SCALE;

        /* Back A */
        v[6].posA[0] = bb->min.v[0] * MODEL_SCALE;
        v[6].posA[1] = bb->max.v[1] * MODEL_SCALE;
        v[6].posA[2] = bb->min.v[2] * MODEL_SCALE;

        v[6].posB[0] = bb->max.v[0] * MODEL_SCALE;
        v[6].posB[1] = bb->max.v[1] * MODEL_SCALE;
        v[6].posB[2] = bb->min.v[2] * MODEL_SCALE;

        /* Back B */
        v[7].posA[0] = bb->max.v[0] * MODEL_SCALE;
        v[7].posA[1] = bb->max.v[1] * MODEL_SCALE;
        v[7].posA[2] = bb->max.v[2] * MODEL_SCALE;

        v[7].posB[0] = bb->min.v[0] * MODEL_SCALE;
        v[7].posB[1] = bb->max.v[1] * MODEL_SCALE;
        v[7].posB[2] = bb->max.v[2] * MODEL_SCALE;

        /* Left A */
        v[8].posA[0] = bb->max.v[0] * MODEL_SCALE;
        v[8].posA[1] = bb->min.v[1] * MODEL_SCALE;
        v[8].posA[2] = bb->min.v[2] * MODEL_SCALE;

        v[8].posB[0] = bb->max.v[0] * MODEL_SCALE;
        v[8].posB[1] = bb->max.v[1] * MODEL_SCALE;
        v[8].posB[2] = bb->min.v[2] * MODEL_SCALE;

        /* Left B */
        v[9].posA[0] = bb->max.v[0] * MODEL_SCALE;
        v[9].posA[1] = bb->max.v[1] * MODEL_SCALE;
        v[9].posA[2] = bb->max.v[2] * MODEL_SCALE;

        v[9].posB[0] = bb->max.v[0] * MODEL_SCALE;
        v[9].posB[1] = bb->min.v[1] * MODEL_SCALE;
        v[9].posB[2] = bb->max.v[2] * MODEL_SCALE;

        /* Right A */
        v[10].posA[0] = bb->min.v[0] * MODEL_SCALE;
        v[10].posA[1] = bb->min.v[1] * MODEL_SCALE;
        v[10].posA[2] = bb->min.v[2] * MODEL_SCALE;

        v[10].posB[0] = bb->min.v[0] * MODEL_SCALE;
        v[10].posB[1] = bb->max.v[1] * MODEL_SCALE;
        v[10].posB[2] = bb->min.v[2] * MODEL_SCALE;

        /* Right B */
        v[11].posA[0] = bb->min.v[0] * MODEL_SCALE;
        v[11].posA[1] = bb->max.v[1] * MODEL_SCALE;
        v[11].posA[2] = bb->max.v[2] * MODEL_SCALE;

        v[11].posB[0] = bb->min.v[0] * MODEL_SCALE;
        v[11].posB[1] = bb->min.v[1] * MODEL_SCALE;
        v[11].posB[2] = bb->max.v[2] * MODEL_SCALE;

        for (i = 0; i < 12; ++i) {
                v[i].rgbaA = color;
                v[i].rgbaB = color;
                v[i].normA = 1;
                v[i].normB = 1;

                if (!(i & 1)) {
                        v[i].stA[0] = 0x0;
                        v[i].stA[1] = 0x0;
                        v[i].stB[0] = aabb_spr->width << 5;
                        v[i].stB[1] = 0x0;
                } else {
                        v[i].stA[0] = aabb_spr->width << 5;
                        v[i].stA[1] = aabb_spr->height << 5;
                        v[i].stB[0] = 0x0;
                        v[i].stB[1] = aabb_spr->height << 5;
                }
        }

        mtxfp = malloc_uncached(sizeof(*mtxfp));
        {
                T3DVec3 s, r, t;

                s = t3d_vec3_one();
                r = t3d_vec3_zero();
                t3d_vec3_scale(&t, &bb->pos_offset, MODEL_SCALE);
                t3d_mat4fp_from_srt_euler(mtxfp, s.v, r.v, t.v);
        }

        rdpq_sync_pipe();
        rdpq_sprite_upload(TILE0, aabb_spr, NULL);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
        rdpq_mode_filter(FILTER_POINT);
        rdpq_mode_alphacompare(1);
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH |
                                T3D_FLAG_TEXTURED);
        t3d_light_set_ambient((uint8_t[4]){0xFF, 0xFF, 0xFF, 0xFF});
        t3d_light_set_count(0);
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
#else
void aabb_render(__attribute__((unused))const struct aabb *bb,
                 __attribute__((unused))const uint32_t color)
{
        return;
}
#endif
