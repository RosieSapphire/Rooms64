#include <malloc.h>

#include "engine/model.h"
#include "engine/util.h"
#include "engine/vector.h"

#include "game/player.h"
#include "game/room.h"
#include "game/locker.h"

static model_t model;
static model_t model_inside;
uint16_t lockers_cnt[ROOM_TYPE_COUNT];
locker *lockers[ROOM_TYPE_COUNT];
static rspq_block_t *locker_outside_blocks[ROOM_TYPE_COUNT];
static rspq_block_t *locker_inside_blocks[ROOM_TYPE_COUNT];

const char *room_locker_paths[ROOM_TYPE_COUNT] = {
	"rom:/room0.dat",
	"rom:/room1.dat",
	"rom:/room2.dat",
	"rom:/room3.dat",
	"rom:/room4.dat",
	"rom:/room5.dat",
};

static void _locker_init(int i)
{
	FILE *file = fopen(room_locker_paths[i], "rb");
	if(!file) {
		debugf("Cannot find locker file for room %d\n", i);
		return;
	}
	fread(lockers_cnt + i, sizeof(uint16_t), 1, file);
	lockers_cnt[i] = uint16_flip_endian(lockers_cnt[i]);
	lockers[i] = malloc(sizeof(locker) * lockers_cnt[i]);
	for(int j = 0; j < lockers_cnt[i]; j++) {
		for(int k = 0; k < 3; k++) {
			uint32_t tmp = 0;
			fread(&tmp, sizeof(float), 1, file);
			tmp = uint32_flip_endian(tmp);
			memcpy(lockers[i][j].pos + k, &tmp, 4);
		}
	}
	fclose(file);
}

void lockers_init(void)
{
	const char *tex_paths[2] = {
		"rom:/metal.ci4.sprite",
		"rom:/metal_face.ci4.sprite",
	};
	model_create_file(&model, "rom:/locker.mdl", tex_paths);
	model_create_file(&model_inside, "rom:/locker_inside.mdl", tex_paths);

	for(int i = 0; i < ROOM_TYPE_COUNT; i++) {
		_locker_init(i);

		rspq_block_begin();
		for(int j = 0; j < lockers_cnt[i]; j++) {
			locker l = lockers[i][j];
			glPushMatrix();

			glTranslatef(l.pos[0], 0, l.pos[2]);
			glRotatef(90.0f * (int)l.pos[1], 0, 1, 0);
			model_draw(&model);
			glPopMatrix();
		}
		locker_outside_blocks[i] = rspq_block_end();

		rspq_block_begin();
		for(int j = 0; j < lockers_cnt[i]; j++) {
			locker l = lockers[i][j];
			glPushMatrix();

			glTranslatef(l.pos[0], 0, l.pos[2]);
			glRotatef(90.0f * (int)l.pos[1], 0, 1, 0);
			model_draw(&model_inside);
			glPopMatrix();
		}
		locker_inside_blocks[i] = rspq_block_end();
	}
}

void lockers_draw(int current_room, float global_offset[3], bool is_inside)
{
	glPushMatrix();
	glTranslatef(global_offset[0], global_offset[1], global_offset[2]);
	int ind = rooms[current_room].index;
	rspq_block_run(locker_outside_blocks[ind]);
	if(is_inside)
		rspq_block_run(locker_inside_blocks[ind]);
	glPopMatrix();
}
