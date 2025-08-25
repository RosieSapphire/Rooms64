#include "engine/aabb.h"

#include "game/room.h"

#include "t3d_ext.h"

static const char *room_mdl_paths[TOTAL_ROOM_COUNT] = {
        "rom:/room00.t3dm",
        "rom:/room01.t3dm",
        "rom:/room02.t3dm"
};

static const char *room_dat_paths[TOTAL_ROOM_COUNT] = {
        "rom:/room00.room",
        "rom:/room01.room",
        "rom:/room02.room"
};

static struct room rooms[TOTAL_ROOM_COUNT];
static struct room *room_prev = rooms;
static struct room *room_cur = rooms;
static struct aabb next_door_hitbox;

static T3DVec3 get_room_offset(const struct room *head, const struct room *tail)
{
        T3DVec3 off;
        const struct room *r;

        off = t3d_vec3_zero();
        for (r = head - 1; r >= tail; --r)
                t3d_vec3_diff(&off, &off, &r->door_pos);

        return off;
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
                free(pos_in);
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

static struct aabb door_hitbox_from_room(const struct room *r)
{
        struct aabb bb;

        bb.pos_offset = r->door_pos;
        bb.min = t3d_vec3_make(-1.84f, 0.f, 0.f);
        bb.max = t3d_vec3_make(1.84f, 2.55f, 2.65f);

        return bb;
}

void rooms_generate(void)
{
        struct room room_refs[ROOM_TYPE_CNT];
        int i;

        for(i = 0; i < ROOM_TYPE_CNT; ++i)
                room_refs[i] = room_load(i);

        /* First room is always the same. */
        rooms[0] = room_refs[0];
        for(int i = 1; i < TOTAL_ROOM_COUNT; i++)
                rooms[i] = room_refs[1 + (rand() % (ROOM_TYPE_CNT - 1))];

        next_door_hitbox = door_hitbox_from_room(room_cur);
}

void room_update(struct player *p, const struct inputs *inp_new,
                 const struct inputs *inp_old)
{
        room_prev = room_cur;

        {
                uint32_t cur, max;
                float progress;

                cur = (room_cur - rooms);
                max = TOTAL_ROOM_COUNT;
                progress = ((float)cur / (float)TOTAL_ROOM_COUNT) * 100.f;
                debugf("Progress: %.2f%% (%lu/%lu)\n", progress, cur, max);
        }

        if (!aabb_does_point_intersect(&next_door_hitbox, &p->position_b) &&
            (p->mode != PLAYER_MODE_NOCLIP ||
             !INPUT_PRESS_PTR(inp_new, inp_old, BTN_A)))
                return;

        if ((++room_cur - rooms) >= TOTAL_ROOM_COUNT) {
                assertf(0, "Game win\n");
                return;
        }

        {
                T3DVec3 from_door, b2a;

                t3d_vec3_diff(&from_door, &p->position_b,
                              &(room_cur - 1)->door_pos);
                t3d_vec3_diff(&b2a, &p->position_a, &p->position_b);
                p->position_b = from_door;
                t3d_vec3_add(&p->position_a, &b2a, &from_door);
                next_door_hitbox = door_hitbox_from_room(room_cur);
        }
}

static void room_render(const struct room *r, const T3DVec3 *offset,
                        const float st)
{
        int i;
        T3DVec3 scale, rot, pos;

        scale = t3d_vec3_one();
        rot = t3d_vec3_zero();
        t3d_vec3_scale(&pos, offset, MODEL_SCALE);
        t3d_mat4fp_from_srt_euler(r->mtx, scale.v, rot.v, pos.v);

        /*
         * There's an almost always the objects
         * will be in from of the room itself,
         * so we render them first so it fails
         * the depth buffer to reduce overdraw.
         */
        for (i = 0; i < r->obj_cnt; ++i)
                object_render(r->objs + i, st);

        rspq_block_run(r->dl);
}

void rooms_render(const float subtick)
{
        const struct room *start, *r;
        T3DVec3 off;
        int i;

        start = room_cur;
        off = t3d_vec3_zero();
        room_render(start, &off, subtick);

        for (r = start - 1; r > start - MAX_ROOMS_ACTIVE_AT_ONCE; --r) {
                /* If we don't have enough rooms to render, bail. */
                if (r - rooms < 0)
                        continue;

                rspq_wait();
                off = get_room_offset(start, r);
                room_render(r, &off, subtick);
        }

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
