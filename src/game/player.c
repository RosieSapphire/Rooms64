#include <malloc.h>
#include <math.h>
#include <GL/glu.h>

#include "engine/util.h"
#include "engine/debug.h"
#include "engine/sfx.h"
#include "engine/raycast.h"
#include "engine/vector.h"

#include "game/player.h"
#include "game/locker.h"
#include "game/entity.h"

#define TURN_SPEED    0.2f
#define FRICTION      6.0f
#define STOP_SPEED    0.05f
#define MAX_SPEED     0.3f
#define MAX_SPEED_AIR 0.01875f
#define ACCEL	      4.0f
#define JUMP_FORCE    0.14f
#define GRAVITY	      0.6f
#define HEADBOB_SCALE 0.7f

#define DELTA_TIME (1.0f / 24.0f)

enum player_state pstate       = PS_NORMAL;
int		  locker_index = -1;
bool		  use_prev     = false;

void player_get_forward(player_t c, GLfloat v[3])
{
	v[0] = sinf(c.pitch) * cosf(c.yaw);
	v[1] = sinf(c.yaw);
	v[2] = cosf(c.pitch) * cosf(c.yaw);
}

void player_get_forward_interp(player_t c, GLfloat v[3], float t)
{
	float pitch = lerpf(c.pitch_last, c.pitch, t);
	float yaw   = lerpf(c.yaw_last, c.yaw, t);

	v[0] = sinf(pitch) * cosf(yaw);
	v[1] = sinf(yaw);
	v[2] = cosf(pitch) * cosf(yaw);
}

static void _player_get_side(player_t c, GLfloat forw[3], GLfloat v[3])
{
	vector_cross(forw, c.up, v);
}

void player_get_focus(player_t c, GLfloat v[3])
{
	GLfloat forw[3];
	player_get_forward(c, forw);
	vector_add(c.eye, forw, v);
}

static void _player_apply_friction(player_t *c)
{
	float speed	= vector_magnitude(c->velocity);
	float control	= speed < STOP_SPEED ? STOP_SPEED : speed;
	float drop	= control * FRICTION * DELTA_TIME;
	float new_speed = speed - drop;
	if (new_speed < 0.01f)
		new_speed = 0;

	if (speed > 0)
		new_speed /= speed;

	vector_scale(c->velocity, new_speed);
}

static void _player_accelerate(player_t *c, float wish_dir[3])
{
	float cur_speed = vector_dot(c->velocity, wish_dir);
	float add_speed = MAX_SPEED - cur_speed;
	if (add_speed < 0)
		return;

	float accel_speed = ACCEL * DELTA_TIME * MAX_SPEED;
	if (accel_speed > add_speed)
		accel_speed = add_speed;

	for (int i = 0; i < 3; i++)
		c->velocity[i] += accel_speed * wish_dir[i];
}

bool player_is_grounded(player_t c)
{
	return c.eye[1] <= PLAYER_HEIGHT;
}

static void _player_update_headbob(player_t *c)
{
	const float speed = vector_magnitude(c->velocity);
	float	    focusbob[3];
	float	    upbob[3];

	GLfloat focus[3];
	player_get_focus(*c, focus);
	vector_copy(c->eyebob, c->eyebob_last);
	vector_copy(c->eye, c->eyebob);
	vector_copy(focus, focusbob);
	vector_copy(c->up, upbob);

	/* Moving headbob */
	float forw[3], side[3], up[3];

	c->bob_timer += speed;
	vector_copy(c->up, up);
	player_get_forward(*c, forw);
	_player_get_side(*c, forw, side);

	static bool played_footstep = false;
	float	    sin_bob	    = sinf(c->bob_timer);

	mixer_ch_set_vol(SFXC_FOOTSTEP, speed * 2, speed * 2);
	if (fabsf(sin_bob) >= 0.8f) {
		if (!played_footstep) {
			wav64_play(&footstep_sfx, 1);
			played_footstep = true;
		}
	} else {
		played_footstep = false;
	}

	float sin2_bob = sinf(c->bob_timer * 2);
	float bob[3];

	vector_scale(side, sin_bob * speed * HEADBOB_SCALE);
	vector_scale(up, sin2_bob * speed * HEADBOB_SCALE * 0.5f);
	vector_add(side, up, bob);
	vector_add(c->eyebob, bob, c->eyebob);
	vector_add(focusbob, bob, focusbob);
}

static void _player_update_look(player_t *c, const joypad_inputs_t held)
{
	c->pitch_tar += (held.btn.c_left - held.btn.c_right) * TURN_SPEED;
	c->yaw_tar += (held.btn.c_up - held.btn.c_down) * TURN_SPEED;
	c->yaw_tar = clampf(c->yaw_tar, -(PI_HALF - 0.1f), PI_HALF - 0.1f);

	c->pitch_last = c->pitch;
	c->yaw_last   = c->yaw;
	c->pitch      = lerpf(c->pitch, c->pitch_tar, 0.35f);
	c->yaw	      = lerpf(c->yaw, c->yaw_tar, 0.35f);

	if (fabsf(c->pitch_tar - c->pitch) < 0.01f)
		c->pitch = c->pitch_tar;

	if (fabsf(c->yaw_tar - c->yaw) < 0.01f)
		c->yaw = c->yaw_tar;
}

static enum player_state _player_update_normal(player_t		    *c,
					       const joypad_inputs_t held,
					       const joypad_inputs_t down,
					       const int16_t	     stick[2],
					       const int current_room)
{
	_player_update_look(c, held);

	GLfloat forw[3];
	GLfloat side[3];

	player_get_forward(*c, forw);
	_player_get_side(*c, forw, side);
	forw[1] = side[1] = 0;

	float smove = (float)stick[0] / 85.0f;
	float fmove = (float)stick[1] / 85.0f;

	/* apply control stick deadzone */
	fmove = (fabsf(fmove) < 0.1) ? 0 : fmove;
	smove = (fabsf(smove) < 0.1) ? 0 : smove;

	float wish_dir[3];
	for (int i = 0; i < 3; i++)
		wish_dir[i] = fmove * forw[i] + smove * side[i];
	vector_normalize(wish_dir);

	_player_update_headbob(c);
	_player_apply_friction(c);
	_player_accelerate(c, wish_dir);
	c->eye[1] += PLAYER_HEIGHT - c->eye[1];

	vector_add(c->eye, c->velocity, c->eye);

	/* Check wall collision */
	float glo_room_offs[2][3];
	room_get_global_pos(current_room, glo_room_offs[0], false);
	room_get_global_pos(current_room - 1, glo_room_offs[1], false);
	for (unsigned int k = 0; k < 1 + (current_room - 1 >= 0); k++) {
		unsigned int num_tris =
				rooms[current_room - k].collis.num_verts / 3;
		for (unsigned int i = 0; i < num_tris; i++) {
			vertex_t *verts = rooms[current_room - k].collis.verts +
					  (i * 3);
			float p1[3], p2[3], p3[3];
			vector_add(verts[0].pos, glo_room_offs[k], p1);
			vector_add(verts[1].pos, glo_room_offs[k], p2);
			vector_add(verts[2].pos, glo_room_offs[k], p3);
			float a[3];
			float b[3];
			float n[3];
			vector_sub(p2, p1, a);
			vector_sub(p3, p1, b);
			vector_cross(a, b, n);
			vector_normalize(n);

			float *tri[3] = { p1, p2, p3 };
			float  dist   = 0.0f;
			float  raydir[3];
			vector_invert(n, raydir);
			if (!raycast_triangle(c->eye, raydir, tri, &dist))
				continue;

			float push = clampf(1.1f - dist, 0, 69);
			vector_scale(n, push);
			vector_add(c->eye, n, c->eye);
		}
	}

	/* Search for lockers */
	float shortest_dist	  = 999.0f;
	int   shortest_ind	  = 999;
	int   locker_base_inds[2] = {
		  rooms[current_room].index,
		  rooms[(int)fmax(current_room - 1, 0)].index,
	};

	int which_set = 0;
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < lockers_cnt[locker_base_inds[j]]; i++) {
			locker l = lockers[locker_base_inds[j]][i];
			float  glo_locker_pos[3];
			room_get_global_pos(current_room - j,
					    glo_locker_pos,
					    false);
			l.pos[1] = 0;
			vector_add(glo_locker_pos, l.pos, glo_locker_pos);
			float pl_planar_pos[3];
			vector_copy(c->eye, pl_planar_pos);
			pl_planar_pos[1] = 0;
			float pdist	 = vector_distance(pl_planar_pos,
							   glo_locker_pos);

			if (pdist >= shortest_dist)
				continue;

			shortest_dist = pdist;
			shortest_ind  = i;
			which_set     = j;
		}
	}

	if (shortest_dist < 2 && (down.btn.b || down.btn.a)) {
		locker_index = shortest_ind;
		use_prev     = which_set;
		return PS_IN_LOCKER;
	}

	return PS_NORMAL;
}

static enum player_state _player_update_locker(player_t		    *c,
					       const joypad_inputs_t held,
					       const joypad_inputs_t down,
					       const int current_room)
{
	float glo_locker_pos[3];
	int   room_ind = current_room - (use_prev * (current_room > 0));
	room_get_global_pos(room_ind, glo_locker_pos, false);
	locker l	    = lockers[rooms[room_ind].index][locker_index];
	float  locker_angle = l.pos[1];
	l.pos[1]	    = 0;
	vector_add(l.pos, glo_locker_pos, glo_locker_pos);
	vector_copy(glo_locker_pos, c->eye);
	float forw[3] = {
		sinf(locker_angle * PI_HALF - PI_HALF) * 0.2f,
		0,
		cosf(locker_angle * PI_HALF - PI_HALF) * 0.2f,
	};
	vector_add(c->eye, forw, c->eye);
	vector_add(c->eye, (float[3]) { 0, 1.24f, 0 }, c->eye);
	vector_copy(c->eyebob, c->eyebob_last);
	vector_copy(c->eye, c->eyebob);
	_player_update_look(c, held);
	c->pitch_tar = (locker_angle * PI_HALF) - PI_HALF;
	c->pitch     = c->pitch_tar;
	c->yaw_tar   = 0;
	vector_copy((float[3]) { 0, 0, 0 }, c->velocity);

	if (!down.btn.b && !down.btn.a)
		return PS_IN_LOCKER;

	/* Exit the locker */
	vector_scale(forw, 7.5f);
	vector_add(c->eye, forw, c->eye);
	return PS_NORMAL;
}

void player_update(player_t		*c,
		   const joypad_inputs_t held,
		   const joypad_inputs_t down,
		   const int16_t	 stick[2],
		   const int		 current_room,
		   const float		 ent_dist,
		   const bool		 ent_active)
{
	static int frame     = 0;
	c->shake_last	     = c->shake;
	float ent_d	     = 1.0f / (ent_dist * ent_dist);
	float ent_dist_clamp = clampf(ent_d - 0.01f, 0, 1);
	c->shake = sinf(frame * 1.35f) * ent_dist_clamp * 4 * ent_active;
	frame++;

	float lvol = clampf(0.08f + ent_d, 0, 1);
	mixer_ch_set_vol(SFXC_ENTITY_LOW, lvol, lvol);
	float hvol = clampf((lvol - 0.08f) * 32, 0, 1);
	mixer_ch_set_vol(SFXC_ENTITY_HIGH, hvol, hvol);

	vector_copy(c->eye, c->eye_last);
	switch (pstate) {
	case PS_NORMAL:
		pstate = _player_update_normal(c,
					       held,
					       down,
					       stick,
					       current_room);
		return;

	case PS_IN_LOCKER:
		pstate = _player_update_locker(c, held, down, current_room);
		return;

	default:
		return;
	}
}

void player_setup_view_mat(player_t c, float subtick)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float forw[3];
	player_get_forward_interp(c, forw, subtick);

	float eyebob_intp[3];
	vector_lerp(c.eyebob_last, c.eyebob, subtick, eyebob_intp);

	float focusbob[3];
	vector_add(eyebob_intp, forw, focusbob);

	float side[3];
	_player_get_side(c, forw, side);
	float shake_lerp = lerpf(c.shake_last, c.shake, subtick);
	vector_scale(side, shake_lerp);

	float up[3] = { 0, 1, 0 };
	vector_add(up, side, up);
	vector_normalize(up);

	gluLookAt(eyebob_intp[0],
		  eyebob_intp[1],
		  eyebob_intp[2],
		  focusbob[0],
		  focusbob[1],
		  focusbob[2],
		  up[0],
		  up[1],
		  up[2]);
}
