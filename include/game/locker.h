#ifndef GAME_LOCKER_H
#define GAME_LOCKER_H

#include "room.h"

typedef struct {
	float pos[3];
	int rot;
} locker;

extern uint16_t lockers_cnt[ROOM_TYPE_COUNT];
extern locker *lockers[ROOM_TYPE_COUNT];

void lockers_init(void);
void lockers_draw(int current_room, float global_offset[3], bool is_inside);

#endif /* GAME_LOCKER_H */
