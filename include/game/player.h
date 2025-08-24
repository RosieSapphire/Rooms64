#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include <t3d/t3d.h>

#include "util.h"
#include "inputs.h"

#define PLAYER_HEIGHT 1.24f

enum {
        PLAYER_MODE_STANDARD,
        PLAYER_MODE_NOCLIP,
        PLAYER_MODE_COUNT
};

struct player {
        T3DVec3 position_a;
        T3DVec3 position_b;
        T3DVec3 up;
        T3DVec3 velocity;
        T3DVec3 headbob_vec_a;
        T3DVec3 headbob_vec_b;
        float headbob_timer_a;
        float headbob_timer_b;
        float yaw_tar;
        float yaw_a;
        float yaw_b;
        float pitch_tar;
        float pitch_a;
        float pitch_b;
        bool has_played_footstep;
        uint8_t mode;
};

struct player player_spawn(const T3DVec3 *spawn_pos, const float spawn_yaw,
                           const float spawn_pitch, const uint8_t mode);
T3DVec3 player_get_forward_dir(const struct player *p, const float subtick);
T3DVec3 player_get_right_dir(const struct player *p, const T3DVec3 *forw_dir);
void player_update(struct player *p, const struct inputs *inp_new,
                   const struct inputs *inp_old, const float ft);
void player_to_view_matrix(const struct player *p, T3DViewport *vp,
                           const float subtick);

#endif /* GAME_PLAYER_H */
