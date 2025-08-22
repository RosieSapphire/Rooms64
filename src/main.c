#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#include "util.h"
#include "inputs.h"

#include "engine/object.h"
#include "engine/player.h"

#include "game/room.h"

#define USE_INTERPOLATION 1
#define TICKRATE 30

#define VIEWPORT_NEAR (.5f * MODEL_SCALE)
#define VIEWPORT_FAR (10.f * MODEL_SCALE)
#define VIEWPORT_FOV_DEG 90.f

int main(void)
{
        T3DViewport viewport;
        int dfs_handle;
        float time_accumulated;

        T3DVec3 light_direction;
        uint8_t light_col_direction[4], light_col_ambi[4];

        struct player player;

        /* Initialize Libdragon. */
        display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3,
                     GAMMA_NONE, FILTERS_RESAMPLE);
        rdpq_init();
        joypad_init();
#ifdef DEBUG
        debug_init_usblog();
        debug_init_isviewer();
        rdpq_debug_start();
#endif
        asset_init_compression(COMPRESS_LEVEL);
        dfs_handle = dfs_init(DFS_DEFAULT_LOCATION);

        /* Initialize Tiny3D. */
        {
                T3DInitParams initparams;

                memset(&initparams, 0, sizeof(initparams));
                t3d_init(initparams);
        }

        viewport = t3d_viewport_create();

        /* Initialize game. */
        room_load_next(ROOM_TYPE_00);

        {
                T3DVec3 pos;
                float yaw, pitch;

                pos = t3d_vec3_make(0.f, -3.4f, 0.f);
                yaw = -(M_PI * .5f);
                pitch = 0.f;
                player = player_init(&pos, yaw, pitch, PLAYER_MODE_STANDARD);
        }

        light_direction = t3d_vec3_make(-1.f, 1.f, 0.f);
        t3d_vec3_norm(&light_direction);
        light_col_direction[0] = 0xFF;
        light_col_direction[1] = 0xFF;
        light_col_direction[2] = 0xFF;
        light_col_direction[3] = 0xFF;
        light_col_ambi[0] = 0x20;
        light_col_ambi[1] = 0x20;
        light_col_ambi[2] = 0x20;
        light_col_ambi[3] = 0xFF;

        /* Main loop. */
        time_accumulated = 0.f;

        for (;;) {
                static struct inputs inp_old, inp_new;

                const float fixed_time = 1.f / TICKRATE;
                float subtick;

                /* Updating */
                for (time_accumulated += display_get_delta_time();
                     time_accumulated >= fixed_time;
                     time_accumulated -= fixed_time) {
                        inp_old = inp_new;
                        inp_new = inputs_get_from_libdragon();

                        player_update(&player, &inp_new, &inp_old, fixed_time);
                        room_update(&player.position_b);
                }

                /* Updating -> Rendering */
#if (USE_INTERPOLATION == 1)
                subtick = time_accumulated / fixed_time;
#else
                subtick = 1.f;
#endif

                t3d_viewport_set_projection(&viewport,
                                            T3D_DEG_TO_RAD(VIEWPORT_FOV_DEG),
                                            VIEWPORT_NEAR, VIEWPORT_FAR);
                player_to_view_matrix(&player, &viewport, subtick);
                room_setup_matrices(subtick);

                /* Rendering */
                rdpq_attach(display_get(), display_get_zbuf());
                t3d_frame_start();
                rdpq_mode_dithering(DITHER_NOISE_NONE);
                rdpq_mode_antialias(AA_NONE);

                t3d_viewport_attach(&viewport);
                {
                        uint32_t ambi32;

                        ambi32 = U8ARR_TO_U32PACK(light_col_ambi);
                        t3d_screen_clear_color(color_from_packed32(ambi32));
                }
                t3d_screen_clear_depth();

                t3d_light_set_ambient(light_col_ambi);
                t3d_light_set_directional(0, light_col_direction,
                                          &light_direction);
                t3d_light_set_count(1);

                room_render();

                rdpq_detach_show();
        }

        /* Terminate game. */
        room_terminate();

        /* Terminate Tiny3D. */
        t3d_destroy();

        /* Terminate Libdragon. */
        dfs_close(dfs_handle);
        joypad_close();
        rdpq_close();
#ifdef DEBUG
        rdpq_debug_stop();
#endif
        display_close();
}
