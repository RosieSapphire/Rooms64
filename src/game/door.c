#include "engine/model.h"
#include "engine/util.h"

#include "game/door.h"

static float open_timers_last[100] = {0};
static float open_timers[100] = {0};
static const char *mdl_path = "rom:/door.mdl";
static const char *tex_path = "rom:/wood.ci4.sprite";
static model_t model;

void door_init(void)
{
	model_create_file(&model, mdl_path, &tex_path);
}

void doors_update_open_anim(int current_room)
{
	int j = current_room - 1;
	if(j < 0)
		return;

	open_timers_last[j] = open_timers[j];
	open_timers[j] = clampf(open_timers[j] + 0.016f, 0, 0.25f);
}

void door_draw(int i, float subtick)
{
	float open_timer = lerpf(open_timers_last[i],
			open_timers[i], subtick);
	glRotatef(open_timer * 360, 0, 1, 0);
	model_draw(&model);
}
