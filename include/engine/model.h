#ifndef ENGINE_MODEL_H_
#define ENGINE_MODEL_H_

#include "gl_mesh.h"

typedef struct {
	uint16_t mesh_cnt;
	gl_mesh_t *meshes;
	rspq_block_t **blocks;
} model_t;

void model_create_file(model_t *m, const char *mdl_path,
		const char **tex_paths);
void model_draw_mesh_index(const model_t *m, int i);
void model_draw_mesh_index_custom_tex(const model_t *m, int i, texture_t tex,
		float r, float g, float b);
void model_draw(const model_t *m);

#endif /* ENGINE_MODEL_H_ */
