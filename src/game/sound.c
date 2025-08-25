#include <libdragon.h>

#include "game/sound.h"

static const char *sound_paths[SFX_COUNT] = {
        "rom:/sfx_footstep.wav64",
        "rom:/sfx_door_open.wav64"
};

static wav64_t sound_samples[SFX_COUNT];

void sound_init(void)
{
        int i;

        audio_init(AUDIO_SAMPLERATE, AUDIO_BUFFER_COUNT);
        mixer_init(MIXER_CH_COUNT);
        wav64_init_compression(COMPRESS_LEVEL);

        for (i = 0; i < MIXER_CH_COUNT; ++i)
                mixer_ch_set_limits(i, 16, AUDIO_SAMPLERATE, 0);

        for (i = 0; i < SFX_COUNT; ++i)
                wav64_open(sound_samples + i, sound_paths[i]);
}

void sound_play(const uint8_t sfx, const uint8_t ch, const float volume)
{
        mixer_ch_set_vol(ch, volume, volume);
        wav64_play(sound_samples + sfx, ch);
}

void sound_poll(void)
{
        if (!audio_can_write())
                return;

        mixer_poll(audio_write_begin(), audio_get_buffer_length());
        audio_write_end();
}

void sound_terminate(void)
{
        int i;

        for (i = 0; i < SFX_COUNT; ++i)
                wav64_close(sound_samples + i);

        mixer_close();
        audio_close();
}
