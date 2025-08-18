#include "game/room.h"

#include "t3d_ext.h"

enum {
        ROOM_00,
        ROOM_01,
        ROOM_CNT
};

static const char *room_paths[ROOM_CNT] = {
        "rom:/room00.t3dm",
        "rom:/room01.t3dm"
};

struct room room_init_from_index(const uint16_t ind)
{
        struct room r;

        r.mdl = t3d_model_load(room_paths[ind]);
        r.mtx = malloc_uncached(sizeof(*r.mtx));

        rspq_block_begin();
        t3d_matrix_push(r.mtx);
        t3d_model_draw(r.mdl);
        t3d_matrix_pop(1);
        r.dl = rspq_block_end();

        r.obj_cnt = 0;
        r.objs = NULL;

        if (r.obj_cnt)
                r.objs = malloc(sizeof(*r.objs) * r.obj_cnt);

        return r;
}

uint16_t room_update(struct room *r, const uint16_t r_ind,
                     const struct inputs *inp_old,
                     const struct inputs *inp_new, const float ft)
{
        return r_ind ^ INPUT_PRESS_PTR(inp_new, inp_old, BTN_A);
}

void room_setup_matrices(struct room *r, const float st)
{
        int i;

        {
                T3DVec3 scale, rot, pos;

                scale = t3d_vec3_one();
                rot = t3d_vec3_zero();
                pos = t3d_vec3_zero();
                t3d_mat4fp_from_srt_euler(r->mtx, scale.v, rot.v, pos.v);
        }

        for (i = 0; i < r->obj_cnt; ++i)
                object_setup_matrix(r->objs + i, st);
}

void room_render(const struct room *r)
{
        int i;

        rspq_block_run(r->dl);
        for (i = 0; i < r->obj_cnt; ++i)
                rspq_block_run(r->objs[i].dl);
}

void room_terminate(struct room *r)
{
        if (r->obj_cnt) {
                free(r->objs);
                r->objs = NULL;
                r->obj_cnt = 0;
        }

        rspq_block_free(r->dl);
        r->dl = NULL;

        free_uncached(r->mtx);
        r->mtx = NULL;

        t3d_model_free(r->mdl);
        r->mdl = NULL;
}
