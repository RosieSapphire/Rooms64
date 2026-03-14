#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "rp_types.h"

#ifndef _DEBUG
#define RP_LOG_DISABLE
#endif /* #ifndef _DEBUG */

#define RP_LOG_IMPLEMENTATION
#include "rp_log.h"

#if 0
#define RP_MEMORY_LOG
#define RP_MEMORY_WRAP_STDLIB
#define RP_MEMORY_IMPLEMENTATION
#include "rp_memory.h"
#endif

#define PATH_BUF_LEN	 1024
#define PATH_DIR_BUF_LEN 512

struct vertex {
	f32 pos[3];
	f32 uv[2];
};

struct gl_mesh {
	struct vertex *verts;
	u16	      *indis;
	u16	       vert_cnt;
	u16	       indi_cnt;
	u32	       _pad;
};

static __inline u16 u16_from_u32_safe(const u32 i)
{
	const u32 o = i & 0xFFFF;

	rp_assertf(i == o, "Downcasing u32 value %u to u16 gives %u\n", i, o);

	return (u16)o;
}

static void _mesh_free(struct gl_mesh *m)
{
	free(m->verts);
	m->verts    = NULL;
	m->vert_cnt = 0;

	free(m->indis);
	m->indis    = NULL;
	m->indi_cnt = 0;
}

static struct gl_mesh _mesh_process(const struct aiMesh *ai_mesh, const u32 i)
{
	struct gl_mesh mesh;
	u16	       vert_cnt, indi_cnt, j;

	rp_logf("Mesh %d:\n", i);

	vert_cnt      = u16_from_u32_safe(ai_mesh->mNumVertices);
	mesh.vert_cnt = vert_cnt;
	mesh.verts    = (struct vertex *)malloc(vert_cnt * sizeof(*mesh.verts));
	rp_logf("\n\t%d Verts:\n", vert_cnt);
	for (j = 0; j < vert_cnt; j++) {
		const struct aiVector3D vert_pos = ai_mesh->mVertices[j];
		const struct aiVector3D vert_uv = ai_mesh->mTextureCoords[0][j];
		const f32	       *p	= ((const f32 *)&vert_pos);
		const f32	       *uv	= ((const f32 *)&vert_uv);

		for (int k = 0; k < 3; k++) {
			mesh.verts[j].pos[k] = p[k];

			if (k == 2)
				continue;
			mesh.verts[j].uv[k] = uv[k];
		}

		rp_logf("\t\t(%.3f, %.3f, %.3f), (%.3f, %.3f)\n",
			(f64)p[0],
			(f64)p[1],
			(f64)p[2],
			(f64)uv[0],
			(f64)uv[1]);
	}

	indi_cnt = u16_from_u32_safe(ai_mesh->mNumFaces * 3);
	rp_logf("\t%d Indis:\n", indi_cnt);
	mesh.indi_cnt = indi_cnt;
	mesh.indis    = (u16 *)malloc(indi_cnt * sizeof(*mesh.indis));

	rp_assertf(0 == (indi_cnt % 3),
		   "Index count is not safely divisible by 3.");
	for (j = 0; j < indi_cnt / 3; j++) {
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

static void _mesh_write(struct gl_mesh *m, FILE *out)
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

/*
 * Gets the length of a string that does NOT include null terminator.
 */
static __inline size_t string_get_length(const char *s, const size_t max)
{
	size_t l = 0;

	while (*s && l < max) {
		++s;
		++l;
	}

	return l;
}

static void path_out_end_ensure_slash(char o[PATH_DIR_BUF_LEN])
{
	const size_t ol		   = string_get_length(o, PATH_DIR_BUF_LEN);
	size_t	     end_slash_cnt = 0;

	(void)o;
	(void)ol;

	/* Figure out how many slashes are at the end of the directory path */
	{
		const char *s = o + ol - 1;
		while (*s == '/' && (s - o) > 0) {
			--s;
			++end_slash_cnt;
		}
	}

	/* Ensure that there is only a single slash at the end of the path */
	o[(ol - end_slash_cnt) + 0] = '/';
	o[(ol - end_slash_cnt) + 1] = '\0';
}

static void path_get_mdl_output_path(char	o[PATH_BUF_LEN],
				     const char i[PATH_BUF_LEN],
				     const char d[PATH_DIR_BUF_LEN])
{
	const size_t dl = string_get_length(d, PATH_DIR_BUF_LEN);
	const char  *end, *ext;
	size_t	     end_len, ext_len;

	(void)o;
	(void)i;
	(void)d;
	(void)dl;

	/* Get just the end from the input path */
	end = strrchr(i, '/') + 1;
	rp_assertf(end, "Failed to get end from input path \"%s\"\n", i);
	end_len = string_get_length(end, PATH_BUF_LEN / 2);

	/* Get just the extension from the end we got */
	ext = strrchr(end, '.') + 1;
	rp_assertf(ext, "Failed to get extension from end \"%s\"\n", end);

	/* Get the length of the extension and make sure it's 3 characters. */
	ext_len = string_get_length(ext, 8);
	rp_assertf(ext_len == 3,
		   "Extension length must be 3 chars, but "
		   "it is %lu chars, and the string is \"%s\"",
		   ext_len,
		   ext);

	/* And just to be safe, make sure our input extension is ".glb" */
	rp_assertf(0 == strncmp(ext, "glb", 3),
		   "Input file's extension must be "
		   "\"glb\", but it is \"%s\" instead",
		   ext);

	/* Copy the output path over to the final buffer */
	memcpy(o, d, dl);
	o[dl] = '\0';

	/* Append the name to the end of it and change the file extension */
	memcpy(o + dl, end, end_len);
	memcpy(o + dl + end_len - 3, "mdl", 3);
	o[dl + end_len] = '\0';
}

int main(int argc, char **argv)
{
	static const u32 import_flags =
			aiProcess_Triangulate | aiProcess_JoinIdenticalVertices;
	const struct aiScene *scene;
	struct gl_mesh	     *meshes;
	FILE		     *out_file;
	size_t		      path_input_len, path_output_dir_len;
	char path_input[PATH_BUF_LEN], path_output_dir[PATH_DIR_BUF_LEN],
			path_output[PATH_BUF_LEN];
	u32 mesh_cnt, i;

	/* Verify input is valid */
	rp_assertf(argv[0], "Somehow program name is not accounted for.");
	rp_assertf(argc == 3, "usage: %s [input_glb] [output_dir]", argv[0]);
	rp_assertf(argv[1], "Input file argument is invalid");
	rp_assertf(argv[2], "Output directory argument is invalid");

	/*
	 * Input path processing
	 */
	path_input_len = string_get_length(argv[1], 2048);
	rp_assertf(path_input_len < PATH_BUF_LEN,
		   "Input path's length is greater than "
		   "maximum buffer size (is %lu; max is %lu)",
		   path_input_len,
		   PATH_BUF_LEN - 1);

	memcpy(path_input, argv[1], path_input_len);
	path_input[path_input_len] = '\0';

	/*
	 * Output path processing
	 */
	path_output_dir_len = string_get_length(argv[2], 2048);
	rp_assertf(path_output_dir_len < PATH_DIR_BUF_LEN,
		   "Input path's length is greater than "
		   "maximum buffer size (is %lu; max is %lu)",
		   path_output_dir_len,
		   PATH_DIR_BUF_LEN - 1);

	memcpy(path_output_dir, argv[2], path_output_dir_len);
	path_output_dir[path_output_dir_len] = '\0';

	path_out_end_ensure_slash(path_output_dir);
	path_get_mdl_output_path(path_output, path_input, path_output_dir);

	/* Logging the input and output paths */
	rp_logf("Input Path  : \"%s\"\n", path_input);
	rp_logf("Output Path : \"%s\"\n", path_output);

	/* Load the scene from the input file */
	scene = aiImportFile(path_input, import_flags);
	rp_assertf(scene, "Failed to load file from %s", path_input);

	/* Convert the meshes over from the input GLB file */
	mesh_cnt = scene->mNumMeshes;
	rp_logf("Mesh Count: %d\n", mesh_cnt);
	rp_assertf(mesh_cnt > 0,
		   "Scene loaded from \"%s\" has 0 meshes.",
		   path_input);
	meshes = (struct gl_mesh *)malloc(sizeof(struct gl_mesh) * mesh_cnt);
	for (i = 0; i < mesh_cnt; i++)
		meshes[i] = _mesh_process(scene->mMeshes[i], i);

	/* Write the meshes out to the output file */
	out_file = fopen(path_output, "wb");
	fwrite(&mesh_cnt, 2, 1, out_file);
	for (i = 0; i < mesh_cnt; i++)
		_mesh_write(meshes + i, out_file);

	fclose(out_file);

	/* Free everything */
	for (i = 0; i < mesh_cnt; ++i)
		_mesh_free(meshes + i);
	free(meshes);

	return 0;
}
