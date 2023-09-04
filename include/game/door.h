#ifndef GAME_DOOR_H
#define GAME_DOOR_H

void door_init(void);
void doors_update_open_anim(int current_room);
void door_draw(int i, float subtick);

#endif /* GAME_DOOR_H */
