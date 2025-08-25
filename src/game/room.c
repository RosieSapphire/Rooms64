#include "engine/aabb.h"

#include "game/sound.h"
#include "game/room.h"

#include "t3d_ext.h"

#define DOOR_OBJECT_COUNT_MAX (MAX_ROOMS_ACTIVE_AT_ONCE + 1)

struct door_object {
        struct object *obj_handle;
        float open_progress;
};

static struct door_object door_objects[DOOR_OBJECT_COUNT_MAX];
static uint16_t door_object_head = 0;

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
static struct room *room_cur = rooms;
static struct aabb next_door_hitbox;

static void door_update(struct object *o, __attribute__((unused))const float ft)
{
        struct door_object *d;

        d = door_objects + o->sub_obj_index;
        debugf("Door is updating. %f\n", d->open_progress);
}

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

        r.obj_cnt = 1;
        r.objs = malloc(sizeof(*r.objs) * r.obj_cnt);
        r.objs[0] = object_create("rom:/obj_door.t3dm", &r.door_pos,
                                  NULL, NULL, door_update);
        r.objs[0].sub_obj_index = door_object_head;
        door_objects[door_object_head].obj_handle = r.objs + 0;
        door_objects[door_object_head].open_progress = 69.f;
        ++door_object_head;

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
        for(i = 1; i < TOTAL_ROOM_COUNT; i++)
                rooms[i] = room_refs[1 + (rand() % (ROOM_TYPE_CNT - 1))];

        next_door_hitbox = door_hitbox_from_room(room_cur);
}

static void room_update(struct room *r, struct player *p,
                        const struct inputs *inp_new,
                        const struct inputs *inp_old, const float ft)
{
        int i;

        for (i = 0; i < r->obj_cnt; ++i) {
                struct object *self;

                self = r->objs + i;
                self->update_function(self, ft);
        }

        /* Only handle door opening if it's the latest room. */
        if (r != room_cur)
                return;

        if (!aabb_does_point_intersect(&next_door_hitbox, &p->position_b) &&
            (p->mode != PLAYER_MODE_NOCLIP ||
             !INPUT_PRESS_PTR(inp_new, inp_old, BTN_A)))
                return;

        sound_play(SFX_DOOR_OPEN, MIXER_CH_DOOR, .14f);

        if ((++room_cur - rooms) < TOTAL_ROOM_COUNT) {
                T3DVec3 from_door, b2a;

                t3d_vec3_diff(&from_door, &p->position_b, &(r - 1)->door_pos);
                t3d_vec3_diff(&b2a, &p->position_a, &p->position_b);
                p->position_b = from_door;
                t3d_vec3_add(&p->position_a, &b2a, &from_door);
                next_door_hitbox = door_hitbox_from_room(r);
                return;
        }

        assertf(0, "Game win\n");
}

void rooms_update(struct player *p, const struct inputs *inp_new,
                  const struct inputs *inp_old, const float ft)
{
        struct room *start, *r;

        start = room_cur;
        for (r = start; r > start - MAX_ROOMS_ACTIVE_AT_ONCE; --r) {
                /* If we don't have enough rooms to update, bail. */
                if (r - rooms < 0)
                        continue;

                room_update(r, p, inp_new, inp_old, ft);
        }

        aabb_render(&next_door_hitbox, 0x183048FF);
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
         * There's an almost always chance
         * the objects will be in from of
         * the room itself, so we render
         * them first so it fails the
         * depth buffer to reduce overdraw.
         */
        /*
        for (i = 0; i < r->obj_cnt; ++i) {
                T3DVec3 off;

                off = get_room_offset(room_cur, r);
                object_render(r->objs + i, &off, st);
        }
        */

        /*
        rspq_block_run(r->dl);
        */
}

void rooms_render(const float subtick)
{
        /*
        const struct room *start, *r;
        T3DVec3 off;

        start = room_cur;
        off = t3d_vec3_zero();
        for (r = start; r > (start - MAX_ROOMS_ACTIVE_AT_ONCE); --r) {
                if (r - rooms < 0)
                        continue;

                rspq_wait();
                off = get_room_offset(start, r);
                room_render(r, &off, subtick);
        }
        */

        /*aabb_render(&next_door_hitbox, 0x183048FF);*/
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
