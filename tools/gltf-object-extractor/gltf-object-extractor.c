#include <stdio.h>
#include <stdbool.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

enum {
        RET_OKAY,
        RET_INVALID_ARGC,
        RET_CGLTF_FAILED,
        RET_DOORPOS_NOT_FOUND,
        RET_TYPE_CNT
};

struct room_file {
        float door_pos[3];
        uint16_t obj_cnt;
        void *objs;
};

static void fwrite_f32_le(float *ptr, size_t cnt, FILE *f)
{
        size_t i;
        uint32_t *arr;

        arr = malloc(cnt * sizeof(float));

        for (i = 0; i < cnt; ++i) {
                uint32_t b;

                b = *((uint32_t *)(ptr + i));
                b = ((b & 0x000000FF) << 24) |
                    ((b & 0x0000FF00) << 8) |
                    ((b & 0x00FF0000) >> 8) |
                    ((b & 0xFF000000) >> 24);
                arr[i] = b;
        }

        fwrite(arr, sizeof(float), cnt, f);

        free(arr);
}

int main(int argc, char **argv)
{
        const char *gltf_path = NULL;
        const char *room_path = NULL;
        cgltf_options options = { 0 };
        cgltf_data *data = NULL;
        cgltf_result result = 0;
        struct room_file room_out;
        bool has_found_doorpos;

        if (argc != 3) {
                printf("ERROR: Must have input & output path as arguments.\n");
                return RET_INVALID_ARGC;
        }

        gltf_path = argv[1];
        room_path = argv[2];

        result = cgltf_parse_file(&options, gltf_path, &data);
        if (result != cgltf_result_success) {
                printf("GLTF file loaded from '%s' failed.\n", gltf_path);
                return RET_CGLTF_FAILED;
        }

        has_found_doorpos = false;
        for (size_t i = 0; i < data->nodes_count; ++i) {
                cgltf_node *n;

                n = data->nodes + i;
                if (strncmp(n->name, "DoorPos", 7))
                        continue;

                memcpy(room_out.door_pos, n->translation, 3 * sizeof(float));
                printf("DoorPos: %f, %f, %f\n", room_out.door_pos[0],
                       room_out.door_pos[1], room_out.door_pos[2]);
                has_found_doorpos = true;
        }

        if (!has_found_doorpos) {
                printf("ERROR: Couldn't find doorpos in room from '%s'.\n",
                       gltf_path);
                return RET_DOORPOS_NOT_FOUND;
        }

        room_out.obj_cnt = 0;
        room_out.objs = NULL;

        {
                FILE *out_file;

                out_file = fopen(room_path, "wb");
                fwrite_f32_le(room_out.door_pos, 3, out_file);
                fclose(out_file);

                /* TODO: Add objects data. */
        }

        cgltf_free(data);

        return RET_OKAY;
}
