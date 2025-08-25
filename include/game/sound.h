#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include <stdint.h>

enum {
        MIXER_CH_PLAYER,
        MIXER_CH_DOOR,
        MIXER_CH_COUNT
};

enum {
        SFX_FOOTSTEP,
        SFX_DOOR_OPEN,
        SFX_COUNT
};

void sound_init(void);
void sound_play(const uint8_t sfx, const uint8_t ch, const float volume);
void sound_poll(void);
void sound_terminate(void);

#endif /* GAME_SOUND_H */
