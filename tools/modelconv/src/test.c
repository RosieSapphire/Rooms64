#include <stdio.h>
#include <stdlib.h>
#include "vertex.h"

int main(void)
{
	FILE *file = fopen("out.mdl", "rb");
	if(!file) {
		fprintf(stderr, "ERROR: Couldn't find model.\n");
		exit(1);
	}

	uint16_t vert_cnt, indi_cnt;
	fread(&vert_cnt, 2, 1, file);
	fread(&indi_cnt, 2, 1, file);
	printf("%d, %d\n", vert_cnt, indi_cnt);

	vertex_t *verts = malloc(sizeof(vertex_t) * vert_cnt);
	for(int i = 0; i < vert_cnt; i++) {
		for(int j = 0; j < 3; j++)
			fread(verts[i].pos + j, sizeof(float), 1, file);

		for(int j = 0; j < 2; j++)
			fread(verts[i].uv + j, sizeof(float), 1, file);

		float *pos = verts[i].pos;
		float *uv = verts[i].uv;
		printf("(%f, %f, %f), (%f, %f)\n",
				pos[0], pos[1], pos[2],
				uv[0], uv[1]);
	}

	uint16_t *indis = malloc(2 * indi_cnt);
	fread(indis, 2, indi_cnt, file);

	for(int i = 0; i < indi_cnt / 3; i++) {
		uint16_t *ind = indis + (i * 3);
		printf("%d, %d, %d\n", ind[0], ind[1], ind[2]);
	}

	fclose(file);
	free(indis);
	free(verts);

	return 0;
}
