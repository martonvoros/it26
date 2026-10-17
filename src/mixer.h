#ifndef IT_MIXER_H
#define IT_MIXER_H

#include "song.h"

#define IT_SCOPE_BUFFER_SIZE 256

typedef struct {
    bool active;
    bool background;
    uint8_t host_channel;
    uint8_t note;
    uint8_t sample_num;
    uint8_t inst_num;

    double sample_pos;
    double sample_step;

    float vol_final;
    float pan_final;      /* -1.0 (left) .. 0.0 (center) .. +1.0 (right) */
    float base_freq;
    float current_freq;

    /* Envelopes */
    bool vol_env_active;
    uint16_t vol_env_tick;
    float vol_env_val;

    bool pan_env_active;
    uint16_t pan_env_tick;
    float pan_env_val;

    bool pitch_env_active;
    uint16_t pitch_env_tick;
    float pitch_env_val;

    /* Fadeout */
    bool note_fade;
    bool note_off;
    int32_t fadeout_count; /* Starts at 1024 */
    uint16_t fadeout_rate;

    /* Resonant Filter */
    bool filter_enabled;
    float filter_cutoff;
    float filter_resonance;
    float flt_low[2];
    float flt_band[2];

    /* Sample vibrato */
    uint8_t smp_vib_pos;
    uint8_t smp_vib_speed;
    uint8_t smp_vib_depth;
    uint8_t smp_vib_type;
} ITVoice;

typedef struct {
    float vu_level;
    float scope[IT_SCOPE_BUFFER_SIZE];
    int scope_write_pos;
} ITChannelVis;

void it_mixer_init(uint32_t sample_rate);
void it_mixer_set_sample_rate(uint32_t sample_rate);
uint32_t it_mixer_get_sample_rate(void);

void it_mixer_reset_voices(void);
ITVoice* it_mixer_alloc_voice(int host_channel, int nna);
void it_mixer_stop_voice(ITVoice* v);
void it_mixer_stop_channel(int host_channel);

void it_mixer_render(ITSong* song, float* out_interleaved, size_t num_frames);

ITVoice* it_mixer_get_voice(int index);
const ITChannelVis* it_mixer_get_channel_vis(int host_channel);
void it_mixer_get_vu_levels(float* out_l, float* out_r);

#endif /* IT_MIXER_H */
