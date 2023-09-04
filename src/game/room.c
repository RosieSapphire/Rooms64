#include <stdlib.h>
#include <GL/gl.h>

#include "engine/vector.h"
#include "engine/model.h"

#include "game/room.h"

room rooms[100];

static float room_door_offs[ROOM_TYPE_COUNT][3] = {
	{ 0.000f, 0.0f, 15.0f},
	{ 0.000f, 0.0f, 10.0f},
	{-2.500f, 0.0f, 10.0f},
	{-3.150f, 0.0f, 12.0f},
	{ 3.800f, 0.0f, 16.0f},
	{ 0.000f, 0.0f, 14.0f},
};

static const char *room_mdl_paths[ROOM_TYPE_COUNT] = {
	"rom:/room0.mdl",
	"rom:/room1.mdl",
	"rom:/room2.mdl",
	"rom:/room3.mdl",
	"rom:/room4.mdl",
	"rom:/room5.mdl",
};

void rooms_init(void)
{
	room room_refs[ROOM_TYPE_COUNT];
	for(int i = 0; i < ROOM_TYPE_COUNT; i++) {
		room_refs[i] = room_create(room_mdl_paths[i],
				room_door_offs[i], i);

	}

	rooms[0] = room_refs[0];
	for(int i = 1; i < 100; i++)
		rooms[i] = room_refs[1 + (rand() % (ROOM_TYPE_COUNT - 1))];
}

room room_create(const char *mdl_path, float rel_door_pos[3], int index)
{
	room r;
	const char *tex_paths[2] = {
		"rom:/wall.ci4.sprite",
		(index > 0) ? "rom:/carpet_blue.ci4.sprite" :
			"rom:/carpet_red.ci4.sprite"
	};

	model_create_file(&r.model, mdl_path, tex_paths);
	vector_copy(rel_door_pos, r.rel_door_pos);
	r.index = index;

	int room_indi_cnt = r.model.meshes->indi_cnt;
	r.collis.num_verts = room_indi_cnt;
	r.collis.verts = malloc(sizeof(vertex_t) * room_indi_cnt);
	for(int i = 0; i < room_indi_cnt; i++) {
		unsigned short *indis = r.model.meshes->indis;
		vertex_t *verts = r.model.meshes->verts;
		r.collis.verts[i] = verts[indis[i]];
	}

	return r;
}

void room_draw(room r, float global_offset[3])
{
	glPushMatrix();
	glTranslatef(global_offset[0], global_offset[1], global_offset[2]);
	model_draw(&r.model);
	glPopMatrix();
}

bool room_check_load_next(float player_pos[3], int current_room)
{
	float global_door_pos[3];
	room_get_global_pos(current_room, global_door_pos, true);
	return fabsf(vector_distance(player_pos, global_door_pos)) < 2;
}

void room_get_global_pos(int current_room, float out[3], bool next)
{
	vector_copy((float[3]){0, 0, 0}, out);
	for(int i = 0; i < current_room + next; i++)
		vector_add(out, rooms[i].rel_door_pos, out);
}
