#include "engine/player.h"

#include "util.h"
#include "perlin.h"

#define PLAYER_TURN_SPEED_SLOW 3.f
#define PLAYER_TURN_SPEED_FAST 16.f
#define PLAYER_TURN_LERP_SPEED 8.f
#define PLAYER_TURN_EPSILON .001f
#define PLAYER_STOP_SPEED .01f
#define PLAYER_FRICTION 6.f

#define PLAYER_MAX_SPEED_SLOW 24.f
#define PLAYER_MAX_SPEED_FAST 48.f

struct player player_create(const T3DVec3 *spawn_pos, const float spawn_yaw,
                            const float spawn_pitch)
{
        struct player p;

        p.position_a = *spawn_pos;
        p.position_b = p.position_a;
        p.up_real = t3d_vec3_zup();
        p.up_visual_a = p.up_real;
        p.up_visual_b = p.up_real;
        p.velocity = t3d_vec3_zero();
        p.head_wiggle_amount = .02f;
        p.yaw_tar = spawn_yaw;
        p.yaw_a = p.yaw_tar;
        p.yaw_b = p.yaw_tar;
        p.yaw_offset_a = p.yaw_a;
        p.yaw_offset_b = p.yaw_b;
        p.pitch_tar = spawn_pitch;
        p.pitch_a = p.pitch_tar;
        p.pitch_b = p.pitch_tar;
        p.pitch_offset_a = p.pitch_a;
        p.pitch_offset_b = p.pitch_b;
        p.state_timer = 0.f;
        p.state = PLAYER_STATE_STANDING;

        return p;
}

static void player_update_turning(struct player *p, const struct inputs *inp,
                                  const float ft)
{
        float turn_speed, pitch_mul, turn_lerp_t, pitch_limit;

        turn_speed = PLAYER_TURN_SPEED_SLOW;
        if ((p->state == PLAYER_STATE_STANDING) && inp->btn[BTN_Z])
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
        p->velocity = t3d_vec3_scale(&p->velocity, new_speed);
}

static void player_update_moving(struct player *p, const struct inputs *inp,
                                 const float ft)
{
        T3DVec3 forw_move, right_move, move_vec;
        T3DVec2 accel_dir;
        float move_speed;

        accel_dir.v[0] = inp->btn[BTN_C_RIGHT] - inp->btn[BTN_C_LEFT];
        accel_dir.v[1] = inp->btn[BTN_C_UP] - inp->btn[BTN_C_DOWN];

        if (!accel_dir.v[0] && !accel_dir.v[1]) {
                p->position_a = p->position_b;
                t3d_vec3_add(&p->position_b, &p->position_b, &p->velocity);
                p->state = PLAYER_STATE_STANDING;
                return;
        }

        p->state = PLAYER_STATE_MOVING;

        accel_dir = t3d_vec2_normalize(&accel_dir);

        forw_move = player_get_forward_dir(p, 1.f, false);
        right_move = player_get_right_dir(p, &forw_move);

        forw_move.v[2] = 0.f;
        forw_move = t3d_vec3_normalize(&forw_move);

        forw_move = t3d_vec3_scale(&forw_move, accel_dir.v[1]);
        right_move = t3d_vec3_scale(&right_move, accel_dir.v[0]);

        t3d_vec3_add(&move_vec, &forw_move, &right_move);
        t3d_vec3_normalize(&move_vec);
        move_speed = ((inp->btn[BTN_Z]) ? PLAYER_MAX_SPEED_FAST :
                                          PLAYER_MAX_SPEED_SLOW) * ft;
        move_vec = t3d_vec3_scale(&move_vec, move_speed * ft);

        t3d_vec3_add(&p->velocity, &p->velocity, &move_vec);

        p->position_a = p->position_b;
        t3d_vec3_add(&p->position_b, &p->position_b, &p->velocity);
}

static void player_update_head_wiggle(struct player *p)
{
        float wigglex0, wiggley0, wigglex1, wiggley1, tscale;

        p->up_visual_a = p->up_visual_b;
        p->yaw_offset_a = p->yaw_offset_b;
        p->pitch_offset_a = p->pitch_offset_b;
        tscale = p->head_wiggle_amount;
        wigglex0 = perlinf(p->state_timer, 0.f) * tscale;
        wiggley0 = perlinf(0.f, p->state_timer) * tscale;
        wigglex1 = perlinf(p->state_timer + 1024.f, 0.f) * tscale;
        wiggley1 = perlinf(0.f, p->state_timer + 1024.f) * tscale;

        p->up_visual_b = t3d_vec3_make(wigglex0, wiggley0, 1.f);
        t3d_vec3_normalize(&p->up_visual_b);
        p->yaw_offset_b = wigglex1;
        p->pitch_offset_b = wiggley1;
}

void player_update(struct player *p, const struct inputs *inp, const float ft)
{
        uint8_t state_old;

        state_old = p->state;

        player_update_turning(p, inp, ft);
        player_update_friction(p, ft);
        player_update_moving(p, inp, ft);
        player_update_head_wiggle(p);

        p->state_timer += ft;
        if (p->state ^ state_old)
                p->state_timer = 0.f;
}

T3DVec3 player_get_forward_dir(const struct player *p, const float subtick,
                               bool get_offset)
{
        float yaw_a, yaw_b, yaw, pitch_a, pitch_b, pitch, cos_pitch;
        T3DVec3 dir;

        /* Decide whether or not to get the offset ones for visual effects. */
        yaw_a = p->yaw_a;
        yaw_b = p->yaw_b;
        pitch_a = p->pitch_a;
        pitch_b = p->pitch_b;

        if (get_offset) {
                yaw_a += p->yaw_offset_a;
                yaw_b += p->yaw_offset_b;
                pitch_a += p->pitch_offset_a;
                pitch_b += p->pitch_offset_b;
        }

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

        t3d_vec3_cross(&right_dir, forw_dir, &p->up_real);

        return right_dir;
}

void player_to_view_matrix(const struct player *p, T3DViewport *vp,
                           const float subtick)
{
        T3DVec3 head_offset, eye, forw_dir, focus, up;

        /* Eye. */
        head_offset = t3d_vec3_make(0.f, 0.f, PLAYER_HEIGHT);
        t3d_vec3_lerp(&eye, &p->position_a, &p->position_b, subtick);
        t3d_vec3_add(&eye, &eye, &head_offset);
        eye = t3d_vec3_scale(&eye, MODEL_SCALE);

        /* Focus. */
        forw_dir = player_get_forward_dir(p, subtick, true);
        t3d_vec3_add(&focus, &eye, &forw_dir);

        t3d_vec3_lerp(&up, &p->up_visual_a, &p->up_visual_b, subtick);

        t3d_viewport_look_at(vp, &eye, &focus, &up);
}
