#pragma once

#include "inputs.h"

#include "engine/object.h"

enum {
        ROOM_TYPE_00,
        ROOM_TYPE_01,
        ROOM_TYPE_CNT
};

struct room {
        T3DModel *mdl;
        T3DMat4FP *mtx;
        rspq_block_t *dl;
        struct object *objs;
        uint16_t obj_cnt;
        uint8_t type;
};

void room_load_next(const uint8_t type);
void room_update(const T3DVec3 *player_pos);
void room_setup_matrices(const float st);
void room_render(void);
void room_terminate(void);
