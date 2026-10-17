#include "player.h"
#include "util.h"
#include <math.h>

static ITPlayer s_player;

void it_player_init(ITSong* song, uint32_t sample_rate) {
    memset(&s_player, 0, sizeof(ITPlayer));
    s_player.song = song;
    s_player.sample_rate = sample_rate ? sample_rate : 48000;
    it_player_stop();
}

void it_player_set_song(ITSong* song) {
    s_player.song = song;
    it_player_stop();
}

ITSong* it_player_get_song(void) {
    return s_player.song;
}

ITPlayer* it_player_get_instance(void) {
    return &s_player;
}

void it_player_stop(void) {
    s_player.playing = false;
    s_player.paused = false;
    s_player.current_row = 0;
    s_player.current_tick = 0;
    s_player.tick_sample_accumulator = 0;
    it_mixer_reset_voices();
}

void it_player_pause(void) {
    s_player.paused = !s_player.paused;
}

bool it_player_is_playing(void) {
    return s_player.playing;
}

bool it_player_is_paused(void) {
    return s_player.paused;
}

int it_player_get_order(void) {
    return s_player.current_order;
}

int it_player_get_row(void) {
    return s_player.current_row;
}

int it_player_get_tick(void) {
    return s_player.current_tick;
}

int it_player_get_pattern(void) {
    return s_player.current_pattern;
}

int it_player_get_tempo(void) {
    return s_player.tempo;
}

int it_player_get_speed(void) {
    return s_player.speed;
}

int it_player_get_global_volume(void) {
    return s_player.global_volume;
}

void it_player_set_channel_mute(int ch, bool mute) {
    if (ch >= 0 && ch < IT_MAX_CHANNELS && s_player.song) {
        s_player.song->channel_mute[ch] = mute;
    }
}

bool it_player_get_channel_mute(int ch) {
    if (ch >= 0 && ch < IT_MAX_CHANNELS && s_player.song) {
        return s_player.song->channel_mute[ch];
    }
    return false;
}

void it_player_toggle_channel_solo(int ch) {
    if (ch < 0 || ch >= IT_MAX_CHANNELS || !s_player.song) return;
    bool is_already_solo = !s_player.song->channel_mute[ch];
    for (int c = 0; c < IT_MAX_CHANNELS; c++) {
        if (c != ch && !s_player.song->channel_mute[c]) {
            is_already_solo = false;
            break;
        }
    }
    if (is_already_solo) {
        /* Unsolo: unmute all 64 channels */
        for (int c = 0; c < IT_MAX_CHANNELS; c++) s_player.song->channel_mute[c] = false;
    } else {
        /* Solo channel ch */
        for (int c = 0; c < IT_MAX_CHANNELS; c++) s_player.song->channel_mute[c] = (c != ch);
    }
}

static void update_samples_per_tick(void) {
    if (s_player.tempo < 32) s_player.tempo = 32;
    /* In tracker timing: samples_per_tick = sample_rate * 2.5 / tempo */
    s_player.samples_per_tick = (uint32_t)((double)s_player.sample_rate * 2.5 / (double)s_player.tempo);
    if (s_player.samples_per_tick == 0) s_player.samples_per_tick = 1;
}

void it_player_play_song(void) {
    if (!s_player.song) return;
    it_mixer_reset_voices();
    s_player.current_order = 0;
    s_player.current_pattern = s_player.song->orders[0];
    s_player.current_row = 0;
    s_player.current_tick = 0;
    s_player.pattern_loop = false;
    s_player.speed = s_player.song->initial_speed ? s_player.song->initial_speed : 6;
    s_player.tempo = s_player.song->initial_tempo ? s_player.song->initial_tempo : 125;
    s_player.global_volume = s_player.song->global_vol;
    s_player.mix_volume = s_player.song->mix_vol;
    update_samples_per_tick();
    s_player.tick_sample_accumulator = 0;
    s_player.playing = true;
    s_player.paused = false;
}

void it_player_play_from_order(int order) {
    if (!s_player.song) return;
    if (order < 0) order = 0;
    if (order >= IT_MAX_ORDERS) order = IT_MAX_ORDERS - 1;
    it_player_play_song();
    s_player.current_order = order;
    s_player.current_pattern = s_player.song->orders[order];
}

void it_player_play_pattern(int pattern, int row) {
    if (!s_player.song) return;
    it_mixer_reset_voices();
    s_player.current_order = 0;
    s_player.current_pattern = pattern;
    s_player.current_row = row;
    s_player.current_tick = 0;
    s_player.pattern_loop = true;
    s_player.speed = s_player.song->initial_speed ? s_player.song->initial_speed : 6;
    s_player.tempo = s_player.song->initial_tempo ? s_player.song->initial_tempo : 125;
    update_samples_per_tick();
    s_player.tick_sample_accumulator = 0;
    s_player.playing = true;
    s_player.paused = false;
}

void it_player_play_row(int pattern, int row) {
    it_player_play_pattern(pattern, row);
}

void it_player_trigger_note(int host_channel, int note, int inst_idx, int vol) {
    if (!s_player.song || host_channel < 0 || host_channel >= IT_MAX_CHANNELS) return;
    ITSong* song = s_player.song;
    ITHostChannel* hc = &s_player.channels[host_channel];

    if (note == IT_NOTE_CUT) {
        it_mixer_stop_channel(host_channel);
        return;
    }
    if (note == IT_NOTE_OFF || note == IT_NOTE_FADE) {
        it_player_key_off(host_channel);
        return;
    }
    if (note < 1 || note > 120) return;

    if (inst_idx > 0) hc->current_inst = (uint8_t)inst_idx;
    int actual_inst = hc->current_inst;
    int actual_smp = actual_inst;
    int play_note = note;

    /* Instrument mode mapping */
    int nna = NNA_CUT;
    uint16_t fadeout = 0;
    float init_cutoff = 127.0f, init_reso = 0.0f;

    if ((song->flags & SONG_FLAG_INSTRUMENTS) && actual_inst > 0 && actual_inst < IT_MAX_INSTRUMENTS) {
        ITInstrument* inst = &song->instruments[actual_inst];
        actual_smp = inst->keyboard_sample[note - 1];
        play_note = inst->keyboard_note[note - 1] + 1;
        nna = inst->nna;
        fadeout = inst->fadeout;
        if (inst->filter_cutoff > 0) init_cutoff = (float)inst->filter_cutoff;
        if (inst->filter_resonance > 0) init_reso = (float)inst->filter_resonance;
    }

    if (actual_smp < 1 || actual_smp >= IT_MAX_SAMPLES) return;
    ITSample* smp = &song->samples[actual_smp];
    if (!smp->data || smp->length == 0) return;

    /* Handle NNA on existing active voices for this channel */
    for (int i = 0; i < IT_MAX_VIRTUAL_CHANNELS; i++) {
        ITVoice* v = it_mixer_get_voice(i);
        if (v && v->active && v->host_channel == host_channel) {
            v->background = true;
            if (nna == NNA_CUT) {
                v->active = false;
            } else if (nna == NNA_NOTE_OFF) {
                v->note_off = true;
            } else if (nna == NNA_NOTE_FADE) {
                v->note_fade = true;
            }
        }
    }

    ITVoice* nv = it_mixer_alloc_voice(host_channel, nna);
    if (!nv) return;

    nv->active = true;
    nv->background = false;
    nv->host_channel = (uint8_t)host_channel;
    nv->note = (uint8_t)play_note;
    nv->sample_num = (uint8_t)actual_smp;
    nv->inst_num = (uint8_t)actual_inst;
    nv->sample_pos = 0.0;
    nv->fadeout_count = 1024;
    nv->fadeout_rate = fadeout;
    nv->note_off = false;
    nv->note_fade = false;

    float freq = it_note_to_freq(play_note, smp->c5_speed);
    nv->base_freq = freq;
    nv->current_freq = freq;
    nv->sample_step = (double)freq / (double)s_player.sample_rate;

    /* Volume */
    float base_vol = (vol >= 0) ? (float)vol / 64.0f : ((smp->default_vol > 0) ? (float)smp->default_vol / 64.0f : 1.0f);
    float ch_vol = (float)song->channel_vol[host_channel] / 64.0f;
    float smp_gvol = (float)smp->global_vol / 64.0f;
    nv->vol_final = base_vol * ch_vol * smp_gvol;

    /* Pan */
    int pan_val = song->channel_pan[host_channel];
    if (smp->default_pan & 0x80) pan_val = smp->default_pan & 0x7F;
    nv->pan_final = ((float)pan_val - 32.0f) / 32.0f; /* -1.0 .. +1.0 */

    /* Filter */
    nv->filter_enabled = (init_cutoff < 127.0f);
    nv->filter_cutoff = init_cutoff;
    nv->filter_resonance = init_reso;
    nv->flt_low[0] = nv->flt_low[1] = 0.0f;
    nv->flt_band[0] = nv->flt_band[1] = 0.0f;

    /* Envelope setup */
    nv->vol_env_tick = 0;
    nv->pan_env_tick = 0;
    nv->pitch_env_tick = 0;
}

void it_player_key_off(int host_channel) {
    for (int i = 0; i < IT_MAX_VIRTUAL_CHANNELS; i++) {
        ITVoice* v = it_mixer_get_voice(i);
        if (v && v->active && v->host_channel == host_channel) {
            v->note_off = true;
            v->note_fade = true;
        }
    }
}

static void process_row(void) {
    if (!s_player.song) return;
    int pat_idx = s_player.current_pattern;
    if (pat_idx < 0 || pat_idx >= IT_MAX_PATTERNS) return;
    ITPattern* pat = &s_player.song->patterns[pat_idx];
    if (!pat->data || s_player.current_row >= pat->rows) return;

    for (int ch = 0; ch < IT_MAX_CHANNELS; ch++) {
        const ITCell* cell = it_pattern_get_cell(pat, s_player.current_row, ch);
        if (!cell) continue;
        ITHostChannel* hc = &s_player.channels[ch];

        /* Effect processing on tick 0 */
        if (cell->effect == 'A' && cell->param > 0) {
            s_player.speed = cell->param;
        } else if (cell->effect == 'T' && cell->param >= 32) {
            s_player.tempo = cell->param;
            update_samples_per_tick();
        } else if (cell->effect == 'B') {
            s_player.next_order = cell->param;
            s_player.order_jump = true;
        } else if (cell->effect == 'C') {
            s_player.break_row = cell->param;
            s_player.pattern_break = true;
        } else if (cell->effect == 'V') {
            s_player.global_volume = cell->param;
            s_player.song->global_vol = cell->param;
        } else if (cell->effect == 'M') {
            s_player.song->channel_vol[ch] = (cell->param <= 64) ? cell->param : 64;
        } else if (cell->effect == 'X') {
            s_player.song->channel_pan[ch] = cell->param / 2;
        }

        /* Note trigger */
        if (cell->note > 0) {
            int vol = (cell->volume > 0 && cell->volume <= 65) ? (cell->volume - 1) : -1;
            it_player_trigger_note(ch, cell->note, cell->instrument, vol);
        }
    }
}

static void process_tick(void) {
    /* Update envelopes, fades, and effects for all active voices */
    for (int i = 0; i < IT_MAX_VIRTUAL_CHANNELS; i++) {
        ITVoice* v = it_mixer_get_voice(i);
        if (!v || !v->active) continue;

        if (v->note_fade && v->fadeout_rate > 0) {
            v->fadeout_count -= (int32_t)v->fadeout_rate;
            if (v->fadeout_count <= 0) {
                v->active = false;
            }
        }
    }
}

void it_player_advance(uint32_t num_frames) {
    if (!s_player.playing || s_player.paused || !s_player.song) return;

    s_player.tick_sample_accumulator += num_frames;

    while (s_player.tick_sample_accumulator >= s_player.samples_per_tick) {
        s_player.tick_sample_accumulator -= s_player.samples_per_tick;

        if (s_player.current_tick == 0) {
            process_row();
        } else {
            process_tick();
        }

        s_player.current_tick++;
        if (s_player.current_tick >= s_player.speed) {
            s_player.current_tick = 0;
            s_player.current_row++;

            int pat_idx = s_player.current_pattern;
            int num_rows = (pat_idx >= 0 && pat_idx < IT_MAX_PATTERNS && s_player.song->patterns[pat_idx].rows > 0)
                           ? s_player.song->patterns[pat_idx].rows : 64;

            if (s_player.pattern_break || s_player.order_jump || s_player.current_row >= num_rows) {
                if (s_player.pattern_loop) {
                    s_player.current_row = 0;
                } else {
                    if (s_player.order_jump) {
                        s_player.current_order = s_player.next_order;
                        s_player.order_jump = false;
                    } else {
                        s_player.current_order++;
                    }

                    if (s_player.current_order >= IT_MAX_ORDERS || s_player.song->orders[s_player.current_order] == IT_ORDER_END) {
                        s_player.current_order = 0; /* Loop back to beginning */
                    }
                    while (s_player.song->orders[s_player.current_order] == IT_ORDER_SKIP) {
                        s_player.current_order++;
                        if (s_player.current_order >= IT_MAX_ORDERS) s_player.current_order = 0;
                    }

                    s_player.current_pattern = s_player.song->orders[s_player.current_order];
                    s_player.current_row = s_player.pattern_break ? s_player.break_row : 0;
                    s_player.pattern_break = false;
                    s_player.break_row = 0;
                }
            }
        }
    }
}
