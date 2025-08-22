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

        room_cur.obj_cnt = 0;
        room_cur.objs = NULL;

        if (!room_cur.obj_cnt)
                return;

        /* TODO: Load objects from file. */
        room_cur.objs = malloc(sizeof(*room_cur.objs) * room_cur.obj_cnt);
}

static bool is_touching_hitbox_for_next_room(const T3DVec3 *player_pos)
{
        T3DVec3 hitbox_pos, player_pos_real, dist_vec;
        float distf;

        hitbox_pos = stored_door_positions[room_ind];
        t3d_vec3_scale(&player_pos_real, player_pos, MODEL_SCALE);
        t3d_vec3_diff(&dist_vec, &hitbox_pos, &player_pos_real);
        distf = t3d_vec3_len(&dist_vec);

        debugf("distf=%f\n", distf);

        return (distf < 128.f);
}

void room_update(const T3DVec3 *player_pos, const float ft)
{
        room_ind_prev = room_ind;

        if (is_touching_hitbox_for_next_room(player_pos)) {
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
