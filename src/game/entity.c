#include "engine/model.h"
#include "engine/config.h"
#include "engine/vector.h"
#include "engine/sfx.h"
#include "engine/util.h"

#include "game/room.h"
#include "game/player.h"
#include "game/entity.h"

#define PROGRESS_RATE 0.07f

static model_t model;

void entities_init(void)
{
	model_create_file(&model, "rom:/door.mdl", NULL);
}

void entity_update(entity_t *e, int current_room)
{
	e->was_active = e->is_active;
	if(!e->is_active)
		return;

	e->progress_last = e->progress;
	e->progress += PROGRESS_RATE;
	if(e->progress >= 1.0f) {
		if(e->cur_point == (current_room + 4)) {
			e->is_active = false;
			e->cur_point = 0;
			e->progress = e->progress_last = 0;
			e->num_rooms_crossed = 0;
			mixer_ch_stop(SFXC_ENTITY_LOW);
			mixer_ch_stop(SFXC_ENTITY_HIGH);
			return;
		}

		e->cur_point++;
		e->num_rooms_crossed++;
		e->progress = 0;
	}

	bool can_kill = current_room == e->cur_point && e->progress > 0.5f;
	bool player_in_locker = pstate == PS_IN_LOCKER;
	if(can_kill && !player_in_locker)
		assertf(0, "You died\n");
}

void entity_get_pos(const entity_t *e, float vec[3], float subtick)
{
	if(e->cur_point == 0)
		vector_sub(rooms[0].rel_door_pos, (float[3]){0}, vec);
	else {
		float a[3], b[3];
		room_get_global_pos(e->cur_point, a, false);
		room_get_global_pos(e->cur_point, b, true);
		vector_sub(b, a, vec);
	}

	float prog_lerp = lerpf(e->progress_last, e->progress, subtick);
	if(fabsf(e->progress_last - e->progress) > 0.5f)
		prog_lerp = e->progress;

	vector_scale(vec, prog_lerp);

	float glob[3];
	room_get_global_pos(e->cur_point, glob, false);
	vector_add(vec, glob, vec);
}

void entity_draw(const entity_t *e, float subtick, float player_dir[3])
{
	if(!e->is_active)
		return;

	glPushMatrix();
	float ent_pos[3];
	entity_get_pos(e, ent_pos, subtick);
	glTranslatef(ent_pos[0], ent_pos[1], ent_pos[2]);

	float phi = atan2f(player_dir[0], player_dir[2]) * TO_DEGREES;
	glRotatef(phi, 0, 1, 0);

	model_draw(&model);

	glPopMatrix();
}

void entity_trigger(entity_t *e, int current_room)
{
	e->is_active = true;
	e->num_rooms_crossed = 0;
	e->cur_point = current_room - 8;

	wav64_play(&a60_low, SFXC_ENTITY_LOW);
	wav64_play(&a60_high, SFXC_ENTITY_HIGH);
}
