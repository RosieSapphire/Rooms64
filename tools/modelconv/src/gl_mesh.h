#ifndef GL_MESH_H_
#define GL_MESH_H_

#include "vertex.h"

typedef struct {
	uint16_t vert_cnt, indi_cnt;
	vertex_t *verts;
	uint16_t *indis;
	GLuint tex;
	void *spr;
} gl_mesh_t;

#endif /* GL_MESH_H_ */
