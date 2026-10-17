#ifndef IT_SONG_H
#define IT_SONG_H

#include "common.h"

typedef struct {
    uint8_t note;        /* 0: none, 1..120: C-0..B-9, 254: cut, 255: off, 253: fade */
    uint8_t instrument;  /* 0: none, 1..99 */
    uint8_t volume;      /* 0: none, 1..65: v0..v64, 128..192: p0..p64, effects */
    uint8_t effect;      /* 0: none, 'A'..'Z', '\' */
    uint8_t param;       /* 0x00..0xFF */
} ITCell;

typedef struct {
    uint16_t rows;
    ITCell* data;        /* rows * IT_MAX_CHANNELS */
} ITPattern;

typedef struct {
    char filename[13];
    char name[26];
    uint8_t flags;
    uint8_t default_vol; /* 0..64 */
    uint8_t global_vol;  /* 0..64 */
    uint8_t default_pan; /* 0..64, bit 7 set = enabled */
    uint32_t length;     /* in sample frames */
    uint32_t loop_start;
    uint32_t loop_end;
    uint32_t c5_speed;   /* Hz */
    uint32_t susloop_start;
    uint32_t susloop_end;
    uint8_t vibrato_speed;
    uint8_t vibrato_depth;
    uint8_t vibrato_rate;
    uint8_t vibrato_type;
    float* data;         /* mono or left channel, -1.0f .. +1.0f */
    float* data_r;       /* right channel if stereo, else NULL */
} ITSample;

typedef struct {
    int8_t y;
    uint16_t tick;
} ITEnvelopeNode;

typedef struct {
    uint8_t flags;       /* ENV_FLAG_ON, ENV_FLAG_LOOP, ENV_FLAG_SUSTAIN, ENV_FLAG_FILTER */
    uint8_t num_nodes;   /* 0..25 */
    uint8_t loop_start;
    uint8_t loop_end;
    uint8_t susloop_start;
    uint8_t susloop_end;
    ITEnvelopeNode nodes[25];
} ITEnvelope;

typedef struct {
    char filename[13];
    char name[26];
    uint8_t nna;
    uint8_t dct;
    uint8_t dca;
    uint16_t fadeout;    /* 0..128, count 1024 */
    int8_t pitch_pan_separation; /* -32..+32 */
    uint8_t pitch_pan_center;     /* 0..119 */
    uint8_t global_vol;  /* 0..128 */
    uint8_t default_pan; /* 0..64, bit 7 set = disabled */
    uint8_t random_vol_variation;
    uint8_t random_pan_variation;
    uint8_t filter_cutoff;       /* 0..127 */
    uint8_t filter_resonance;    /* 0..127 */
    uint8_t midi_channel;
    uint8_t midi_program;
    uint16_t midi_bank;
    uint8_t keyboard_note[120];
    uint8_t keyboard_sample[120];
    ITEnvelope vol_env;
    ITEnvelope pan_env;
    ITEnvelope pitch_env;
} ITInstrument;

typedef struct {
    char name[26];
    uint16_t hilight_rows;       /* Low byte: primary (default 4), High byte: secondary (default 16) */
    uint16_t order_count;
    uint16_t instrument_count;
    uint16_t sample_count;
    uint16_t pattern_count;
    uint16_t cwt;
    uint16_t cmwt;
    uint16_t flags;
    uint16_t special;
    uint8_t global_vol;          /* 0..128 */
    uint8_t mix_vol;             /* 0..128 */
    uint8_t initial_speed;       /* default 6 */
    uint8_t initial_tempo;       /* default 125 */
    uint8_t panning_separation;  /* 0..128, default 128 */
    uint8_t pitch_wheel_depth;
    char* message;
    uint8_t channel_pan[IT_MAX_CHANNELS];
    uint8_t channel_vol[IT_MAX_CHANNELS];
    bool channel_mute[IT_MAX_CHANNELS];
    uint8_t orders[IT_MAX_ORDERS];
    ITPattern patterns[IT_MAX_PATTERNS];
    ITSample samples[IT_MAX_SAMPLES];
    ITInstrument instruments[IT_MAX_INSTRUMENTS];
} ITSong;

ITSong* it_song_create(void);
void it_song_destroy(ITSong* song);
void it_song_clear(ITSong* song);
void it_song_init_default(ITSong* song);

/* Helper getters and setters */
ITCell* it_pattern_get_cell(ITPattern* pat, int row, int ch);
void it_pattern_alloc(ITPattern* pat, int rows);
void it_pattern_free(ITPattern* pat);
void it_sample_free_data(ITSample* smp);
void it_sample_init_synth(ITSample* smp, int type, const char* name);

const char* it_note_to_string(uint8_t note);
int it_string_to_note(const char* str);

#endif /* IT_SONG_H */
