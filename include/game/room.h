#pragma once

#include "engine/object.h"

struct room {
        T3DModel *mdl;
        T3DMat4FP *mtx;
        rspq_block_t *dl;
        uint16_t obj_cnt;
        struct object *objs;
};

struct room room_init_from_path(const char *path);
void room_update(struct room *r, const float ft);
void room_setup_matrices(struct room *r, const float st);
void room_render(const struct room *r);
void room_terminate(struct room *r);
