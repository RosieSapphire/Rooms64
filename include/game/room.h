#pragma once

#include "inputs.h"

#include "engine/object.h"

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
void room_update(const T3DVec3 *player_pos);
void rooms_render(const float subtick);
void room_terminate(struct room *r);
