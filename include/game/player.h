#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#define PLAYER_HEIGHT 1.24f

#include <GL/gl.h>
#include <libdragon.h>

typedef struct {
	GLfloat eye[3];
	GLfloat eye_last[3];
	GLfloat pitch, yaw, pitch_last, yaw_last;
	GLfloat pitch_tar, yaw_tar;
	GLfloat up[3];
	GLfloat velocity[3];
	GLfloat bob_timer;
	GLfloat eyebob[3];
	GLfloat eyebob_last[3];
	GLfloat shake, shake_last;
} player_t;

enum player_state { PS_NORMAL, PS_IN_LOCKER, PS_COUNT };

extern enum player_state pstate;
extern int		 locker_index;

void player_get_forward(player_t c, GLfloat v[3]);
void player_get_forward_interp(player_t c, GLfloat v[3], float t);
void player_update(player_t		 *c,
		   const joypad_inputs_t held,
		   const joypad_inputs_t down,
		   const int16_t	  stick[2],
		   const int		  current_room,
		   const float		  ent_dist,
		   const bool		  ent_active);
void player_setup_view_mat(player_t c, float subtick);
void player_get_focus(player_t c, GLfloat v[3]);
bool player_is_grounded(player_t c);

#endif /* GAME_PLAYER_H */
