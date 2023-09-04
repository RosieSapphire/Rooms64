#ifndef ENGINE_SFX_H_
#define ENGINE_SFX_H_

#include <libdragon.h>

enum sfx_channels {
	SFXC_DOOR,
	SFXC_FOOTSTEP,
	SFXC_ENTITY_LOW,
	SFXC_ENTITY_HIGH,
	SFXC_COUNT,
};

extern wav64_t door_open_sfx, footstep_sfx, a60_low, a60_high;

void sfx_load(void);

#endif /* ENGINE_SFX_H_ */
