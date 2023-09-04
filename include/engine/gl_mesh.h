#ifndef GL_MESH_H_
#define GL_MESH_H_

#include "vertex.h"
#include <libdragon.h>
#include "texture.h"

typedef struct {
	uint16_t vert_cnt, indi_cnt;
	vertex_t *verts;
	uint16_t *indis;
	texture_t tex;
} gl_mesh_t;

void gl_mesh_create_data(gl_mesh_t *m, unsigned int vert_cnt,
		const vertex_t *verts, unsigned int indi_cnt,
		const unsigned int *indis, const char *tex_path);
void gl_mesh_create_file(gl_mesh_t *m, const char *tex_path, FILE *mdl);
void gl_mesh_destroy(gl_mesh_t *m);
void gl_mesh_draw(const gl_mesh_t *m);
void gl_mesh_draw_custom_tex(const gl_mesh_t *m, texture_t tex,
		float r, float g, float b);
rspq_block_t *gl_mesh_gen_instance_block(const gl_mesh_t *m);
void gl_mesh_draw_instance(rspq_block_t *mesh_block);

#endif /* GL_MESH_H_ */
