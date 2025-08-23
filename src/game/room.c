#include "engine/aabb.h"

#include "game/room.h"

#include "t3d_ext.h"

#define ROOM_CNT 1000

static const char *room_mdl_paths[ROOM_CNT] = {
        "rom:/room00.t3dm",
        "rom:/room01.t3dm"
};

static const char *room_dat_paths[ROOM_CNT] = {
        "rom:/room00.room",
        "rom:/room01.room"
};

static struct room rooms[ROOM_CNT];
static struct room *room_prev = rooms;
static struct room *room_cur = rooms;
static struct aabb next_door_hitbox;

static T3DVec3 get_absolute_door_pos(const struct room *r,
                                     const bool count_last)
{
        T3DVec3 total;
        int max, i;

        total = t3d_vec3_zero();
        max = r - rooms;
        for (i = 0; i < max + count_last; ++i)
                t3d_vec3_add(&total, &total, &rooms[i].door_pos);

        return total;
}

static struct room room_load(const uint8_t type)
{
        struct room r;

        r.mdl = t3d_model_load(room_mdl_paths[type]);
        r.mtx = malloc_uncached(sizeof(*r.mtx));

        rspq_block_begin();
        t3d_matrix_push(r.mtx);
        t3d_model_draw(r.mdl);
        t3d_matrix_pop(1);
        r.dl = rspq_block_end();

        /* Load door position from file. */
        {
                float *pos_in;

                pos_in = (float *)asset_load(room_dat_paths[type], NULL);
                /* TODO: This gets overwritten a bunch. Maybe just cache it. */
                r.door_pos = t3d_vec3_make(pos_in[0], pos_in[1], pos_in[2]);
                t3d_vec3_scale(&r.door_pos, &r.door_pos, MODEL_SCALE);
        }

        r.obj_cnt = 0;

        if (!r.obj_cnt) {
                r.objs = NULL;
                return r;
        }

        /* TODO: Load objects from file. */
        r.objs = malloc(sizeof(*r.objs) * r.obj_cnt);

        return r;
}

void rooms_generate(void)
{
        struct room room_refs[ROOM_TYPE_CNT];
        int i;

        for(i = 0; i < ROOM_TYPE_CNT; ++i)
                room_refs[i] = room_load(i);

        /* First room is always the same. */
        rooms[0] = room_refs[0];
        for(int i = 1; i < ROOM_CNT; i++)
                rooms[i] = room_refs[1 + (rand() % (ROOM_TYPE_CNT - 1))];

        {
                T3DVec3 bb_min, bb_max, abs_door_pos;

                bb_min = t3d_vec3_make(-128.f, -16.f, 0.f);
                bb_max = t3d_vec3_make(128.f, 16.f, 256.f);
                abs_door_pos = get_absolute_door_pos(room_cur, true);
                next_door_hitbox = aabb_make(&abs_door_pos, &bb_min, &bb_max);
        }
}

void room_update(const T3DVec3 *player_pos)
{
        T3DVec3 player_pos_real;

        /* 
        debugf("%d -> %d\n",
               room_prev - rooms,
               room_cur - rooms);
         */
        t3d_vec3_scale(&player_pos_real, player_pos, MODEL_SCALE);

        room_prev = room_cur;
        if (aabb_does_point_intersect(&next_door_hitbox, &player_pos_real)) {
                T3DVec3 bb_min, bb_max, abs_door_pos;

                if ((++room_cur - rooms) >= ROOM_CNT) {
                        assertf(0, "Game win\n");
                        return;
                }

                bb_min = t3d_vec3_make(-128.f, -16.f, 0.f);
                bb_max = t3d_vec3_make(128.f, 16.f, 256.f);
                abs_door_pos = get_absolute_door_pos(room_cur, true);
                next_door_hitbox = aabb_make(&abs_door_pos, &bb_min, &bb_max);
        }
        /*

        if (room_prev == room_cur)
                return;

        room_terminate(room_prev);
        rspq_wait();
        room_load_next(ROOM_TYPE_01);
        */
}

void room_setup_matrices(const float st)
{
        int i;
        T3DVec3 scale, rot, pos;

        scale = t3d_vec3_one();
        rot = t3d_vec3_zero();
        pos = get_absolute_door_pos(room_cur, false);
        t3d_mat4fp_from_srt_euler(room_cur->mtx, scale.v, rot.v, pos.v);

        for (i = 0; i < room_cur->obj_cnt; ++i)
                object_setup_matrix(room_cur->objs + i, st);
}

static void room_render(const struct room *r)
{
        int i;

        rspq_block_run(r->dl);
        for (i = 0; i < r->obj_cnt; ++i)
                rspq_block_run(r->objs[i].dl);

        aabb_render(&next_door_hitbox, 0x183048FF);
}

void rooms_render(void)
{
        const struct room *start;
        T3DVec3 door_pos, room_pos_old, room_pos;
        int i;

        start = room_cur - 3;
        if ((start - rooms) < 0)
                start = rooms;

        door_pos = start->door_pos;
        room_pos_old = t3d_vec3_zero();
        room_pos = t3d_vec3_zero();
        for (i = 0; i <= (room_cur - rooms); ++i) {
                room_pos_old = room_pos;

                if (i < (start - rooms)) {
                        /* We still have to count up to the current room. */
                        t3d_vec3_add(&door_pos, &door_pos,
                                     &rooms[i + 1].door_pos);
                        t3d_vec3_add(&room_pos, &room_pos,
                                     &rooms[i].door_pos);
                        continue;
                }
        }

        room_render(room_cur);
        aabb_render(&next_door_hitbox, 0x183048FF);
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
