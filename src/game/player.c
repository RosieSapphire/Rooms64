#include "game/player.h"

#include "util.h"
#include "perlin.h"

#define PLAYER_TURN_SPEED_SLOW 3.f
#define PLAYER_TURN_SPEED_FAST 16.f
#define PLAYER_TURN_LERP_SPEED 8.f
#define PLAYER_TURN_EPSILON .001f
#define PLAYER_STOP_SPEED .01f
#define PLAYER_FRICTION 6.f

#define PLAYER_MAX_SPEED 0.82f
#define PLAYER_NOCLIP_SPEED_SLOW 4.2f
#define PLAYER_NOCLIP_SPEED_FAST 12.2f
#define PLAYER_HEADBOB_SCALE 0.03f
#define PLAYER_HEADBOB_FACTOR 48.f

struct player player_init(const T3DVec3 *spawn_pos, const float spawn_yaw,
                          const float spawn_pitch, const uint8_t mode)
{
        struct player p;

        p.position_a = *spawn_pos;
        p.position_b = p.position_a;
        p.up = t3d_vec3_zup();
        p.velocity = t3d_vec3_zero();
        p.headbob_vec_a = t3d_vec3_zero();
        p.headbob_vec_b = t3d_vec3_zero();
        p.headbob_timer_a = 0.f;
        p.headbob_timer_b = 0.f;
        p.yaw_tar = spawn_yaw;
        p.yaw_a = p.yaw_tar;
        p.yaw_b = p.yaw_tar;
        p.pitch_tar = spawn_pitch;
        p.pitch_a = p.pitch_tar;
        p.pitch_b = p.pitch_tar;
        p.has_played_footstep = false;
        p.mode = mode;

        return p;
}

static void player_update_turning_normal(struct player *p,
                                         const struct inputs *inp,
                                         const float ft)
{
        float turn_speed, pitch_mul, turn_lerp_t, pitch_limit;

        turn_speed = PLAYER_TURN_SPEED_SLOW;
        if (inp->btn[BTN_Z])
                turn_speed = PLAYER_TURN_SPEED_FAST;
        p->yaw_tar -= inp->stick.v[0] * turn_speed * ft;

        pitch_mul = (inp->btn[BTN_Z]) ? .35f : 1.f;
        p->pitch_tar -= inp->stick.v[1] * pitch_mul * turn_speed * ft;

        pitch_limit = T3D_DEG_TO_RAD(85.f);
        if (p->pitch_tar > pitch_limit)
                p->pitch_tar = pitch_limit;

        if (p->pitch_tar < -pitch_limit)
                p->pitch_tar = -pitch_limit;

        turn_lerp_t = clampf(ft * PLAYER_TURN_LERP_SPEED, 0.f, 1.f);
        p->yaw_a = p->yaw_b;
        p->yaw_b = lerpf(p->yaw_b, p->yaw_tar, turn_lerp_t);
        p->pitch_a = p->pitch_b;
        p->pitch_b = lerpf(p->pitch_b, p->pitch_tar, turn_lerp_t);

        if (fabsf(p->yaw_b - p->yaw_tar) < PLAYER_TURN_EPSILON)
                p->yaw_b = p->yaw_tar;

        if (fabsf(p->pitch_b - p->pitch_tar) < PLAYER_TURN_EPSILON)
                p->pitch_b = p->pitch_tar;
}

#ifdef PLAYER_NOCLIP
static void player_update_turning_noclip(struct player *p,
                                         const struct inputs *inp,
                                         const float ft)
{
        float turn_speed, pitch_limit;

        turn_speed = PLAYER_TURN_SPEED_SLOW;
        if (inp->btn[BTN_Z])
                turn_speed = PLAYER_TURN_SPEED_FAST;

        p->yaw_tar -= inp->stick.v[0] * turn_speed * ft;
        p->pitch_tar -= inp->stick.v[1] * turn_speed * ft;

        pitch_limit = T3D_DEG_TO_RAD(85.f);
        if (p->pitch_tar > pitch_limit)
                p->pitch_tar = pitch_limit;

        if (p->pitch_tar < -pitch_limit)
                p->pitch_tar = -pitch_limit;

        p->yaw_a = p->yaw_b;
        p->yaw_b = p->yaw_tar;
        p->pitch_a = p->pitch_b;
        p->pitch_b = p->pitch_tar;
}
#endif

static void player_update_friction(struct player *p, const float ft)
{
        float control, drop, speed, new_speed;

        speed = t3d_vec3_len(&p->velocity);
        if (speed <= 0.f)
                return;

        control = (speed < PLAYER_STOP_SPEED) ? PLAYER_STOP_SPEED : speed;
        drop = control * PLAYER_FRICTION * ft;
        new_speed = speed - drop;
        if (new_speed < 0.f)
                new_speed = 0.f;

        new_speed /= speed;
        t3d_vec3_scale(&p->velocity, &p->velocity, new_speed);
}

static void player_update_moving_normal(struct player *p,
                                        const struct inputs *inp,
                                        const float ft)
{
        T3DVec3 forw_move, right_move, move_vec;
        T3DVec2 accel_dir;

        accel_dir.v[0] = inp->btn[BTN_C_RIGHT] - inp->btn[BTN_C_LEFT];
        accel_dir.v[1] = inp->btn[BTN_C_UP] - inp->btn[BTN_C_DOWN];

        if (!accel_dir.v[0] && !accel_dir.v[1]) {
                p->position_a = p->position_b;
                t3d_vec3_add(&p->position_b, &p->position_b, &p->velocity);
                return;
        }

        accel_dir = t3d_vec2_normalize(&accel_dir);

        forw_move = player_get_forward_dir(p, 1.f);
        right_move = player_get_right_dir(p, &forw_move);

        forw_move.v[2] = 0.f;
        forw_move = t3d_vec3_normalize(&forw_move);
        right_move.v[2] = 0.f;
        right_move = t3d_vec3_normalize(&right_move);

        t3d_vec3_scale(&forw_move, &forw_move, accel_dir.v[1]);
        t3d_vec3_scale(&right_move, &right_move, accel_dir.v[0]);

        t3d_vec3_add(&move_vec, &forw_move, &right_move);
        t3d_vec3_normalize(&move_vec);
        t3d_vec3_scale(&move_vec, &move_vec, PLAYER_MAX_SPEED * ft);

        t3d_vec3_add(&p->velocity, &p->velocity, &move_vec);

        p->position_a = p->position_b;
        t3d_vec3_add(&p->position_b, &p->position_b, &p->velocity);
}

#ifdef PLAYER_NOCLIP
static void player_update_moving_noclip(struct player *p,
                                        const struct inputs *inp,
                                        const float ft)
{
        T3DVec3 forw_move, right_move, move_vec;
        T3DVec2 input_dir;
        float speed;

        input_dir.v[0] = inp->btn[BTN_C_RIGHT] - inp->btn[BTN_C_LEFT];
        input_dir.v[1] = inp->btn[BTN_C_UP] - inp->btn[BTN_C_DOWN];

        if (!input_dir.v[0] && !input_dir.v[1]) {
                p->position_a = p->position_b;
                return;
        }

        input_dir = t3d_vec2_normalize(&input_dir);

        forw_move = player_get_forward_dir(p, 1.f);
        right_move = player_get_right_dir(p, &forw_move);

        right_move.v[2] = 0.f;
        right_move = t3d_vec3_normalize(&right_move);

        t3d_vec3_scale(&forw_move, &forw_move, input_dir.v[1]);
        t3d_vec3_scale(&right_move, &right_move, input_dir.v[0]);

        t3d_vec3_add(&move_vec, &forw_move, &right_move);
        t3d_vec3_normalize(&move_vec);

        speed = (inp->btn[BTN_R]) ? PLAYER_NOCLIP_SPEED_FAST :
                                    PLAYER_NOCLIP_SPEED_SLOW;
        t3d_vec3_scale(&move_vec, &move_vec, speed * ft);

        p->position_a = p->position_b;
        t3d_vec3_add(&p->position_b, &p->position_b, &move_vec);
}
#endif

static void player_update_headbob(struct player *p, const float ft)
{
        T3DVec3 side, up;
        float speed, sin_bob, sin2_bob;

        speed = t3d_vec3_len(&p->velocity) * PLAYER_HEADBOB_FACTOR;
        p->headbob_timer_a = p->headbob_timer_b;
        p->headbob_timer_b += speed * ft;
        sin_bob = sinf(p->headbob_timer_b);
        sin2_bob = sinf(p->headbob_timer_b * 2);

        /* TODO: Play footstep sound. */
#if 0
        mixer_ch_set_vol(SFXC_FOOTSTEP, speed * 2, speed * 2);

        if(fabsf(sin_bob) >= 0.8f) {
                if(!played_footstep) {
                        wav64_play(&footstep_sfx, 1);
                        played_footstep = true;
                }
        } else {
                played_footstep = false;
        }
#endif

        {
                T3DVec3 forw;

                forw = player_get_forward_dir(p, 1.f);
                side = player_get_right_dir(p, &forw);
        }
        t3d_vec3_scale(&side, &side, sin_bob * speed * PLAYER_HEADBOB_SCALE);

        up = p->up;
        t3d_vec3_scale(&up, &up, sin2_bob * speed * PLAYER_HEADBOB_SCALE * .5f);

        p->headbob_vec_a = p->headbob_vec_b;
        t3d_vec3_add(&p->headbob_vec_b, &side, &up);
}

void player_update(struct player *p, const struct inputs *inp_new,
                   const struct inputs *inp_old, const float ft)
{

#ifdef PLAYER_NOCLIP
        p->mode ^= INPUT_PRESS_PTR(inp_new, inp_old, BTN_START);

        switch (p->mode) {
        case PLAYER_MODE_STANDARD:
#endif
                player_update_turning_normal(p, inp_new, ft);
                player_update_friction(p, ft);
                player_update_moving_normal(p, inp_new, ft);
                player_update_headbob(p, ft);
#ifdef PLAYER_NOCLIP
                return;

        case PLAYER_MODE_NOCLIP:
                player_update_turning_noclip(p, inp_new, ft);
                player_update_moving_noclip(p, inp_new, ft);
                return;
        }
#endif
}

T3DVec3 player_get_forward_dir(const struct player *p, const float subtick)
{
        float yaw_a, yaw_b, yaw, pitch_a, pitch_b, pitch, cos_pitch;
        T3DVec3 dir;

        /* Decide whether or not to get the offset ones for visual effects. */
        yaw_a = p->yaw_a;
        yaw_b = p->yaw_b;
        pitch_a = p->pitch_a;
        pitch_b = p->pitch_b;

        yaw = lerpf(yaw_a, yaw_b, subtick);
        pitch = lerpf(pitch_a, pitch_b, subtick);
        cos_pitch = cosf(pitch);
        dir = t3d_vec3_make(cosf(yaw) * cos_pitch,
                            sinf(yaw) * cos_pitch, sinf(pitch));

        return dir;
}

T3DVec3 player_get_right_dir(const struct player *p, const T3DVec3 *forw_dir)
{
        T3DVec3 right_dir;

        t3d_vec3_cross(&right_dir, forw_dir, &p->up);

        return right_dir;
}

void player_to_view_matrix(const struct player *p, T3DViewport *vp,
                           const float subtick)
{
        T3DVec3 head_offset, bob, eye, forw_dir, focus;

        /* Eye. */
        head_offset = t3d_vec3_make(0.f, 0.f, PLAYER_HEIGHT);
        t3d_vec3_lerp(&bob, &p->headbob_vec_a, &p->headbob_vec_b, subtick);
        t3d_vec3_add(&head_offset, &head_offset, &bob);
        t3d_vec3_lerp(&eye, &p->position_a, &p->position_b, subtick);
        t3d_vec3_add(&eye, &eye, &head_offset);
        t3d_vec3_scale(&eye, &eye, MODEL_SCALE);

        /* Focus. */
        forw_dir = player_get_forward_dir(p, subtick);
        t3d_vec3_add(&focus, &eye, &forw_dir);

        t3d_viewport_look_at(vp, &eye, &focus, &p->up);
}
