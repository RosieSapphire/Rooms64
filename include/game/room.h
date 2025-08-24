#ifndef GAME_ROOM_H
#define GAME_ROOM_H

#include "inputs.h"

#include "engine/object.h"

#include "game/player.h"

enum {
        ROOM_TYPE_00,
        ROOM_TYPE_01,
        ROOM_TYPE_02,
        ROOM_TYPE_CNT
};

struct room {
        T3DModel *mdl;
        T3DMat4FP *mtx;
        rspq_block_t *dl;
        struct object *objs;
        T3DVec3 door_pos;
        uint16_t obj_cnt;
};

void rooms_generate(void);
void room_update(struct player *p);
void rooms_render(const float subtick);
void room_terminate(struct room *r);

#endif /* GAME_ROOM_H */
