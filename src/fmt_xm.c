#include "fmt_xm.h"
#include "util.h"

bool it_load_xm(ITSong* song, const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return false;

    char magic[18] = {0};
    fread(magic, 1, 17, f);
    if (memcmp(magic, "Extended Module: ", 17) != 0) {
        fclose(f);
        return false;
    }

    it_song_clear(song);

    it_read_bytes(f, song->name, 20);
    song->name[20] = '\0';

    fseek(f, 38, SEEK_SET);
    it_read_u8(f); /* 0x1A */
    char tracker[21] = {0};
    it_read_bytes(f, tracker, 20);
    it_read_u16le(f); /* version */

    uint32_t header_size = it_read_u32le(f);
    long header_data_start = ftell(f);

    song->order_count = it_read_u16le(f);
    it_read_u16le(f); /* restart pos */
    uint16_t num_channels = it_read_u16le(f);
    song->pattern_count = it_read_u16le(f);
    song->instrument_count = it_read_u16le(f);
    uint16_t flags = it_read_u16le(f);
    song->initial_speed = (uint8_t)it_read_u16le(f);
    song->initial_tempo = (uint8_t)it_read_u16le(f);
    song->global_vol = 128;
    song->mix_vol = 48;
    song->panning_separation = 128;
    song->flags = SONG_FLAG_STEREO;
    if (flags & 1) song->flags |= SONG_FLAG_LINEAR_SLIDES;

    for (int i = 0; i < 64; i++) {
        if (i < num_channels) {
            song->channel_mute[i] = false;
            song->channel_vol[i] = 64;
            song->channel_pan[i] = (i % 2 == 0) ? 20 : 44;
        } else {
            song->channel_mute[i] = true;
        }
    }

    uint8_t orders[256];
    it_read_bytes(f, orders, 256);
    for (int i = 0; i < song->order_count && i < IT_MAX_ORDERS; i++) {
        song->orders[i] = orders[i];
    }
    for (int i = song->order_count; i < IT_MAX_ORDERS; i++) {
        song->orders[i] = IT_ORDER_END;
    }

    fseek(f, header_data_start + header_size - 4, SEEK_SET);

    /* Patterns */
    for (int p = 0; p < song->pattern_count && p < IT_MAX_PATTERNS; p++) {
        uint32_t pat_hdr_len = it_read_u32le(f);
        uint8_t pack_type = it_read_u8(f);
        (void)pack_type;
        uint16_t num_rows = it_read_u16le(f);
        uint16_t pat_data_len = it_read_u16le(f);

        it_pattern_alloc(&song->patterns[p], num_rows);
        if (pat_hdr_len > 9) {
            fseek(f, pat_hdr_len - 9, SEEK_CUR);
        }

        if (pat_data_len == 0) continue;

        uint8_t* pdata = (uint8_t*)malloc(pat_data_len);
        if (!pdata) continue;
        it_read_bytes(f, pdata, pat_data_len);

        size_t offset = 0;
        for (int r = 0; r < num_rows; r++) {
            for (int ch = 0; ch < num_channels && ch < IT_MAX_CHANNELS; ch++) {
                if (offset >= pat_data_len) break;
                uint8_t b = pdata[offset++];
                ITCell* cell = it_pattern_get_cell(&song->patterns[p], r, ch);

                if (b & 0x80) {
                    if (b & 0x01 && offset < pat_data_len) {
                        uint8_t note = pdata[offset++];
                        cell->note = (note == 97) ? IT_NOTE_OFF : (note > 0 && note <= 96 ? note + 12 : IT_NOTE_NONE);
                    }
                    if (b & 0x02 && offset < pat_data_len) cell->instrument = pdata[offset++];
                    if (b & 0x04 && offset < pat_data_len) {
                        uint8_t vol = pdata[offset++];
                        if (vol >= 0x10 && vol <= 0x50) cell->volume = (vol - 0x10) + 1;
                    }
                    if (b & 0x08 && offset < pat_data_len) cell->effect = pdata[offset++];
                    if (b & 0x10 && offset < pat_data_len) cell->param = pdata[offset++];
                } else {
                    cell->note = (b == 97) ? IT_NOTE_OFF : (b > 0 && b <= 96 ? b + 12 : IT_NOTE_NONE);
                    if (offset < pat_data_len) cell->instrument = pdata[offset++];
                    if (offset < pat_data_len) {
                        uint8_t vol = pdata[offset++];
                        if (vol >= 0x10 && vol <= 0x50) cell->volume = (vol - 0x10) + 1;
                    }
                    if (offset < pat_data_len) cell->effect = pdata[offset++];
                    if (offset < pat_data_len) cell->param = pdata[offset++];
                }
            }
        }
        free(pdata);
    }

    /* Instruments and Samples */
    int current_sample_slot = 1;
    for (int i = 0; i < song->instrument_count && i < IT_MAX_INSTRUMENTS; i++) {
        long inst_start = ftell(f);
        uint32_t inst_size = it_read_u32le(f);
        ITInstrument* inst = &song->instruments[i + 1];

        it_read_bytes(f, inst->name, 22);
        inst->name[22] = '\0';
        it_read_u8(f); /* type */
        uint16_t num_samples = it_read_u16le(f);

        if (num_samples == 0) {
            fseek(f, inst_start + inst_size, SEEK_SET);
            continue;
        }

        fseek(f, inst_start + 33, SEEK_SET);
        /* Sample headers */
        for (int s = 0; s < num_samples && current_sample_slot < IT_MAX_SAMPLES; s++) {
            ITSample* smp = &song->samples[current_sample_slot];
            smp->length = it_read_u32le(f);
            smp->loop_start = it_read_u32le(f);
            smp->loop_end = smp->loop_start + it_read_u32le(f);
            smp->default_vol = it_read_u8(f);
            it_read_u8(f); /* finetune */
            uint8_t smp_type = it_read_u8(f);
            smp->default_pan = it_read_u8(f);
            it_read_u8(f); /* relnote */
            it_read_u8(f); /* reserved */
            it_read_bytes(f, smp->name, 22);

            smp->flags = SMP_FLAG_EXISTS;
            if (smp_type & 1) smp->flags |= SMP_FLAG_LOOP;
            if (smp_type & 2) smp->flags |= SMP_FLAG_PINGPONG;
            if (smp_type & 16) smp->flags |= SMP_FLAG_16BIT;
            smp->global_vol = 64;
            smp->c5_speed = 8363 * 4;

            /* Link instrument note table to this sample */
            for (int k = 0; k < 120; k++) {
                inst->keyboard_sample[k] = (uint8_t)current_sample_slot;
                inst->keyboard_note[k] = (uint8_t)k;
            }

            current_sample_slot++;
        }

        /* Read sample data (delta PCM) */
        fseek(f, inst_start + inst_size, SEEK_SET);
        int base_smp = current_sample_slot - num_samples;
        for (int s = 0; s < num_samples && (base_smp + s) < IT_MAX_SAMPLES; s++) {
            ITSample* smp = &song->samples[base_smp + s];
            if (smp->length == 0) continue;

            smp->data = (float*)malloc(smp->length * sizeof(float));
            if (!smp->data) continue;

            bool is_16bit = (smp->flags & SMP_FLAG_16BIT) != 0;
            if (is_16bit) {
                int16_t* raw16 = (int16_t*)malloc(smp->length * sizeof(int16_t));
                if (raw16) {
                    it_read_bytes(f, raw16, smp->length * sizeof(int16_t));
                    int16_t val = 0;
                    for (uint32_t n = 0; n < smp->length; n++) {
                        val += raw16[n];
                        smp->data[n] = (float)val / 32768.0f;
                    }
                    free(raw16);
                }
            } else {
                int8_t* raw8 = (int8_t*)malloc(smp->length);
                if (raw8) {
                    it_read_bytes(f, raw8, smp->length);
                    int8_t val = 0;
                    for (uint32_t n = 0; n < smp->length; n++) {
                        val += raw8[n];
                        smp->data[n] = (float)val / 128.0f;
                    }
                    free(raw8);
                }
            }
        }
    }
    song->sample_count = current_sample_slot - 1;

    fclose(f);
    return true;
}
