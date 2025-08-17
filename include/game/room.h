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

struct room room_init_from_index(const uint16_t ind);
uint16_t room_update(struct room *r, const struct inputs *inp_old,
                     const struct inputs *inp_new, const float ft);
void room_setup_matrices(struct room *r, const float st);
void room_render(const struct room *r);
void room_terminate(struct room *r);
