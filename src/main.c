#include <stdlib.h>
#include <GL/gl.h>
#include <GL/gl_integration.h>

#include "engine/vector.h"
#include "engine/debug.h"
#include "engine/sfx.h"
#include "engine/config.h"

#include "game/player.h"
#include "game/room.h"
#include "game/door.h"
#include "game/door_numplate.h"
#include "game/locker.h"
#include "game/entity.h"

#define WIN_WIDTH 320
#define WIN_HEIGHT 240
#define WIN_ASPECT ((float)WIN_WIDTH / (float)WIN_HEIGHT)
#define NEAR_PLANE 0.1f
#define FAR_PLANE 24.0f

static void _n64_init(void)
{
	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3,
			GAMMA_NONE, ANTIALIAS_RESAMPLE);
	dfs_init(DFS_DEFAULT_LOCATION);
	rdpq_init();
	gl_init();
	textures_init();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	controller_init();
	audio_init(32000, 4);
	mixer_init(SFXC_COUNT);
	sfx_load();

	/*
	debug_load();
	debug_init_isviewer();
	debug_init_usblog();
	rdpq_debug_start();
	*/

	srand(TICKS_READ());
}

int main(void)
{
	_n64_init();

	int last_room = 0;
	int current_room = 0;

	door_init();
	door_numplate_init();
	lockers_init();
	rooms_init();

	player_t cam = {0};
	cam.eye[0] = cam.eye_last[0] = 0;
	cam.eye[1] = cam.eye_last[1] = 1.24f;
	cam.eye[2] = cam.eye_last[2] = 1;
	cam.up[0] = cam.up[2] = 0;
	cam.eye[1] = 1.24f;
	cam.up[1] = 1;
	static float cam_debug_speed = 0.0f;
	static bool cam_debug_is_grounded = true;
	debug_add("Player Pos", cam.eye, DV_VECTOR3);
	debug_add("Player Angle", &cam.pitch, DV_VECTOR2);
	debug_add("Player Speed", &cam_debug_speed, DV_FLOAT);
	debug_add("Player Is Grounded", &cam_debug_is_grounded, DV_BOOL);
	debug_add("Room Current", &current_room, DV_INT);

	surface_t dep = surface_alloc(FMT_RGBA16, 320, 240);

	entities_init();
	entity_t a60 = {0};

	/* Setup projection */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-NEAR_PLANE * WIN_ASPECT, NEAR_PLANE * WIN_ASPECT,
			-NEAR_PLANE, NEAR_PLANE,
			NEAR_PLANE, FAR_PLANE);

	long ticks_accum = 0;
	long ticks_last = TICKS_READ();
	while(1) {
		/*
		assertf(current_room < 100,
				"Congratulations, you got through 100 rooms.\n");
				*/

		long ticks_now = get_ticks();
		long tick_delta = TICKS_DISTANCE(ticks_last, ticks_now);
		ticks_last = ticks_now;

		ticks_accum += tick_delta;
		while(ticks_accum >= DELTATICKS) {
			controller_scan();
			struct controller_data held = get_keys_held();
			struct controller_data down = get_keys_down();

			static bool trigd = false;
			bool room_changed = current_room != last_room;
			bool cur_room_has_lockers =
				lockers_cnt[rooms[current_room].index] > 0;
			bool prev_room_has_lockers =
				lockers_cnt[rooms[current_room - 1].index] > 0;
			if(room_changed && !trigd &&
					(cur_room_has_lockers ||
					 prev_room_has_lockers)) {
				int ran = rand() % 5;
				if(ran == 0) {
					//debugf("Triggered\n");
					entity_trigger(&a60, current_room);
				}
				//debugf("Ran: %d\n", ran);

				trigd = true;
			}

			if((a60.was_active && !a60.is_active) ||
					(current_room != last_room))
				trigd = false;

			entity_update(&a60, current_room);

			float a60_pos[3];
			entity_get_pos(&a60, a60_pos, 1);
			float a60_dir[3];
			vector_sub(a60_pos,
					(float[3]){cam.eye[0], 0, cam.eye[2]},
					a60_dir);

			player_update(&cam, held, down, current_room,
					vector_magnitude(a60_dir),
					a60.is_active);
			doors_update_open_anim(current_room);

			last_room = current_room;
			if(room_check_load_next(cam.eye, current_room)) {
				current_room++;
				wav64_play(&door_open_sfx, SFXC_DOOR);
			}

			ticks_accum -= DELTATICKS;
		}

		// long t1 = get_ticks();
		surface_t *col = display_get();
		rdpq_attach(col, &dep);
		gl_context_begin();
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float subtick = (float)ticks_accum / DELTATICKS;

		player_setup_view_mat(cam, subtick);

		int start = current_room - 3;
		if(start < 0)
			start = 0;

		float door_pos_global[3] = {0};
		vector_add(door_pos_global,
				rooms[0].rel_door_pos, door_pos_global);
		float room_pos_global_last[3] = {0};
		float room_pos_global[3] = {0};
		for(int i = 0; i < current_room + 1; i++) {
			vector_copy(room_pos_global, room_pos_global_last);

			/* Only draw rooms within 3 */
			if(i < start) {
				vector_add(door_pos_global,
						rooms[i + 1].rel_door_pos,
						door_pos_global);
				
				vector_add(room_pos_global,
						rooms[i].rel_door_pos,
						room_pos_global);
				continue;
			}

			glPushMatrix();
			glTranslatef(door_pos_global[0],
					door_pos_global[1],
					door_pos_global[2]);

			door_numplate_draw(current_room, i);
			door_draw(i, subtick);
			glPopMatrix();

			room_draw(rooms[i], room_pos_global);
			if(current_room == i)
				lockers_draw(current_room, room_pos_global,
						pstate == PS_IN_LOCKER);

			if(current_room - 1 == i)
				lockers_draw(current_room - 1,
						room_pos_global_last,
						pstate == PS_IN_LOCKER);

			vector_add(door_pos_global,
					rooms[i + 1].rel_door_pos,
					door_pos_global);
			
			vector_add(room_pos_global,
					rooms[i].rel_door_pos, room_pos_global);
		}

		float forw[3];
		player_get_forward_interp(cam, forw, subtick);
		entity_draw(&a60, subtick, forw);

		gl_context_end();

		rdpq_detach_show();
		/*
		long t2 = get_ticks();
		debugf("%f\n", (float)TICKS_DISTANCE(t1, t2)
				/ (float)TICKS_PER_SECOND);
				*/

		if(!audio_can_write())
			continue;

		short *audio_buf = audio_write_begin();
		mixer_poll(audio_buf, audio_get_buffer_length());
		audio_write_end();
		
	}

	return 0;
}
