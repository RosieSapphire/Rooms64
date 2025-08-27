#include "engine/aabb.h"

#include "game/sound.h"
#include "game/room.h"

#include "t3d_ext.h"

#define DOOR_TURN_SPEED 180.f
#define DOOR_X_OFFSET 0.625f

#define DOOR_FROM_OBJ_PTR(OBJ_PTR) (door_objects + (OBJ_PTR)->sub_obj_index)
#define AS_ROOM_PTR(VOID_PTR) ((struct room *)(VOID_PTR))

struct door_object {
        struct object *header;
        float start_angle;
        float open_progress;
};

/*
 * TODO: Make this the size of how many doors that can be
 * visible at one time. That way instead of taking up a
 * bunch of space with the `door_objects` array, it only
 * has how many are gonna be used at a given time, and
 * when you enter another room, the whole array gets
 * shifted down (which works because you can't go back
 * through a door that's already been closed.
 */
static struct door_object door_objects[TOTAL_ROOM_COUNT];
static uint32_t door_object_head;

static const char *room_mdl_paths[TOTAL_ROOM_COUNT] = {
        "rom:/room00.t3dm",
        "rom:/room01.t3dm",
        "rom:/room02.t3dm",
        "rom:/room03.t3dm",
        "rom:/room04.t3dm",
        "rom:/room05.t3dm"
};

static const char *room_dat_paths[TOTAL_ROOM_COUNT] = {
        "rom:/room00.room",
        "rom:/room01.room",
        "rom:/room02.room",
        "rom:/room03.room",
        "rom:/room04.room",
        "rom:/room05.room"
};

static struct room rooms[TOTAL_ROOM_COUNT];
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

static T3DVec3 get_absolute_door_pos(struct door_object *d)
{
        return t3d_vec3_zero();
}

static void door_update(struct object *o, const float ft)
{
        struct door_object *d;
        struct room *r;
        T3DVec3 rot, pos;
        float min;

        d = DOOR_FROM_OBJ_PTR(o);
        r = AS_ROOM_PTR(o->parent);
        min = d->start_angle - 90.f;
        if (r < room_cur)
                d->open_progress -= ft * DOOR_TURN_SPEED;

        if (d->open_progress <= min)
                d->open_progress = min;

        rot = t3d_vec3_zero();
        rot.v[2] = T3D_DEG_TO_RAD(d->start_angle + d->open_progress);

        pos = get_room_offset(room_cur, r);
        t3d_vec3_add(&pos, &pos, &r->door_pos);
        {
                T3DVec3 offset;

                offset = t3d_vec3_make(DOOR_X_OFFSET, 0.f, 0.f);
                t3d_vec3_add(&pos, &pos, &offset);
        }
        object_set_transforms(o, NULL, &rot, &pos, INTERP_ROT);
}

static struct aabb door_hitbox_from_room(const struct room *r)
{
        struct aabb bb;

        bb.pos_offset = r->door_pos;
        bb.min = t3d_vec3_make(-1.84f, 0.f, 0.f);
        bb.max = t3d_vec3_make(1.84f, 2.55f, 2.65f);

        return bb;
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

        return r;
}

static void room_init_door(struct room *r, const float start_angle)
{
        struct object *o;
        struct door_object *d;
        T3DVec3 pos, off;

        /* Allocate object slot */
        r->obj_cnt = 1;
        r->objs = malloc(sizeof(*r->objs) * r->obj_cnt);

        /* Create object in slot */
        o = r->objs + 0;
        pos = get_room_offset(rooms, r);
        t3d_vec3_add(&pos, &pos, &r->door_pos);
        off = t3d_vec3_make(DOOR_X_OFFSET, 0.f, 0.f);
        t3d_vec3_add(&pos, &pos, &off);
        *o = object_create("rom:/obj_door.t3dm", NULL, NULL,
                           &pos, door_update, r);

        /*
         * FIXME: This works fine now because
         * the number of door slots is equal
         * to the number of rooms, but once
         * this gets optimized to only the
         * amount of rooms visible at a time,
         * we will need a different way of
         * handling this. Possibly a head
         * integer that wraps around, something
         * like that.
         */
        o->sub_obj_index = r - rooms;

        /* Setup subobject (door) variables */
        d = DOOR_FROM_OBJ_PTR(o);
        d->header = o;
        d->start_angle = start_angle;
        d->open_progress = start_angle;

}

void rooms_generate(void)
{
        struct room room_refs[ROOM_TYPE_CNT];
        int i;

        for(i = 0; i < ROOM_TYPE_CNT; ++i)
                room_refs[i] = room_load(i);

        /* First room is always the same. */
        rooms[0] = room_refs[0];
        room_init_door(rooms + 0, 0.f);
        for(i = 1; i < TOTAL_ROOM_COUNT; i++) {
                struct room *r;

                assertf(i <= TOTAL_ROOM_COUNT,
                        "Too many door objects have been loaded (%d).", i);

                r = rooms + i;
                *r = room_refs[1 + (rand() % (ROOM_TYPE_CNT - 1))];
                room_init_door(r, 0.f);
        }

        next_door_hitbox = door_hitbox_from_room(room_cur);
}

static void room_update_current(struct player *p, const struct inputs *inp_new,
                                const struct inputs *inp_old, const float ft)
{
        if (!aabb_does_point_intersect(&next_door_hitbox, &p->position_b) &&
            (p->mode != PLAYER_MODE_NOCLIP ||
             !INPUT_PRESS_PTR(inp_new, inp_old, BTN_A)))
                return;

        sound_play(SFX_DOOR_OPEN, MIXER_CH_DOOR, .14f);

        int prev = (room_cur - rooms);
        if ((++room_cur - rooms) < TOTAL_ROOM_COUNT) {
                T3DVec3 from_door, b2a;
                int cur;

                t3d_vec3_diff(&from_door, &p->position_b,
                              &(room_cur - 1)->door_pos);
                t3d_vec3_diff(&b2a, &p->position_a, &p->position_b);
                p->position_b = from_door;
                t3d_vec3_add(&p->position_a, &b2a, &from_door);
                next_door_hitbox = door_hitbox_from_room(room_cur);
                cur = (room_cur - rooms);
                return;
        }

        assertf(0, "Game win\n");
}

static void room_update_objects(struct room *r, const float ft)
{
        int i;

        for (i = 0; i < r->obj_cnt; ++i) {
                struct object *obj;

                obj = r->objs + i;
                obj->update_function(obj, ft);
        }
}

void rooms_update(struct player *p, const struct inputs *inp_new,
                  const struct inputs *inp_old, const float ft)
{
        struct room *start, *r;
        struct room *room_prev;

        start = room_cur;
        room_prev = room_cur;
        room_update_current(p, inp_new, inp_old, ft);
        for (r = start; r > start - MAX_ROOMS_ACTIVE_AT_ONCE; --r) {
                if (r - rooms < 0)
                        continue;

                room_update_objects(r, ft);
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
         * There's an almost always chance
         * the objects will be in from of
         * the room itself, so we render
         * them first so it fails the
         * depth buffer to reduce overdraw.
         */
        for (i = 0; i < r->obj_cnt; ++i) {
                struct object *o;
                struct door_object *d;
                T3DVec3 or, op, door_off;

                o = r->objs + i;
                d = DOOR_FROM_OBJ_PTR(o);
                object_render(o, st);
        }

        rspq_block_run(r->dl);
}

void rooms_render(const float subtick)
{
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
