#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "rp_types.h"

#define RP_LOG_IMPLEMENTATION
#include "rp_log.h"

#ifdef _DEBUG
#define MODELCONV_DEBUG
#endif /* #ifdef _DEBUG */

struct vertex {
	f32 pos[3];
	f32 uv[2];
};

struct gl_mesh {
	struct vertex *verts;
	u16	      *indis;
	void	      *spr;
	u32	       tex;
	u16	       vert_cnt;
	u16	       indi_cnt;
};

/* FIXME: This only accounts for the extension being 3 characters */
char *path_remove_extension(const char *buf)
{
	int len	  = strlen(buf);
	char *new = malloc(len - 3);
	snprintf(new, len - 3, "%s", buf);
	return new;
}

static struct gl_mesh _mesh_process(const struct aiMesh *ai_mesh, int i)
{
	struct gl_mesh mesh;
	rp_logf("Mesh %d:\n", i);

	const u16 vert_cnt = ai_mesh->mNumVertices;
	mesh.vert_cnt	   = vert_cnt;
	mesh.verts	   = malloc(vert_cnt * sizeof(*mesh.verts));
	rp_logf("\n\t%d Verts:\n", vert_cnt);
	for (int j = 0; j < vert_cnt; j++) {
		const struct aiVector3D vert_pos = ai_mesh->mVertices[j];
		const struct aiVector3D vert_uv = ai_mesh->mTextureCoords[0][j];

		f32 *p	= ((f32 *)&vert_pos);
		f32 *uv = ((f32 *)&vert_uv);
		for (int k = 0; k < 3; k++) {
			mesh.verts[j].pos[k] = p[k];

			if (k == 2)
				continue;
			mesh.verts[j].uv[k] = uv[k];
		}

		rp_logf("\t\t(%.3f, %.3f, %.3f), (%.3f, %.3f)\n",
			p[0],
			p[1],
			p[2],
			uv[0],
			uv[1]);
	}

	const u16 num_indis = ai_mesh->mNumFaces * 3;
	rp_logf("\t%d Indis:\n", num_indis);
	mesh.indi_cnt = num_indis;
	mesh.indis    = malloc(num_indis * sizeof(*mesh.indis));
	for (int j = 0; j < num_indis / 3; j++) {
		const struct aiFace face = ai_mesh->mFaces[j];

		rp_logf("\t\t");
		for (int k = 0; k < 3; k++) {
			rp_logf("%d ", face.mIndices[k]);
			memcpy(mesh.indis + (j * 3 + k),
			       face.mIndices + k,
			       sizeof(u16));
		}
		rp_logf("\n");
	}

	return mesh;
}

static void _mesh_write(struct gl_mesh *m, int i, FILE *out)
{
	fwrite(&m->vert_cnt, 2, 1, out);
	fwrite(&m->indi_cnt, 2, 1, out);

	for (int j = 0; j < m->vert_cnt; j++) {
		for (int k = 0; k < 3; k++)
			fwrite(m->verts[j].pos + k, sizeof(f32), 1, out);

		for (int k = 0; k < 2; k++)
			fwrite(m->verts[j].uv + k, sizeof(f32), 1, out);
	}

	fwrite(m->indis, 2, m->indi_cnt, out);
}

int main(int argc, char **argv)
{
	static const u32 import_flags =
			aiProcess_Triangulate | aiProcess_JoinIdenticalVertices;
	const struct aiScene *scene;
	const char	     *path_input, *path_real;
	char		     *just_name;

	rp_assertf(argv[0], "Somehow program name is not accounted for.");
	rp_assertf(argc == 2, "usage: %s [input_glb]", argv[0]);

	path_input = argv[1];
	path_real  = realpath(path_input, NULL);
	just_name  = path_remove_extension(path_input);
	scene	   = aiImportFile(path_real, import_flags);
	if (!scene) {
		fprintf(stderr,
			"ERROR: Failed to load file from %s\n",
			path_real);
		exit(1);
	}

	int num_meshes = scene->mNumMeshes;
	rp_logf("Mesh Count: %d\n", num_meshes);
	rp_assertf(num_meshes > 0,
		   "Scene loaded from \"%s\" has 0 meshes.",
		   path_input);
	struct gl_mesh *meshes = malloc(sizeof(struct gl_mesh) * num_meshes);
	for (int i = 0; i < num_meshes; i++)
		meshes[i] = _mesh_process(scene->mMeshes[i], i);

	const char *outpath = strcat(just_name, ".mdl");
	FILE	   *out	    = fopen(outpath, "wb");
	if (out) {
		remove(outpath);
		fclose(out);
		out = fopen(outpath, "wb");
	}

	fwrite(&num_meshes, 2, 1, out);
	for (int i = 0; i < num_meshes; i++)
		_mesh_write(meshes + i, i, out);

	fclose(out);

	return 0;
}
