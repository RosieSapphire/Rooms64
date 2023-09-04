#include "engine/texture.h"
#include "engine/model.h"

#include "game/door_numplate.h"

static const char *plate_mdl_path = "rom:/door_numplate.mdl";
static const char *plate_tex_path = "rom:/wood_yel.ci4.sprite";
static model_t plate_model;

static const char *nums_mdl_path = "rom:/door_nums.mdl";
static model_t nums_model;

static rdpq_font_t *num_font;
static texture_t num_tex;

void door_numplate_init(void)
{
	model_create_file(&plate_model, plate_mdl_path, &plate_tex_path);
	model_create_file(&nums_model, nums_mdl_path, NULL);
	num_font = rdpq_font_load("rom:/door_num_font.font64");
	rdpq_font_style(num_font, 0, &(rdpq_fontstyle_t){.color =
			RGBA32(0xFF, 0xFF, 0xFF, 0xFF)});
	rdpq_text_register_font(2, num_font);
	num_tex = texture_create_empty(FMT_I8, 128, 32);
}

void door_numplate_draw(int current_room, int i) {
	bool cur_room, prev_room;
	cur_room = i == current_room;
	prev_room = i == current_room - 1;
	if(!cur_room && !prev_room)
		return;

	glTranslatef(0, 2.4f, 0);
	model_draw(&plate_model);

	int hunds, tens, ones;
	int room_val = cur_room ? current_room : current_room - 1;
	hunds = room_val / 100;
	tens  = (room_val % 100) / 10;
	ones  = room_val % 10;

	/* Render number to buffer */
	rdpq_attach_clear(&num_tex.surf, NULL);
	rdpq_text_printf(NULL, 2, 16, 28, "A-%d%d%d\n", hunds, tens, ones);
	rdpq_detach();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_EQUAL);
	model_draw_mesh_index_custom_tex(&nums_model, 0, num_tex, 0, 0, 0);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	glTranslatef(0, -2.4f, 0);
}
