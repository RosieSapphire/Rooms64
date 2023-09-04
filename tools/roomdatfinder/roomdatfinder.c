#include <assimp/matrix4x4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#define MAX_LOCKERS 32

char *remove_extension(const char *buf)
{
	int len = strlen(buf);
	char *new = malloc(len - 3);
	snprintf(new, len - 3, "%s", buf);
	return new;
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

	uint16_t num_elements = scene->mRootNode->mNumChildren;
	printf("Element Count: %d\n", num_elements);
	assert(num_elements > 0);
	const char *outpath = strcat(just_name, ".dat");
	FILE *data_file = fopen(outpath, "wb");
	if(data_file) {
		fclose(data_file);
		remove(outpath);
		data_file = fopen(outpath, "wb");
	}

	int num_lockers = 0;
	int locker_inds[MAX_LOCKERS];
	for(int i = 0; i < num_elements; i++) {
		const char *name = scene->mRootNode->mChildren[i]->mName.data;
		if(!strncmp("LockerPos", name, strlen("LockerPos"))) {
			locker_inds[num_lockers++] = i;
			continue;
		}
	}

	printf("Locker Count: %d\n", num_lockers);
	fwrite(&num_lockers, 2, 1, data_file);
	for(int i = 0; i < num_lockers; i++) {
		const struct aiMatrix4x4 mat =
			scene->mRootNode->mChildren[locker_inds[i]]->
			mTransformation;
		const char *name =
			scene->mRootNode->mChildren[locker_inds[i]]->
			mName.data;

		fwrite(&mat.a4, sizeof(float), 1, data_file);
		fwrite(&mat.b4, sizeof(float), 1, data_file);
		fwrite(&mat.c4, sizeof(float), 1, data_file);
		printf("\t%d: %f, %f, %f\n", i, mat.a4, mat.b4, mat.c4);
	}
	fclose(data_file);

	return 0;
}
