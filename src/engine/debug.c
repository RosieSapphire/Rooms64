#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>

#include "engine/debug.h"

dvalue_t	   *vals;
int		    val_cnt = 0;
static rdpq_font_t *debug_font;

void debug_load(void)
{
	debug_font = rdpq_font_load("rom:/debug_font.font64");
	rdpq_font_style(debug_font,
			0,
			&(rdpq_fontstyle_t) { .color = RGBA32(0xFF,
							      0xFF,
							      0xFF,
							      0xFF) });
	rdpq_text_register_font(1, debug_font);
}

void debug_add(const char *name, void *val, enum val_type type)
{
	dvalue_t *v;

	++val_cnt;
	if (!vals)
		vals = malloc(val_cnt * sizeof(*vals));
	else
		vals = realloc(vals, val_cnt * sizeof(*vals));
	assertf(vals, "Failed to allocate values for debugging");

	v	= vals + (val_cnt - 1);
	v->name = name;
	v->val	= val;
	v->type = type;
}

void debug_clear(void)
{
	assert(vals);
	vals	= realloc(vals, 0);
	val_cnt = 0;
}

void debug_draw(void)
{
	for (int i = 0; i < val_cnt; i++) {
		dvalue_t v = vals[i];

		char buf[128];
		switch (v.type) {
		case DV_INT:
			sprintf(buf, "%s: %d\n", v.name, *(int *)v.val);
			break;

		case DV_FLOAT:
			sprintf(buf, "%s: %f\n", v.name, *(float *)v.val);
			break;

		case DV_BOOL:
			sprintf(buf,
				"%s: %s\n",
				v.name,
				*(bool *)v.val > 0 ? "TRUE" : "FALSE");
			break;

		case DV_VECTOR2:
			{
				float *vec = (float *)v.val;
				sprintf(buf,
					"%s: (%.2f, %.2f)\n",
					v.name,
					vec[0],
					vec[1]);
				break;
			}

		case DV_VECTOR3:
			{
				float *vec = (float *)v.val;
				sprintf(buf,
					"%s: (%.2f, %.2f, %.2f)\n",
					v.name,
					vec[0],
					vec[1],
					vec[2]);
			}
		}

		rdpq_text_print(NULL, 1, 16, 16 + 12 * i, buf);
	}
}
