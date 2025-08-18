#include "game/room.h"

#include "t3d_ext.h"

enum {
        ROOM_00,
        ROOM_01,
        ROOM_CNT
};

static const char *room_mdl_paths[ROOM_CNT] = {
        "rom:/room00.t3dm",
        "rom:/room01.t3dm"
};

static const char *room_dat_paths[ROOM_CNT] = {
        "rom:/room00.room",
        "rom:/room01.room"
};

static T3DVec3 stored_door_positions[ROOM_CNT];

struct room room_init_from_index(const uint16_t ind)
{
        struct room r;

        r.mdl = t3d_model_load(room_mdl_paths[ind]);
        r.mtx = malloc_uncached(sizeof(*r.mtx));

        rspq_block_begin();
        t3d_matrix_push(r.mtx);
        t3d_model_draw(r.mdl);
        t3d_matrix_pop(1);
        r.dl = rspq_block_end();

        /* Load door position from file. */
        {
                float *pos_in;

                pos_in = (float *)asset_load(room_dat_paths[ind], NULL);
                stored_door_positions[ind] =
                        t3d_vec3_make(pos_in[0], pos_in[1], pos_in[2]);
                t3d_vec3_scale(stored_door_positions + ind,
                               stored_door_positions + ind, MODEL_SCALE);
        }

        r.obj_cnt = 0;
        r.objs = NULL;

        if (r.obj_cnt)
                r.objs = malloc(sizeof(*r.objs) * r.obj_cnt);

        return r;
}

uint16_t room_update(struct room *r, uint16_t r_ind,
                     const struct inputs *inp_old,
                     const struct inputs *inp_new, const float ft)
{
        if (INPUT_PRESS_PTR(inp_new, inp_old, BTN_A)) {
                if (++r_ind >= ROOM_CNT)
                        r_ind = 0;
        }

        return r_ind;
}

static T3DVec3 room_get_absolute_door_pos(const uint16_t ind)
{
        T3DVec3 total = { 0 };
        int i;

        for (i = 1; i <= ind; ++i)
                t3d_vec3_add(&total, &total, stored_door_positions + i - 1);

        debugf_t3d_vec3("total", &total);

        return total;
}

void room_setup_matrices(struct room *r, const uint16_t ind, const float st)
{
        int i;
        T3DVec3 scale, rot, pos;

        scale = t3d_vec3_one();
        rot = t3d_vec3_zero();
        pos = room_get_absolute_door_pos(ind);
        t3d_mat4fp_from_srt_euler(r->mtx, scale.v, rot.v, pos.v);

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
