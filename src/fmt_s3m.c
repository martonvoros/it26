#include "fmt_s3m.h"
#include "util.h"

bool it_load_s3m(ITSong* song, const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return false;

    fseek(f, 0x2C, SEEK_SET);
    uint32_t magic = it_read_u32le(f);
    if (magic != 0x4D524353) { /* "SCRM" */
        fclose(f);
        return false;
    }

    fseek(f, 0, SEEK_SET);
    it_song_clear(song);

    it_read_bytes(f, song->name, 26);
    song->name[25] = '\0';
    fseek(f, 0x20, SEEK_SET);

    song->order_count = it_read_u16le(f);
    song->sample_count = it_read_u16le(f);
    song->pattern_count = it_read_u16le(f);
    song->flags = it_read_u16le(f);
    song->cwt = it_read_u16le(f);
    song->cmwt = it_read_u16le(f);
    it_read_u32le(f); /* SCRM magic */

    song->global_vol = it_read_u8(f) * 2; /* S3M 0..64 -> IT 0..128 */
    song->initial_speed = it_read_u8(f);
    song->initial_tempo = it_read_u8(f);
    song->mix_vol = (it_read_u8(f) & 0x7F);
    song->panning_separation = 128;

    fseek(f, 0x40, SEEK_SET);
    uint8_t ch_settings[32];
    it_read_bytes(f, ch_settings, 32);
    for (int i = 0; i < 32; i++) {
        if (ch_settings[i] == 0xFF) {
            song->channel_mute[i] = true;
        } else {
            song->channel_mute[i] = false;
            song->channel_vol[i] = 64;
            /* S3M left channels 0..7, right 8..15 */
            song->channel_pan[i] = (ch_settings[i] < 8) ? 16 : 48;
        }
    }
    for (int i = 32; i < 64; i++) {
        song->channel_mute[i] = true;
    }

    /* Orders */
    uint16_t actual_orders = IT_MIN(song->order_count, IT_MAX_ORDERS);
    it_read_bytes(f, song->orders, actual_orders);
    for (int i = actual_orders; i < IT_MAX_ORDERS; i++) song->orders[i] = IT_ORDER_END;

    /* Parapointers */
    uint16_t smp_ptrs[IT_MAX_SAMPLES];
    uint16_t pat_ptrs[IT_MAX_PATTERNS];

    for (int i = 0; i < song->sample_count && i < IT_MAX_SAMPLES; i++) {
        smp_ptrs[i] = it_read_u16le(f);
    }
    for (int i = 0; i < song->pattern_count && i < IT_MAX_PATTERNS; i++) {
        pat_ptrs[i] = it_read_u16le(f);
    }

    /* Load Samples */
    for (int i = 0; i < song->sample_count && i < IT_MAX_SAMPLES; i++) {
        if (smp_ptrs[i] == 0) continue;
        fseek(f, (long)smp_ptrs[i] * 16, SEEK_SET);
        ITSample* smp = &song->samples[i + 1];

        uint8_t type = it_read_u8(f);
        if (type != 1) continue; /* 1 = Sample */

        it_read_bytes(f, smp->filename, 12);
        smp->filename[12] = '\0';

        uint8_t ptr_hi = it_read_u8(f);
        uint16_t ptr_lo = it_read_u16le(f);
        uint32_t data_offset = ((uint32_t)ptr_hi << 16) | ptr_lo;

        smp->length = it_read_u32le(f);
        smp->loop_start = it_read_u32le(f);
        smp->loop_end = it_read_u32le(f);
        smp->default_vol = it_read_u8(f);
        it_read_u8(f); /* dsk */
        it_read_u8(f); /* pack */
        uint8_t flags = it_read_u8(f);
        smp->c5_speed = it_read_u32le(f);

        fseek(f, (long)smp_ptrs[i] * 16 + 0x30, SEEK_SET);
        it_read_bytes(f, smp->name, 26);
        smp->name[25] = '\0';

        smp->flags = SMP_FLAG_EXISTS;
        if (flags & 1) smp->flags |= SMP_FLAG_LOOP;
        smp->global_vol = 64;

        if (smp->length > 0 && data_offset > 0) {
            fseek(f, (long)data_offset * 16, SEEK_SET);
            smp->data = (float*)malloc(smp->length * sizeof(float));
            if (smp->data) {
                for (uint32_t s = 0; s < smp->length; s++) {
                    uint8_t b = it_read_u8(f);
                    smp->data[s] = (float)((int8_t)(b - 128)) / 128.0f; /* S3M is unsigned 8-bit */
                }
            }
        }
    }

    /* Load Patterns */
    for (int p = 0; p < song->pattern_count && p < IT_MAX_PATTERNS; p++) {
        it_pattern_alloc(&song->patterns[p], 64);
        if (pat_ptrs[p] == 0) continue;

        fseek(f, (long)pat_ptrs[p] * 16, SEEK_SET);
        uint16_t pat_len = it_read_u16le(f);
        if (pat_len <= 2) continue;

        uint8_t* pdata = (uint8_t*)malloc(pat_len);
        if (!pdata) continue;
        it_read_bytes(f, pdata, pat_len - 2);

        size_t offset = 0;
        int row = 0;
        while (row < 64 && offset < (size_t)(pat_len - 2)) {
            uint8_t b = it_mem_u8(pdata, &offset, pat_len - 2);
            if (b == 0) {
                row++;
                continue;
            }
            int ch = b & 31;
            ITCell* cell = it_pattern_get_cell(&song->patterns[p], row, ch);

            if (b & 32) { /* Note and Instrument present */
                uint8_t note_raw = it_mem_u8(pdata, &offset, pat_len - 2);
                uint8_t inst_raw = it_mem_u8(pdata, &offset, pat_len - 2);
                if (note_raw == 254) cell->note = IT_NOTE_CUT;
                else if (note_raw == 255) cell->note = IT_NOTE_OFF;
                else {
                    int oct = note_raw >> 4;
                    int sem = note_raw & 0x0F;
                    cell->note = 1 + oct * 12 + sem;
                }
                cell->instrument = inst_raw;
            }
            if (b & 64) { /* Volume present */
                uint8_t vol_raw = it_mem_u8(pdata, &offset, pat_len - 2);
                cell->volume = (vol_raw <= 64) ? (vol_raw + 1) : 65;
            }
            if (b & 128) { /* Effect and Param present */
                cell->effect = it_mem_u8(pdata, &offset, pat_len - 2);
                cell->param = it_mem_u8(pdata, &offset, pat_len - 2);
            }
        }
        free(pdata);
    }

    fclose(f);
    return true;
}
