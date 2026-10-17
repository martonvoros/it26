#include "song.h"

static const char* s_note_names[12] = {
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

ITSong* it_song_create(void) {
    ITSong* song = (ITSong*)calloc(1, sizeof(ITSong));
    if (!song) return NULL;
    it_song_init_default(song);
    return song;
}

void it_song_destroy(ITSong* song) {
    if (!song) return;
    it_song_clear(song);
    free(song);
}

void it_song_clear(ITSong* song) {
    if (!song) return;
    if (song->message) {
        free(song->message);
        song->message = NULL;
    }
    for (int p = 0; p < IT_MAX_PATTERNS; p++) {
        it_pattern_free(&song->patterns[p]);
    }
    for (int s = 0; s < IT_MAX_SAMPLES; s++) {
        it_sample_free_data(&song->samples[s]);
    }
    memset(song, 0, sizeof(ITSong));
}

void it_pattern_alloc(ITPattern* pat, int rows) {
    if (!pat) return;
    it_pattern_free(pat);
    if (rows <= 0) rows = IT_DEFAULT_ROWS;
    if (rows > IT_MAX_ROWS) rows = IT_MAX_ROWS;
    pat->rows = (uint16_t)rows;
    pat->data = (ITCell*)calloc(rows * IT_MAX_CHANNELS, sizeof(ITCell));
}

void it_pattern_free(ITPattern* pat) {
    if (!pat) return;
    if (pat->data) {
        free(pat->data);
        pat->data = NULL;
    }
    pat->rows = 0;
}

ITCell* it_pattern_get_cell(ITPattern* pat, int row, int ch) {
    if (!pat || !pat->data) return NULL;
    if (row < 0 || row >= pat->rows) return NULL;
    if (ch < 0 || ch >= IT_MAX_CHANNELS) return NULL;
    return &pat->data[row * IT_MAX_CHANNELS + ch];
}

void it_sample_free_data(ITSample* smp) {
    if (!smp) return;
    if (smp->data) {
        free(smp->data);
        smp->data = NULL;
    }
    if (smp->data_r) {
        free(smp->data_r);
        smp->data_r = NULL;
    }
    smp->length = 0;
}

void it_sample_init_synth(ITSample* smp, int type, const char* name) {
    if (!smp) return;
    it_sample_free_data(smp);
    memset(smp, 0, sizeof(ITSample));

    if (name) {
        strncpy(smp->name, name, sizeof(smp->name) - 1);
        strncpy(smp->filename, name, sizeof(smp->filename) - 1);
    }

    smp->flags = SMP_FLAG_EXISTS | SMP_FLAG_LOOP;
    smp->default_vol = 64;
    smp->global_vol = 64;
    smp->default_pan = 0x80 | 32; /* Bit 7 ON: use sample default pan (center) */
    smp->c5_speed = 8363 * 4;     /* Standard C-5 ~33452 Hz */

    int sample_rate = smp->c5_speed;
    int len = 0;

    switch (type) {
        case 0: /* Sawtooth wave */ {
            len = 1024;
            smp->length = len;
            smp->loop_start = 0;
            smp->loop_end = len;
            smp->data = (float*)malloc(len * sizeof(float));
            for (int i = 0; i < len; i++) {
                smp->data[i] = 1.0f - 2.0f * ((float)i / (float)len);
            }
            break;
        }
        case 1: /* Square wave (50% pulse) */ {
            len = 1024;
            smp->length = len;
            smp->loop_start = 0;
            smp->loop_end = len;
            smp->data = (float*)malloc(len * sizeof(float));
            for (int i = 0; i < len; i++) {
                smp->data[i] = (i < len / 2) ? 0.8f : -0.8f;
            }
            break;
        }
        case 2: /* Pure Sine wave */ {
            len = 1024;
            smp->length = len;
            smp->loop_start = 0;
            smp->loop_end = len;
            smp->data = (float*)malloc(len * sizeof(float));
            for (int i = 0; i < len; i++) {
                smp->data[i] = sinf(2.0f * (float)M_PI * (float)i / (float)len);
            }
            break;
        }
        case 3: /* Kick drum (frequency swept sine) */ {
            len = sample_rate / 2; /* 0.5s */
            smp->flags = SMP_FLAG_EXISTS; /* no loop */
            smp->length = len;
            smp->loop_start = 0;
            smp->loop_end = 0;
            smp->data = (float*)malloc(len * sizeof(float));
            float phase = 0.0f;
            for (int i = 0; i < len; i++) {
                float t = (float)i / (float)sample_rate;
                float freq = 160.0f * expf(-t * 18.0f) + 40.0f;
                phase += 2.0f * (float)M_PI * freq / (float)sample_rate;
                float amp = expf(-t * 9.0f);
                smp->data[i] = sinf(phase) * amp;
            }
            break;
        }
    }
}

void it_song_init_default(ITSong* song) {
    if (!song) return;
    it_song_clear(song);

    strncpy(song->name, "Untitled Impulse", sizeof(song->name) - 1);
    song->hilight_rows = 4 | (16 << 8);
    song->order_count = 1;
    song->instrument_count = 0;
    song->sample_count = 4;
    song->pattern_count = 1;
    song->cwt = 0x0214;  /* Impulse Tracker 2.14 */
    song->cmwt = 0x0214;
    song->flags = SONG_FLAG_STEREO | SONG_FLAG_LINEAR_SLIDES | SONG_FLAG_VOL0MIX;
    song->global_vol = 128;
    song->mix_vol = 48;
    song->initial_speed = 6;
    song->initial_tempo = 125;
    song->panning_separation = 128;
    song->pitch_wheel_depth = 2;

    /* Channel panning: alternate 16 (L), 48 (R), 48 (R), 16 (L) */
    for (int c = 0; c < IT_MAX_CHANNELS; c++) {
        song->channel_vol[c] = 64;
        song->channel_mute[c] = (c >= 8); /* First 8 channels active */
        if (c % 4 == 0) song->channel_pan[c] = 16;
        else if (c % 4 == 1) song->channel_pan[c] = 48;
        else if (c % 4 == 2) song->channel_pan[c] = 48;
        else song->channel_pan[c] = 16;
    }

    /* Set first order to pattern 0, rest 255 (end) */
    song->orders[0] = 0;
    for (int o = 1; o < IT_MAX_ORDERS; o++) {
        song->orders[o] = IT_ORDER_END;
    }

    /* Allocate pattern 0 with 64 rows */
    it_pattern_alloc(&song->patterns[0], 64);

    /* Initialize starter synth samples */
    it_sample_init_synth(&song->samples[1], 0, "Saw Lead");
    it_sample_init_synth(&song->samples[2], 1, "Pulse Bass");
    it_sample_init_synth(&song->samples[3], 2, "Sine Warm");
    it_sample_init_synth(&song->samples[4], 3, "Kick 808");
}

const char* it_note_to_string(uint8_t note) {
    static char buf[8];
    if (note == IT_NOTE_NONE) return "...";
    if (note == IT_NOTE_CUT)  return "^^^";
    if (note == IT_NOTE_OFF)  return "===";
    if (note == IT_NOTE_FADE) return "~~~";

    int n = note - 1; /* 0..119 */
    if (n < 0 || n >= 120) return "???";

    int semitone = n % 12;
    int octave = n / 12;
    snprintf(buf, sizeof(buf), "%s%d", s_note_names[semitone], octave);
    return buf;
}

int it_string_to_note(const char* str) {
    if (!str || strlen(str) < 3) return IT_NOTE_NONE;
    if (strncmp(str, "...", 3) == 0) return IT_NOTE_NONE;
    if (strncmp(str, "^^^", 3) == 0) return IT_NOTE_CUT;
    if (strncmp(str, "===", 3) == 0) return IT_NOTE_OFF;
    if (strncmp(str, "~~~", 3) == 0) return IT_NOTE_FADE;

    int semitone = -1;
    for (int i = 0; i < 12; i++) {
        if (str[0] == s_note_names[i][0] && str[1] == s_note_names[i][1]) {
            semitone = i;
            break;
        }
    }
    if (semitone < 0) return IT_NOTE_NONE;
    int octave = str[2] - '0';
    if (octave < 0 || octave > 9) return IT_NOTE_NONE;

    return 1 + octave * 12 + semitone;
}
