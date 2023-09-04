#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <libdragon.h>

#include "engine/vector.h"
#include "engine/util.h"
#include "engine/gl_mesh.h"

void gl_mesh_create_data(gl_mesh_t *m, unsigned int vert_cnt,
		const vertex_t *verts, unsigned int indi_cnt,
		const unsigned int *indis, const char *tex_path)
{
	m->vert_cnt = vert_cnt;
	m->indi_cnt = indi_cnt;
	unsigned int verts_size = sizeof(*verts) * vert_cnt;
	unsigned int indis_size = sizeof(*indis) * indi_cnt;
	m->verts = malloc(verts_size);
	m->indis = malloc(indis_size);
	memcpy(m->verts, verts, verts_size);
	memcpy(m->indis, indis, indis_size);

	m->tex.id = 0;
	if(tex_path)
		m->tex = texture_create_file(tex_path);
}

void gl_mesh_create_file(gl_mesh_t *m, const char *tex_path, FILE *mdl)
{
	uint16_t vert_cnt, indi_cnt;

	fread(&vert_cnt, sizeof(uint16_t), 1, mdl);
	fread(&indi_cnt, sizeof(uint16_t), 1, mdl);

	m->vert_cnt = uint16_flip_endian(vert_cnt);
	m->indi_cnt = uint16_flip_endian(indi_cnt);
	
	m->verts = malloc(m->vert_cnt * sizeof(vertex_t));
	m->indis = malloc(m->indi_cnt * sizeof(uint16_t));
	for(unsigned int i = 0; i < m->vert_cnt; i++) {
		for(unsigned int j = 0; j < 3; j++) {
			uint32_t tmp;
			fread(&tmp, sizeof(float), 1, mdl);
			tmp = uint32_flip_endian(tmp);
			memcpy(m->verts[i].pos + j, &tmp, sizeof(float));
		}

		for(unsigned int j = 0; j < 2; j++) {
			uint32_t tmp;
			fread(&tmp, sizeof(float), 1, mdl);
			tmp = uint32_flip_endian(tmp);
			memcpy(m->verts[i].uv + j, &tmp, sizeof(float));
		}
	}

	for(unsigned int i = 0; i < m->indi_cnt; i++) {
		uint16_t tmp;
		fread(&tmp, sizeof(uint16_t), 1, mdl);
		tmp = uint16_flip_endian(tmp);
		m->indis[i] = tmp;
	}

	m->tex.id = 0;
	if(tex_path)
		m->tex = texture_create_file(tex_path);
}

void gl_mesh_destroy(gl_mesh_t *m)
{
	texture_destroy(&m->tex);
	free(m->verts);
	free(m->indis);
	m->vert_cnt = 0;
	m->indi_cnt = 0;
}

void gl_mesh_draw(const gl_mesh_t *m)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(float), m->verts->pos);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(float), m->verts->uv);
	glDrawElements(GL_TRIANGLES, m->indi_cnt,
			GL_UNSIGNED_SHORT, m->indis);

	glDisable(GL_TEXTURE_2D);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void gl_mesh_draw_custom_tex(const gl_mesh_t *m, texture_t tex,
		float r, float g, float b)
{
	glColor3f(r, g, b);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex.id);
	gl_mesh_draw(m);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

rspq_block_t *gl_mesh_gen_instance_block(const gl_mesh_t *m)
{
	if(m->tex.id > 0) {
		rspq_block_begin();
		glColor3f(1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m->tex.id);
		gl_mesh_draw(m);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		return rspq_block_end();
	}

	rspq_block_begin();
	glDisable(GL_TEXTURE_2D);
	glColor3f(0, 0, 0);
	gl_mesh_draw(m);
	return rspq_block_end();
}

void gl_mesh_draw_instance(rspq_block_t *mesh_block)
{
	rspq_block_run(mesh_block);
}
