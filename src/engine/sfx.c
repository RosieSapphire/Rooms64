#include "engine/sfx.h"

wav64_t door_open_sfx, footstep_sfx, a60_low, a60_high;

void sfx_load(void)
{
	mixer_ch_set_vol(SFXC_DOOR, 0.2f, 0.2f);
	wav64_open(&door_open_sfx, "rom:/door_open.wav64");
	wav64_open(&footstep_sfx, "rom:/footstep.wav64");

	mixer_ch_set_vol(SFXC_ENTITY_LOW, 0.2f, 0.2f);
	wav64_open(&a60_low, "rom:/a60_low.wav64");
	wav64_set_loop(&a60_low, true);
	mixer_ch_set_vol(SFXC_ENTITY_HIGH, 0.4f, 0.4f);
	wav64_open(&a60_high, "rom:/a60_high.wav64");
	wav64_set_loop(&a60_high, true);
}
