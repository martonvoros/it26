#include "fmt_mod.h"
#include "util.h"

static const uint16_t s_mod_period_table[60] = {
    1712,1616,1525,1440,1357,1281,1209,1141,1077,1017, 961, 907, /* Oct 0 */
     856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453, /* Oct 1 */
     428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226, /* Oct 2 */
     214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113, /* Oct 3 */
     107, 101,  95,  90,  85,  80,  76,  71,  67,  64,  60,  57  /* Oct 4 */
};

static uint8_t mod_period_to_note(uint16_t period) {
    if (period == 0) return IT_NOTE_NONE;
    int best_diff = 999999;
    int best_note = 0;
    for (int i = 0; i < 60; i++) {
        int diff = abs((int)period - (int)s_mod_period_table[i]);
        if (diff < best_diff) {
            best_diff = diff;
            best_note = i + 1 + 24; /* Shift to align C-3 as middle C */
        }
    }
    return (uint8_t)best_note;
}

bool it_load_mod(ITSong* song, const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return false;

    fseek(f, 1080, SEEK_SET);
    char tag[5] = {0};
    fread(tag, 1, 4, f);

    int num_channels = 4;
    if (memcmp(tag, "M.K.", 4) == 0 || memcmp(tag, "M!K!", 4) == 0 || memcmp(tag, "4CHN", 4) == 0 || memcmp(tag, "FLT4", 4) == 0) {
        num_channels = 4;
    } else if (memcmp(tag, "6CHN", 4) == 0) {
        num_channels = 6;
    } else if (memcmp(tag, "8CHN", 4) == 0 || memcmp(tag, "OCTA", 4) == 0 || memcmp(tag, "CD81", 4) == 0) {
        num_channels = 8;
    } else if (memcmp(tag, "2CHN", 4) == 0) {
        num_channels = 2;
    } else {
        /* Not a recognized standard MOD tag */
        fclose(f);
        return false;
    }

    it_song_clear(song);
    fseek(f, 0, SEEK_SET);

    it_read_bytes(f, song->name, 20);
    song->name[20] = '\0';

    song->sample_count = 31;
    song->initial_speed = 6;
    song->initial_tempo = 125;
    song->global_vol = 128;
    song->mix_vol = 48;
    song->panning_separation = 128;
    song->flags = SONG_FLAG_STEREO;

    for (int i = 0; i < 64; i++) {
        if (i < num_channels) {
            song->channel_mute[i] = false;
            song->channel_vol[i] = 64;
            /* Classic Amiga L-R-R-L panning */
            song->channel_pan[i] = (i % 4 == 0 || i % 4 == 3) ? 16 : 48;
        } else {
            song->channel_mute[i] = true;
        }
    }

    uint32_t smp_lengths[32] = {0};
    uint32_t smp_loop_starts[32] = {0};
    uint32_t smp_loop_lens[32] = {0};

    for (int i = 1; i <= 31; i++) {
        ITSample* smp = &song->samples[i];
        it_read_bytes(f, smp->name, 22);
        smp->name[22] = '\0';

        uint16_t len_words = (it_read_u8(f) << 8) | it_read_u8(f);
        smp_lengths[i] = (uint32_t)len_words * 2;
        smp->length = smp_lengths[i];

        uint8_t finetune = it_read_u8(f) & 0x0F;
        (void)finetune;
        smp->default_vol = it_read_u8(f);
        smp->global_vol = 64;
        smp->c5_speed = 8363 * 4;

        uint16_t loop_start_words = (it_read_u8(f) << 8) | it_read_u8(f);
        uint16_t loop_len_words = (it_read_u8(f) << 8) | it_read_u8(f);
        smp_loop_starts[i] = (uint32_t)loop_start_words * 2;
        smp_loop_lens[i] = (uint32_t)loop_len_words * 2;

        smp->loop_start = smp_loop_starts[i];
        smp->loop_end = smp->loop_start + smp_loop_lens[i];
        if (smp_loop_lens[i] > 2 && smp->loop_end <= smp->length) {
            smp->flags = SMP_FLAG_EXISTS | SMP_FLAG_LOOP;
        } else if (smp->length > 0) {
            smp->flags = SMP_FLAG_EXISTS;
        }
    }

    song->order_count = it_read_u8(f);
    it_read_u8(f); /* restart pos */

    uint8_t order_table[128];
    it_read_bytes(f, order_table, 128);

    uint8_t max_pat = 0;
    for (int i = 0; i < song->order_count && i < 128; i++) {
        song->orders[i] = order_table[i];
        if (order_table[i] > max_pat) max_pat = order_table[i];
    }
    for (int i = song->order_count; i < IT_MAX_ORDERS; i++) {
        song->orders[i] = IT_ORDER_END;
    }
    song->pattern_count = max_pat + 1;

    fseek(f, 1084, SEEK_SET);

    /* Read patterns */
    for (int p = 0; p < song->pattern_count && p < IT_MAX_PATTERNS; p++) {
        it_pattern_alloc(&song->patterns[p], 64);
        for (int r = 0; r < 64; r++) {
            for (int ch = 0; ch < num_channels; ch++) {
                uint8_t b0 = it_read_u8(f);
                uint8_t b1 = it_read_u8(f);
                uint8_t b2 = it_read_u8(f);
                uint8_t b3 = it_read_u8(f);

                uint8_t sample_num = (b0 & 0xF0) | (b2 >> 4);
                uint16_t period = ((b0 & 0x0F) << 8) | b1;
                uint8_t eff = b2 & 0x0F;
                uint8_t param = b3;

                ITCell* cell = it_pattern_get_cell(&song->patterns[p], r, ch);
                if (period > 0) {
                    cell->note = mod_period_to_note(period);
                }
                cell->instrument = sample_num;
                if (eff || param) {
                    cell->effect = eff;
                    cell->param = param;
                }
            }
        }
    }

    /* Read sample audio data (8-bit signed PCM) */
    for (int i = 1; i <= 31; i++) {
        ITSample* smp = &song->samples[i];
        if (smp->length > 0) {
            smp->data = (float*)malloc(smp->length * sizeof(float));
            if (smp->data) {
                int8_t* raw = (int8_t*)malloc(smp->length);
                if (raw) {
                    it_read_bytes(f, raw, smp->length);
                    for (uint32_t s = 0; s < smp->length; s++) {
                        smp->data[s] = (float)raw[s] / 128.0f;
                    }
                    free(raw);
                }
            }
        }
    }

    fclose(f);
    return true;
}
