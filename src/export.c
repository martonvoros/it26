#include "export.h"
#include "mixer.h"
#include "util.h"
#include "player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* --- WAV Export Helpers --- */

static void write_u16_le(FILE* f, uint16_t v) {
    uint8_t b[2] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF) };
    fwrite(b, 1, 2, f);
}

static void write_u32_le(FILE* f, uint32_t v) {
    uint8_t b[4] = {
        (uint8_t)(v & 0xFF),
        (uint8_t)((v >> 8) & 0xFF),
        (uint8_t)((v >> 16) & 0xFF),
        (uint8_t)((v >> 24) & 0xFF)
    };
    fwrite(b, 1, 4, f);
}

static void write_u32_be(FILE* f, uint32_t v) {
    uint8_t b[4] = {
        (uint8_t)((v >> 24) & 0xFF),
        (uint8_t)((v >> 16) & 0xFF),
        (uint8_t)((v >> 8) & 0xFF),
        (uint8_t)(v & 0xFF)
    };
    fwrite(b, 1, 4, f);
}

static void write_u16_be(FILE* f, uint16_t v) {
    uint8_t b[2] = { (uint8_t)((v >> 8) & 0xFF), (uint8_t)(v & 0xFF) };
    fwrite(b, 1, 2, f);
}

static void write_vlq(FILE* f, uint32_t val) {
    uint8_t buf[5];
    int len = 0;
    buf[len++] = val & 0x7F;
    while ((val >>= 7) > 0) {
        buf[len++] = (val & 0x7F) | 0x80;
    }
    for (int i = len - 1; i >= 0; i--) {
        fputc(buf[i], f);
    }
}

bool it_export_wav_loop(const ITSong* song, int pattern_idx, const char* filepath) {
    if (!song || !filepath) return false;
    if (pattern_idx < 0 || pattern_idx >= IT_MAX_PATTERNS) pattern_idx = 0;
    const ITPattern* pat = &song->patterns[pattern_idx];
    if (!pat->data || pat->rows == 0) return false;

    FILE* f = fopen(filepath, "wb");
    if (!f) return false;

    /* Write WAV Header */
    fwrite("RIFF", 1, 4, f);
    write_u32_le(f, 0); /* placeholder for file size - 8 */
    fwrite("WAVEfmt ", 1, 8, f);
    write_u32_le(f, 16);     /* PCM format chunk length */
    write_u16_le(f, 1);      /* Audio format: 1 (PCM) */
    write_u16_le(f, 2);      /* Channels: 2 (Stereo) */
    write_u32_le(f, 44100);  /* Sample rate */
    write_u32_le(f, 44100 * 2 * 2); /* Byte rate */
    write_u16_le(f, 4);      /* Block align */
    write_u16_le(f, 16);     /* Bits per sample */
    fwrite("data", 1, 4, f);
    write_u32_le(f, 0);      /* placeholder for data length */

    /* Setup player for offline rendering */
    ITSong* prev_song = it_player_get_song();
    it_player_init((ITSong*)song, 44100);
    it_player_play_pattern(pattern_idx, 0);

    float render_buf[1024 * 2];
    int16_t pcm_buf[1024 * 2];
    uint32_t total_bytes = 0;

    int last_row = -1;
    int rows_completed = 0;

    while (rows_completed < pat->rows) {
        int cur_row = it_player_get_row();
        if (cur_row != last_row) {
            if (last_row >= 0) rows_completed++;
            last_row = cur_row;
            if (rows_completed >= pat->rows) break;
        }

        uint32_t chunk = 512;
        it_player_advance(chunk);
        it_mixer_render((ITSong*)song, render_buf, chunk);

        for (uint32_t i = 0; i < chunk * 2; i++) {
            float s = render_buf[i];
            int val = (int)(s * 32767.0f);
            if (val > 32767) val = 32767;
            if (val < -32768) val = -32768;
            pcm_buf[i] = (int16_t)val;
        }
        fwrite(pcm_buf, sizeof(int16_t), chunk * 2, f);
        total_bytes += chunk * 2 * sizeof(int16_t);
    }

    /* Stop player and restore */
    it_player_stop();
    if (prev_song) it_player_set_song(prev_song);

    /* Patch header lengths */
    fseek(f, 4, SEEK_SET);
    write_u32_le(f, total_bytes + 36);
    fseek(f, 40, SEEK_SET);
    write_u32_le(f, total_bytes);

    fclose(f);
    return true;
}

bool it_export_wav_song(const ITSong* song, const char* filepath) {
    if (!song || !filepath) return false;

    FILE* f = fopen(filepath, "wb");
    if (!f) return false;

    fwrite("RIFF", 1, 4, f);
    write_u32_le(f, 0);
    fwrite("WAVEfmt ", 1, 8, f);
    write_u32_le(f, 16);
    write_u16_le(f, 1);
    write_u16_le(f, 2);
    write_u32_le(f, 44100);
    write_u32_le(f, 44100 * 2 * 2);
    write_u16_le(f, 4);
    write_u16_le(f, 16);
    fwrite("data", 1, 4, f);
    write_u32_le(f, 0);

    ITSong* prev_song = it_player_get_song();
    it_player_init((ITSong*)song, 44100);
    it_player_play_song();

    float render_buf[1024 * 2];
    int16_t pcm_buf[1024 * 2];
    uint32_t total_bytes = 0;

    int order_limit = song->order_count > 0 ? song->order_count : 1;
    int last_order = 0;

    while (it_player_is_playing()) {
        int cur_order = it_player_get_order();
        if (cur_order < last_order || cur_order >= order_limit) {
            /* Song looped back or finished order list */
            break;
        }
        last_order = cur_order;

        uint32_t chunk = 512;
        it_player_advance(chunk);
        it_mixer_render((ITSong*)song, render_buf, chunk);

        for (uint32_t i = 0; i < chunk * 2; i++) {
            float s = render_buf[i];
            int val = (int)(s * 32767.0f);
            if (val > 32767) val = 32767;
            if (val < -32768) val = -32768;
            pcm_buf[i] = (int16_t)val;
        }
        fwrite(pcm_buf, sizeof(int16_t), chunk * 2, f);
        total_bytes += chunk * 2 * sizeof(int16_t);
    }

    it_player_stop();
    if (prev_song) it_player_set_song(prev_song);

    fseek(f, 4, SEEK_SET);
    write_u32_le(f, total_bytes + 36);
    fseek(f, 40, SEEK_SET);
    write_u32_le(f, total_bytes);

    fclose(f);
    return true;
}

/* --- MIDI Export Implementation --- */

typedef struct {
    uint8_t pitch;
    uint8_t vel;
    uint32_t on_tick;
    bool active;
} ChannelMidiState;

static void export_midi_track(FILE* f, const ITSong* song, int start_order, int num_orders, int pat_single, int ch) {
    long trk_pos = ftell(f);
    fwrite("MTrk", 1, 4, f);
    write_u32_be(f, 0); /* length placeholder */

    long data_start = ftell(f);

    /* Track Name Meta Event */
    write_vlq(f, 0);
    fputc(0xFF, f);
    fputc(0x03, f);
    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "Channel %02d", ch + 1);
    uint8_t nlen = (uint8_t)strlen(name_buf);
    fputc(nlen, f);
    fwrite(name_buf, 1, nlen, f);

    uint8_t midi_ch = (uint8_t)(ch % 16);
    /* Channel 10 (index 9) in MIDI is GM Drums */
    if (ch >= 0 && ch <= 3) midi_ch = 9;

    uint32_t current_tick = 0;
    uint32_t last_event_tick = 0;
    ChannelMidiState state = {0};

    int ticks_per_row = 24; /* 96 PPQN / 4 rows per beat */

    int o_count = (pat_single >= 0) ? 1 : num_orders;
    for (int o = 0; o < o_count; o++) {
        int pat_idx = (pat_single >= 0) ? pat_single : song->orders[o];
        if (pat_idx >= IT_MAX_PATTERNS || pat_idx == IT_ORDER_END) break;
        if (pat_idx == IT_ORDER_SKIP) continue;

        const ITPattern* pat = &song->patterns[pat_idx];
        if (!pat->data || pat->rows == 0) continue;

        for (int r = 0; r < pat->rows; r++) {
            const ITCell* c = it_pattern_get_cell((ITPattern*)pat, r, ch);
            if (c) {
                if (c->note == IT_NOTE_CUT || c->note == IT_NOTE_OFF || c->note == IT_NOTE_FADE) {
                    if (state.active) {
                        uint32_t delta = current_tick - last_event_tick;
                        write_vlq(f, delta);
                        fputc(0x80 | midi_ch, f);
                        fputc(state.pitch, f);
                        fputc(0x40, f);
                        last_event_tick = current_tick;
                        state.active = false;
                    }
                } else if (c->note >= 1 && c->note <= 120) {
                    if (state.active) {
                        uint32_t delta = current_tick - last_event_tick;
                        write_vlq(f, delta);
                        fputc(0x80 | midi_ch, f);
                        fputc(state.pitch, f);
                        fputc(0x40, f);
                        last_event_tick = current_tick;
                        state.active = false;
                    }
                    int p = c->note + 11; /* Convert IT note 61 (C-5) to MIDI 72 */
                    if (p < 0) p = 0;
                    if (p > 127) p = 127;
                    uint8_t vel = (c->volume > 0 && c->volume <= 65) ? (uint8_t)((c->volume - 1) * 2) : 100;
                    if (vel > 127) vel = 127;

                    uint32_t delta = current_tick - last_event_tick;
                    write_vlq(f, delta);
                    fputc(0x90 | midi_ch, f);
                    fputc((uint8_t)p, f);
                    fputc(vel, f);
                    last_event_tick = current_tick;

                    state.pitch = (uint8_t)p;
                    state.vel = vel;
                    state.on_tick = current_tick;
                    state.active = true;
                }
            }
            current_tick += ticks_per_row;
        }
    }

    /* Turn off hanging note */
    if (state.active) {
        uint32_t delta = current_tick - last_event_tick;
        write_vlq(f, delta);
        fputc(0x80 | midi_ch, f);
        fputc(state.pitch, f);
        fputc(0x40, f);
        last_event_tick = current_tick;
    }

    /* End of Track Meta Event */
    write_vlq(f, 0);
    fputc(0xFF, f);
    fputc(0x2F, f);
    fputc(0x00, f);

    long data_end = ftell(f);
    uint32_t trk_len = (uint32_t)(data_end - data_start);
    fseek(f, trk_pos + 4, SEEK_SET);
    write_u32_be(f, trk_len);
    fseek(f, data_end, SEEK_SET);
}

static bool export_midi_internal(const ITSong* song, int pat_single, const char* filepath) {
    if (!song || !filepath) return false;

    FILE* f = fopen(filepath, "wb");
    if (!f) return false;

    int active_channels = 16; /* Export 16 tracks */
    uint16_t num_tracks = (uint16_t)(1 + active_channels);

    /* Write MThd Header */
    fwrite("MThd", 1, 4, f);
    write_u32_be(f, 6);
    write_u16_be(f, 1);          /* Format 1 */
    write_u16_be(f, num_tracks);
    write_u16_be(f, 96);         /* 96 PPQN */

    /* Track 0: Tempo and Time Signature */
    long trk0_pos = ftell(f);
    fwrite("MTrk", 1, 4, f);
    write_u32_be(f, 0);
    long trk0_start = ftell(f);

    /* Time Signature: 4/4 */
    write_vlq(f, 0);
    fputc(0xFF, f);
    fputc(0x58, f);
    fputc(0x04, f);
    fputc(0x04, f); /* numerator 4 */
    fputc(0x02, f); /* denominator 2^2 = 4 */
    fputc(0x18, f); /* 24 MIDI clocks per metronome tick */
    fputc(0x08, f); /* 8 32nd notes per 24 clocks */

    /* Set Tempo */
    int tempo = song->initial_tempo ? song->initial_tempo : 125;
    uint32_t us_per_beat = (uint32_t)(60000000 / tempo);
    write_vlq(f, 0);
    fputc(0xFF, f);
    fputc(0x51, f);
    fputc(0x03, f);
    fputc((us_per_beat >> 16) & 0xFF, f);
    fputc((us_per_beat >> 8) & 0xFF, f);
    fputc(us_per_beat & 0xFF, f);

    /* End of Track */
    write_vlq(f, 0);
    fputc(0xFF, f);
    fputc(0x2F, f);
    fputc(0x00, f);

    long trk0_end = ftell(f);
    uint32_t trk0_len = (uint32_t)(trk0_end - trk0_start);
    fseek(f, trk0_pos + 4, SEEK_SET);
    write_u32_be(f, trk0_len);
    fseek(f, trk0_end, SEEK_SET);

    /* Tracks 1..16 */
    for (int ch = 0; ch < active_channels; ch++) {
        export_midi_track(f, song, 0, song->order_count, pat_single, ch);
    }

    fclose(f);
    return true;
}

bool it_export_midi_loop(const ITSong* song, int pattern_idx, const char* filepath) {
    if (!song || !filepath) return false;
    if (pattern_idx < 0 || pattern_idx >= IT_MAX_PATTERNS) pattern_idx = 0;
    return export_midi_internal(song, pattern_idx, filepath);
}

bool it_export_midi_song(const ITSong* song, const char* filepath) {
    if (!song || !filepath) return false;
    return export_midi_internal(song, -1, filepath);
}
