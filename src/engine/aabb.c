#include "t3d_ext.h"

#include "engine/aabb.h"

static sprite_t *aabb_spr = NULL;

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

#ifdef AABB_RENDER
void aabb_render(const struct aabb *bb, const uint32_t color)
{
        T3DVertPacked *v;
        T3DMat4FP *mtxfp;

        if (!aabb_spr)
                aabb_spr = sprite_load("rom:/aabb_border.ia4.sprite");

        v = malloc_uncached(sizeof(*v) * 12);

        /* Bottom A */
        v[0].posA[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[0].posA[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[0].posA[2] = bb->min.v[2] + bb->pos_offset.v[2];
        v[0].posB[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[0].posB[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[0].posB[2] = bb->min.v[2] + bb->pos_offset.v[2];

        v[0].rgbaA = color;
        v[0].rgbaB = color;

        v[0].normA = 1;
        v[0].normB = 1;

        v[0].stA[0] = 0x0;
        v[0].stA[1] = 0x0;
        v[0].stB[0] = aabb_spr->width << 5;
        v[0].stB[1] = 0x0;

        /* Bottom B */
        v[1].posA[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[1].posA[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[1].posA[2] = bb->min.v[2] + bb->pos_offset.v[2];
        v[1].posB[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[1].posB[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[1].posB[2] = bb->min.v[2] + bb->pos_offset.v[2];

        v[1].rgbaA = color;
        v[1].rgbaB = color;

        v[1].normA = 1;
        v[1].normB = 1;

        v[1].stA[0] = aabb_spr->width << 5;
        v[1].stA[1] = aabb_spr->height << 5;
        v[1].stB[0] = 0x0;
        v[1].stB[1] = aabb_spr->height << 5;

        /* Top A */
        v[2].posA[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[2].posA[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[2].posA[2] = bb->max.v[2] + bb->pos_offset.v[2];
        v[2].posB[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[2].posB[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[2].posB[2] = bb->max.v[2] + bb->pos_offset.v[2];

        v[2].rgbaA = color;
        v[2].rgbaB = color;

        v[2].normA = 1;
        v[2].normB = 1;

        v[2].stA[0] = 0x0;
        v[2].stA[1] = 0x0;
        v[2].stB[0] = aabb_spr->width << 5;
        v[2].stB[1] = 0x0;

        /* Top B */
        v[3].posA[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[3].posA[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[3].posA[2] = bb->max.v[2] + bb->pos_offset.v[2];
        v[3].posB[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[3].posB[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[3].posB[2] = bb->max.v[2] + bb->pos_offset.v[2];

        v[3].rgbaA = color;
        v[3].rgbaB = color;

        v[3].normA = 1;
        v[3].normB = 1;

        v[3].stA[0] = aabb_spr->width << 5;
        v[3].stA[1] = aabb_spr->height << 5;
        v[3].stB[0] = 0x0;
        v[3].stB[1] = aabb_spr->height << 5;

        /* Front A */
        v[4].posA[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[4].posA[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[4].posA[2] = bb->min.v[2] + bb->pos_offset.v[2];
        v[4].posB[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[4].posB[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[4].posB[2] = bb->min.v[2] + bb->pos_offset.v[2];

        v[4].rgbaA = color;
        v[4].rgbaB = color;

        v[4].normA = 1;
        v[4].normB = 1;

        v[4].stA[0] = 0x0;
        v[4].stA[1] = 0x0;
        v[4].stB[0] = aabb_spr->width << 5;
        v[4].stB[1] = 0x0;

        /* Front B */
        v[5].posA[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[5].posA[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[5].posA[2] = bb->max.v[2] + bb->pos_offset.v[2];
        v[5].posB[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[5].posB[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[5].posB[2] = bb->max.v[2] + bb->pos_offset.v[2];

        v[5].rgbaA = color;
        v[5].rgbaB = color;

        v[5].normA = 1;
        v[5].normB = 1;

        v[5].stA[0] = aabb_spr->width << 5;
        v[5].stA[1] = aabb_spr->height << 5;
        v[5].stB[0] = 0x0;
        v[5].stB[1] = aabb_spr->height << 5;

        /* Back A */
        v[6].posA[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[6].posA[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[6].posA[2] = bb->min.v[2] + bb->pos_offset.v[2];
        v[6].posB[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[6].posB[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[6].posB[2] = bb->min.v[2] + bb->pos_offset.v[2];

        v[6].rgbaA = color;
        v[6].rgbaB = color;

        v[6].normA = 1;
        v[6].normB = 1;

        v[6].stA[0] = 0x0;
        v[6].stA[1] = 0x0;
        v[6].stB[0] = aabb_spr->width << 5;
        v[6].stB[1] = 0x0;

        /* Back B */
        v[7].posA[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[7].posA[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[7].posA[2] = bb->max.v[2] + bb->pos_offset.v[2];
        v[7].posB[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[7].posB[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[7].posB[2] = bb->max.v[2] + bb->pos_offset.v[2];

        v[7].rgbaA = color;
        v[7].rgbaB = color;

        v[7].normA = 1;
        v[7].normB = 1;

        v[7].stA[0] = aabb_spr->width << 5;
        v[7].stA[1] = aabb_spr->height << 5;
        v[7].stB[0] = 0x0;
        v[7].stB[1] = aabb_spr->height << 5;

        /* Left A */
        v[8].posA[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[8].posA[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[8].posA[2] = bb->min.v[2] + bb->pos_offset.v[2];
        v[8].posB[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[8].posB[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[8].posB[2] = bb->min.v[2] + bb->pos_offset.v[2];

        v[8].rgbaA = color;
        v[8].rgbaB = color;

        v[8].normA = 1;
        v[8].normB = 1;

        v[8].stA[0] = 0x0;
        v[8].stA[1] = 0x0;
        v[8].stB[0] = aabb_spr->width << 5;
        v[8].stB[1] = 0x0;

        /* Left B */
        v[9].posA[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[9].posA[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[9].posA[2] = bb->max.v[2] + bb->pos_offset.v[2];
        v[9].posB[0] = bb->max.v[0] + bb->pos_offset.v[0];
        v[9].posB[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[9].posB[2] = bb->max.v[2] + bb->pos_offset.v[2];

        v[9].rgbaA = color;
        v[9].rgbaB = color;

        v[9].normA = 1;
        v[9].normB = 1;

        v[9].stA[0] = aabb_spr->width << 5;
        v[9].stA[1] = aabb_spr->height << 5;
        v[9].stB[0] = 0x0;
        v[9].stB[1] = aabb_spr->height << 5;

        /* Right A */
        v[10].posA[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[10].posA[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[10].posA[2] = bb->min.v[2] + bb->pos_offset.v[2];
        v[10].posB[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[10].posB[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[10].posB[2] = bb->min.v[2] + bb->pos_offset.v[2];

        v[10].rgbaA = color;
        v[10].rgbaB = color;

        v[10].normA = 1;
        v[10].normB = 1;

        v[10].stA[0] = 0x0;
        v[10].stA[1] = 0x0;
        v[10].stB[0] = aabb_spr->width << 5;
        v[10].stB[1] = 0x0;

        /* Right B */
        v[11].posA[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[11].posA[1] = bb->max.v[1] + bb->pos_offset.v[1];
        v[11].posA[2] = bb->max.v[2] + bb->pos_offset.v[2];
        v[11].posB[0] = bb->min.v[0] + bb->pos_offset.v[0];
        v[11].posB[1] = bb->min.v[1] + bb->pos_offset.v[1];
        v[11].posB[2] = bb->max.v[2] + bb->pos_offset.v[2];

        v[11].rgbaA = color;
        v[11].rgbaB = color;

        v[11].normA = 1;
        v[11].normB = 1;

        v[11].stA[0] = aabb_spr->width << 5;
        v[11].stA[1] = aabb_spr->height << 5;
        v[11].stB[0] = 0x0;
        v[11].stB[1] = aabb_spr->height << 5;

        mtxfp = malloc_uncached(sizeof(*mtxfp));

        {
                T3DVec3 s, r, t;

                s = t3d_vec3_one();
                r = t3d_vec3_zero();
                t = t3d_vec3_zero();
                t3d_mat4fp_from_srt_euler(mtxfp, s.v, r.v, t.v);
        }

        rdpq_sync_pipe();
        rdpq_sprite_upload(TILE0, aabb_spr, NULL);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
        rdpq_mode_alphacompare(1);
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH |
                                T3D_FLAG_CULL_BACK | T3D_FLAG_TEXTURED);
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
