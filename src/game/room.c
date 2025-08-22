#include "engine/aabb.h"

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

static uint16_t room_ind_prev = 0;
static uint16_t room_ind;
static struct room room_cur;
static struct aabb room_cur_door_bb;

void room_init_from_index(const uint16_t ind)
{
        room_ind = ind;
        room_cur.mdl = t3d_model_load(room_mdl_paths[ind]);
        room_cur.mtx = malloc_uncached(sizeof(*room_cur.mtx));

        rspq_block_begin();
        t3d_matrix_push(room_cur.mtx);
        t3d_model_draw(room_cur.mdl);
        t3d_matrix_pop(1);
        room_cur.dl = rspq_block_end();

        /* Load door position from file. */
        {
                float *pos_in;

                pos_in = (float *)asset_load(room_dat_paths[ind], NULL);
                stored_door_positions[ind] =
                        t3d_vec3_make(pos_in[0], pos_in[1], pos_in[2]);
                t3d_vec3_scale(stored_door_positions + ind,
                               stored_door_positions + ind, MODEL_SCALE);
        }

        /* Make bounding box for door. */
        {
                T3DVec3 bb_min, bb_max;

                bb_min = t3d_vec3_make(-32.f, 0.f, -32.f);
                bb_max = t3d_vec3_make(32.f, 64.f, 32.f);
                room_cur_door_bb = aabb_make(stored_door_positions + ind,
                                             &bb_min, &bb_max);
        }

        room_cur.obj_cnt = 0;
        room_cur.objs = NULL;

        if (!room_cur.obj_cnt)
                return;

        /* TODO: Load objects from file. */
        room_cur.objs = malloc(sizeof(*room_cur.objs) * room_cur.obj_cnt);
}

void room_update(const T3DVec3 *player_pos, const float ft)
{
        T3DVec3 player_pos_real;

        room_ind_prev = room_ind;

        t3d_vec3_scale(&player_pos_real, player_pos, MODEL_SCALE);
        if (aabb_does_point_intersect(&room_cur_door_bb, &player_pos_real)) {
                if (++room_ind >= ROOM_CNT)
                        room_ind = 0;
        }

        if (!(room_ind_prev ^ room_ind))
                return;

        debugf("ROOM INDEX CHANGED!\n");
        room_terminate();
        rspq_wait();
        room_init_from_index(room_ind);
}

static T3DVec3 room_get_absolute_door_pos(const uint16_t ind)
{
        T3DVec3 total = { 0 };
        int i;

        for (i = 1; i <= ind; ++i)
                t3d_vec3_add(&total, &total, stored_door_positions + i - 1);

        return total;
}

void room_setup_matrices(const float st)
{
        int i;
        T3DVec3 scale, rot, pos;

        scale = t3d_vec3_one();
        rot = t3d_vec3_zero();
        pos = room_get_absolute_door_pos(room_ind);
        t3d_mat4fp_from_srt_euler(room_cur.mtx, scale.v, rot.v, pos.v);

        for (i = 0; i < room_cur.obj_cnt; ++i)
                object_setup_matrix(room_cur.objs + i, st);
}

void room_render(void)
{
        int i;

        rspq_block_run(room_cur.dl);
        for (i = 0; i < room_cur.obj_cnt; ++i)
                rspq_block_run(room_cur.objs[i].dl);

        aabb_render(&room_cur_door_bb);
}

void room_terminate(void)
{
        if (room_cur.obj_cnt) {
                free(room_cur.objs);
                room_cur.objs = NULL;
                room_cur.obj_cnt = 0;
        }

        rspq_block_free(room_cur.dl);
        room_cur.dl = NULL;

        free_uncached(room_cur.mtx);
        room_cur.mtx = NULL;

        t3d_model_free(room_cur.mdl);
        room_cur.mdl = NULL;
}
