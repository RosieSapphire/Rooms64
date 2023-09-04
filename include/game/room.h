#ifndef GAME_ROOM_H
#define GAME_ROOM_H

#include <libdragon.h>

#include "engine/model.h"
#include "engine/collision.h"

#define ROOM_TYPE_COUNT 6

typedef struct {
	model_t model;
	float rel_door_pos[3];
	rspq_block_t *block;
	int index;
	collision_mesh_t collis;
} room;

extern room rooms[100];

void rooms_init(void);
room room_create(const char *mdl_path, float rel_door_pos[3], int index);
void room_draw(room r, float global_offset[3]);
bool room_check_load_next(float player_pos[3], int current_room);
void room_get_global_pos(int current_room, float out[3], bool next);

#endif /* GAME_ROOM_H */
