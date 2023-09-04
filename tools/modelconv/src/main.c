#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string.h>
#include "vertex.h"
#include "gl_mesh.h"

char *remove_extension(const char *buf)
{
	int len = strlen(buf);
	char *new = malloc(len - 3);
	snprintf(new, len - 3, "%s", buf);
	return new;
}

static gl_mesh_t _mesh_process(const struct aiMesh *ai_mesh, int i)
{
	gl_mesh_t mesh;
	printf("Mesh %d:\n", i);

	const uint16_t vert_cnt = ai_mesh->mNumVertices;
	mesh.vert_cnt = vert_cnt;
	mesh.verts = malloc(vert_cnt * sizeof(*mesh.verts));
	printf("\n\t%d Verts:\n", vert_cnt);
	for(int j = 0; j < vert_cnt; j++) {
		const struct aiVector3D vert_pos = ai_mesh->mVertices[j];
		const struct aiVector3D vert_uv = ai_mesh->mTextureCoords[0][j];

		float *p = ((float *)&vert_pos);
		float *uv = ((float *)&vert_uv);
		for(int k = 0; k < 3; k++) {
			mesh.verts[j].pos[k] = p[k];

			if(k == 2)
				continue;
			mesh.verts[j].uv[k] = uv[k];
		}

		printf("\t\t(%.3f, %.3f, %.3f), (%.3f, %.3f)\n",
				p[0], p[1], p[2], uv[0], uv[1]);
	}

	const uint16_t num_indis = ai_mesh->mNumFaces * 3;
	printf("\t%d Indis:\n", num_indis);
	mesh.indi_cnt = num_indis;
	mesh.indis = malloc(num_indis * sizeof(*mesh.indis));
	for(int j = 0; j < num_indis / 3; j++) {
		const struct aiFace face = ai_mesh->mFaces[j];

		printf("\t\t");
		for(int k = 0; k < 3; k++) {
			printf("%d ", face.mIndices[k]);
			memcpy(mesh.indis + (j * 3 + k), face.mIndices + k,
					sizeof(uint16_t));
		}
		printf("\n");
	}


	return mesh;
}


static void _mesh_write(gl_mesh_t *m, int i, FILE *out)
{
	fwrite(&m->vert_cnt, 2, 1, out);
	fwrite(&m->indi_cnt, 2, 1, out);

	for(int j = 0; j < m->vert_cnt; j++) {
		for(int k = 0; k < 3; k++)
			fwrite(m->verts[j].pos + k, sizeof(float), 1, out);

		for(int k = 0; k < 2; k++)
			fwrite(m->verts[j].uv + k, sizeof(float), 1, out);
	}

	fwrite(m->indis, 2, m->indi_cnt, out);
}

int main(int argc, char **argv)
{
	assert(argc == 2);

	const char *filename = argv[1];
	const char *filepath = realpath(filename, NULL);
	char *just_name = remove_extension(filename);
	const struct aiScene *scene = aiImportFile(filepath,
			aiProcess_Triangulate | 
			aiProcess_JoinIdenticalVertices);
	if(!scene) {
		fprintf(stderr, "ERROR: Failed to load file from %s\n",
				filepath);
		exit(1);
	}

	int num_meshes = scene->mNumMeshes;
	printf("Mesh Count: %d\n", num_meshes);
	assert(num_meshes > 0);
	gl_mesh_t *meshes = malloc(sizeof(gl_mesh_t) * num_meshes);
	for(int i = 0; i < num_meshes; i++)
		meshes[i] = _mesh_process(scene->mMeshes[i], i);

	const char *outpath = strcat(just_name, ".mdl");
	FILE *out = fopen(outpath, "wb");
	if(out) {
		remove(outpath);
		fclose(out);
		out = fopen(outpath, "wb");
	}

	fwrite(&num_meshes, 2, 1, out);
	for(int i = 0; i < num_meshes; i++)
		_mesh_write(meshes + i, i, out);

	fclose(out);

	return 0;
}
