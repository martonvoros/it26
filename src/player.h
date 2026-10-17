#ifndef IT_PLAYER_H
#define IT_PLAYER_H

#include "song.h"
#include "mixer.h"

typedef struct {
    uint8_t current_note;
    uint8_t current_inst;
    uint8_t current_vol;
    uint8_t current_pan;
    uint8_t last_note;
    uint8_t last_inst;
    uint8_t last_vol;
    uint8_t last_effect;
    uint8_t last_param;

    /* Slide memory */
    uint8_t vol_slide_mem;
    uint8_t pitch_slide_mem;
    uint8_t portamento_mem;
    float   portamento_target_freq;
    float   portamento_speed;

    /* Vibrato memory */
    uint8_t vib_speed;
    uint8_t vib_depth;
    uint8_t vib_pos;
    uint8_t vib_type;

    /* Tremolo memory */
    uint8_t tremolo_speed;
    uint8_t tremolo_depth;
    uint8_t tremolo_pos;

    /* Tremor */
    uint8_t tremor_ontime;
    uint8_t tremor_offtime;
    uint8_t tremor_count;
    bool    tremor_active;

    /* Retrig */
    uint8_t retrig_count;

    /* Note delay / cut */
    uint8_t note_delay_tick;
    ITCell  delayed_cell;
    uint8_t note_cut_tick;

    bool muted;
    bool solo;
} ITHostChannel;

typedef struct {
    ITSong* song;
    bool playing;
    bool paused;
    bool pattern_loop;

    int current_order;
    int current_pattern;
    int current_row;
    int current_tick;

    int tempo;
    int speed;
    int global_volume;
    int mix_volume;

    int break_row;
    int next_order;
    bool pattern_break;
    bool order_jump;

    uint32_t sample_rate;
    uint32_t samples_per_tick;
    uint32_t tick_sample_accumulator;

    ITHostChannel channels[IT_MAX_CHANNELS];
} ITPlayer;

void it_player_init(ITSong* song, uint32_t sample_rate);
void it_player_set_song(ITSong* song);
ITSong* it_player_get_song(void);

void it_player_play_song(void);
void it_player_play_from_order(int order);
void it_player_play_pattern(int pattern, int row);
void it_player_play_row(int pattern, int row);
void it_player_stop(void);
void it_player_pause(void);
bool it_player_is_playing(void);
bool it_player_is_paused(void);

void it_player_advance(uint32_t num_frames);
void it_player_trigger_note(int host_channel, int note, int inst, int vol);
void it_player_key_off(int host_channel);

int  it_player_get_order(void);
int  it_player_get_row(void);
int  it_player_get_tick(void);
int  it_player_get_pattern(void);
int  it_player_get_tempo(void);
int  it_player_get_speed(void);
int  it_player_get_global_volume(void);

void it_player_set_channel_mute(int ch, bool mute);
bool it_player_get_channel_mute(int ch);
void it_player_toggle_channel_solo(int ch);

ITPlayer* it_player_get_instance(void);

#endif /* IT_PLAYER_H */
