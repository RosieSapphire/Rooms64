#include <stdio.h>
#include <malloc.h>

#include "engine/model.h"
#include "engine/util.h"

void model_create_file(model_t *m, const char *mdl_path,
		const char **tex_paths)
{
	FILE *mdl = fopen(mdl_path, "rb");
	assertf(mdl, "Failed to load model from path %s\n", mdl_path);
	fread(&m->mesh_cnt, sizeof(uint16_t), 1, mdl);
	m->mesh_cnt = uint16_flip_endian(m->mesh_cnt);

	m->blocks = malloc(sizeof(rspq_block_t *) * m->mesh_cnt);
	m->meshes = malloc(sizeof(gl_mesh_t) * m->mesh_cnt);
	for(int i = 0; i < m->mesh_cnt; i++) {
		gl_mesh_create_file(m->meshes + i, tex_paths != NULL ?
					tex_paths[i] : NULL, mdl);
		m->blocks[i] = gl_mesh_gen_instance_block(m->meshes + i);
	}
	fclose(mdl);
}

void model_draw_mesh_index(const model_t *m, int i)
{
	gl_mesh_draw_instance(m->blocks[i]);
}

void model_draw_mesh_index_custom_tex(const model_t *m, int i, texture_t tex,
		float r, float g, float b)
{
	gl_mesh_draw_custom_tex(m->meshes + i, tex, r, g, b);
}

void model_draw(const model_t *m)
{
	for(int i = 0; i < m->mesh_cnt; i++)
		gl_mesh_draw_instance(m->blocks[i]);
}
