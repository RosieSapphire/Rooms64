#ifndef GAME_ENTITIY_H
#define GAME_ENTITIY_H

typedef struct {
	int cur_point, num_rooms_crossed;
	float progress, progress_last;
	bool was_active, is_active;
} entity_t;

void entities_init(void);
void entity_get_pos(const entity_t *e, float vec[3], float subtick);
void entity_update(entity_t *e, int current_room);
void entity_draw(const entity_t *e, float subtick, float player_pos[3]);
void entity_trigger(entity_t *e, int current_room);

#endif /* GAME_ENTITIY_H */
