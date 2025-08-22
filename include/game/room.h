#pragma once

#include "inputs.h"

#include "engine/object.h"

struct room {
        T3DModel *mdl;
        T3DMat4FP *mtx;
        rspq_block_t *dl;
        uint16_t obj_cnt;
        struct object *objs;
};

void room_init_from_index(const uint16_t ind);
void room_update(const struct inputs *inp_old,
                 const struct inputs *inp_new, const float ft);
void room_setup_matrices(const float st);
void room_render(void);
void room_terminate(void);
