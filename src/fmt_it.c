#include "fmt_it.h"
#include "util.h"

/* IT bit-reader for compressed sample blocks */
typedef struct {
    const uint8_t* data;
    size_t size;
    size_t bit_pos;
} ITBitReader;

static uint32_t it_read_bits(ITBitReader* br, int count) {
    uint32_t val = 0;
    for (int i = 0; i < count; i++) {
        size_t byte_idx = br->bit_pos >> 3;
        int bit_idx = br->bit_pos & 7;
        if (byte_idx < br->size) {
            uint32_t bit = (br->data[byte_idx] >> bit_idx) & 1;
            val |= (bit << i);
        }
        br->bit_pos++;
    }
    return val;
}

static void it_decompress8(const uint8_t* src, size_t src_len, int8_t* dst, uint32_t len) {
    ITBitReader br = { src, src_len, 0 };
    uint32_t pos = 0;
    while (pos < len) {
        uint32_t block_len = IT_MIN(len - pos, 0x8000);
        uint32_t width = 9;
        int16_t last = 0;
        for (uint32_t i = 0; i < block_len; i++) {
            uint32_t val = it_read_bits(&br, width);
            if (width <= 6) {
                int32_t top = 1 << (width - 1);
                if (val == (uint32_t)top) {
                    val = it_read_bits(&br, 3) + 1;
                    width = (val < width) ? val : val + 1;
                    continue;
                }
            } else if (width < 9) {
                uint32_t border = (0xFF >> (9 - width)) - 4;
                if (val > border && val <= border + 8) {
                    val -= border;
                    width = (val < width) ? val : val + 1;
                    continue;
                }
            } else if (width == 9) {
                if (val & 0x100) {
                    width = (val + 1) & 0xFF;
                    continue;
                }
            } else {
                if (val & 0x100) {
                    width = 9;
                    continue;
                }
            }
            int8_t sample;
            if (width < 8) {
                uint8_t shift = 8 - width;
                sample = (int8_t)((val << shift) & 0xFF);
            } else {
                sample = (int8_t)(val & 0xFF);
            }
            last += sample;
            dst[pos++] = (int8_t)last;
        }
    }
}

bool it_load_file(ITSong* song, const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return false;

    uint32_t magic = it_read_u32le(f);
    if (magic != 0x4D504D49) { /* "IMPM" in little endian */
        fclose(f);
        return false;
    }

    it_song_clear(song);

    it_read_bytes(f, song->name, 26);
    song->name[25] = '\0';

    song->hilight_rows = it_read_u16le(f);
    song->order_count = it_read_u16le(f);
    song->instrument_count = it_read_u16le(f);
    song->sample_count = it_read_u16le(f);
    song->pattern_count = it_read_u16le(f);
    song->cwt = it_read_u16le(f);
    song->cmwt = it_read_u16le(f);
    song->flags = it_read_u16le(f);
    song->special = it_read_u16le(f);

    song->global_vol = it_read_u8(f);
    song->mix_vol = it_read_u8(f);
    song->initial_speed = it_read_u8(f);
    song->initial_tempo = it_read_u8(f);
    song->panning_separation = it_read_u8(f);
    song->pitch_wheel_depth = it_read_u8(f);

    uint16_t msg_length = it_read_u16le(f);
    uint32_t msg_offset = it_read_u32le(f);
    it_read_u32le(f); /* reserved */

    it_read_bytes(f, song->channel_pan, 64);
    it_read_bytes(f, song->channel_vol, 64);

    for (int i = 0; i < 64; i++) {
        if (song->channel_pan[i] & 128) {
            song->channel_mute[i] = true;
            song->channel_pan[i] &= 127;
        } else {
            song->channel_mute[i] = false;
        }
    }

    /* Orders */
    uint16_t actual_orders = IT_MIN(song->order_count, IT_MAX_ORDERS);
    it_read_bytes(f, song->orders, actual_orders);
    for (int i = actual_orders; i < IT_MAX_ORDERS; i++) {
        song->orders[i] = IT_ORDER_END;
    }

    /* Offset tables */
    uint32_t inst_offsets[IT_MAX_INSTRUMENTS];
    uint32_t smp_offsets[IT_MAX_SAMPLES];
    uint32_t pat_offsets[IT_MAX_PATTERNS];

    for (int i = 0; i < song->instrument_count && i < IT_MAX_INSTRUMENTS; i++) {
        inst_offsets[i] = it_read_u32le(f);
    }
    for (int i = 0; i < song->sample_count && i < IT_MAX_SAMPLES; i++) {
        smp_offsets[i] = it_read_u32le(f);
    }
    for (int i = 0; i < song->pattern_count && i < IT_MAX_PATTERNS; i++) {
        pat_offsets[i] = it_read_u32le(f);
    }

    /* Song message */
    if ((song->special & 1) && msg_offset > 0 && msg_length > 0) {
        fseek(f, msg_offset, SEEK_SET);
        song->message = (char*)malloc(msg_length + 1);
        if (song->message) {
            it_read_bytes(f, song->message, msg_length);
            song->message[msg_length] = '\0';
        }
    }

    /* Instruments */
    for (int i = 0; i < song->instrument_count && i < IT_MAX_INSTRUMENTS; i++) {
        if (inst_offsets[i] == 0) continue;
        fseek(f, inst_offsets[i], SEEK_SET);
        ITInstrument* inst = &song->instruments[i + 1];

        uint32_t magic_inst = it_read_u32le(f);
        if (magic_inst != 0x49504D49) continue; /* "IMPI" */

        it_read_bytes(f, inst->filename, 12);
        inst->filename[12] = '\0';
        it_read_u8(f); /* 00h */

        inst->nna = it_read_u8(f);
        inst->dct = it_read_u8(f);
        inst->dca = it_read_u8(f);
        inst->fadeout = it_read_u16le(f);
        inst->pitch_pan_separation = (int8_t)it_read_u8(f);
        inst->pitch_pan_center = it_read_u8(f);
        inst->global_vol = it_read_u8(f);
        inst->default_pan = it_read_u8(f);
        inst->random_vol_variation = it_read_u8(f);
        inst->random_pan_variation = it_read_u8(f);

        it_read_u16le(f); /* tracker version */
        it_read_u8(f);    /* number of samples */
        it_read_u8(f);    /* reserved */

        it_read_bytes(f, inst->name, 26);
        inst->name[25] = '\0';

        inst->filter_cutoff = it_read_u8(f);
        inst->filter_resonance = it_read_u8(f);
        inst->midi_channel = it_read_u8(f);
        inst->midi_program = it_read_u8(f);
        inst->midi_bank = it_read_u16le(f);

        /* Keyboard table (120 note-sample pairs) */
        for (int k = 0; k < 120; k++) {
            inst->keyboard_note[k] = it_read_u8(f);
            inst->keyboard_sample[k] = it_read_u8(f);
        }

        /* Read Volume, Panning, Pitch Envelopes */
        ITEnvelope* envs[3] = { &inst->vol_env, &inst->pan_env, &inst->pitch_env };
        for (int e = 0; e < 3; e++) {
            envs[e]->flags = it_read_u8(f);
            envs[e]->num_nodes = it_read_u8(f);
            envs[e]->loop_start = it_read_u8(f);
            envs[e]->loop_end = it_read_u8(f);
            envs[e]->susloop_start = it_read_u8(f);
            envs[e]->susloop_end = it_read_u8(f);
            for (int n = 0; n < 25; n++) {
                envs[e]->nodes[n].y = (int8_t)it_read_u8(f);
                envs[e]->nodes[n].tick = it_read_u16le(f);
            }
            it_read_u8(f); /* pad byte */
        }
    }

    /* Samples */
    for (int i = 0; i < song->sample_count && i < IT_MAX_SAMPLES; i++) {
        if (smp_offsets[i] == 0) continue;
        fseek(f, smp_offsets[i], SEEK_SET);
        ITSample* smp = &song->samples[i + 1];

        uint32_t magic_smp = it_read_u32le(f);
        if (magic_smp != 0x53504D49) continue; /* "IMPS" */

        it_read_bytes(f, smp->filename, 12);
        smp->filename[12] = '\0';
        it_read_u8(f); /* 00h */

        smp->global_vol = it_read_u8(f);
        smp->flags = it_read_u8(f);
        smp->default_vol = it_read_u8(f);

        it_read_bytes(f, smp->name, 26);
        smp->name[25] = '\0';

        uint8_t cvt = it_read_u8(f);
        smp->default_pan = it_read_u8(f);

        smp->length = it_read_u32le(f);
        smp->loop_start = it_read_u32le(f);
        smp->loop_end = it_read_u32le(f);
        smp->c5_speed = it_read_u32le(f);
        smp->susloop_start = it_read_u32le(f);
        smp->susloop_end = it_read_u32le(f);
        uint32_t smp_data_offset = it_read_u32le(f);

        smp->vibrato_speed = it_read_u8(f);
        smp->vibrato_depth = it_read_u8(f);
        smp->vibrato_rate = it_read_u8(f);
        smp->vibrato_type = it_read_u8(f);

        if (smp->length > 0 && smp_data_offset > 0 && (smp->flags & SMP_FLAG_EXISTS)) {
            fseek(f, smp_data_offset, SEEK_SET);
            smp->data = (float*)malloc(smp->length * sizeof(float));
            bool is_16bit = (smp->flags & SMP_FLAG_16BIT) != 0;
            bool is_signed = (cvt & 1) != 0;
            bool is_delta = (cvt & 4) != 0;
            bool is_compressed = (smp->flags & SMP_FLAG_COMPRESSED) != 0;

            if (is_compressed && !is_16bit) {
                /* IT compressed 8-bit */
                uint8_t* comp_buf = (uint8_t*)malloc(smp->length * 2);
                int8_t* raw_buf = (int8_t*)malloc(smp->length);
                if (comp_buf && raw_buf) {
                    size_t read_sz = it_read_bytes(f, comp_buf, smp->length * 2);
                    it_decompress8(comp_buf, read_sz, raw_buf, smp->length);
                    for (uint32_t s = 0; s < smp->length; s++) {
                        smp->data[s] = (float)raw_buf[s] / 128.0f;
                    }
                }
                free(comp_buf);
                free(raw_buf);
            } else if (!is_16bit) {
                uint8_t* raw = (uint8_t*)malloc(smp->length);
                if (raw) {
                    it_read_bytes(f, raw, smp->length);
                    int8_t last = 0;
                    for (uint32_t s = 0; s < smp->length; s++) {
                        int8_t v;
                        if (is_delta) {
                            last += (int8_t)raw[s];
                            v = last;
                        } else if (is_signed) {
                            v = (int8_t)raw[s];
                        } else {
                            v = (int8_t)(raw[s] - 128);
                        }
                        smp->data[s] = (float)v / 128.0f;
                    }
                    free(raw);
                }
            } else {
                /* 16-bit sample */
                int16_t* raw16 = (int16_t*)malloc(smp->length * sizeof(int16_t));
                if (raw16) {
                    it_read_bytes(f, raw16, smp->length * sizeof(int16_t));
                    int16_t last = 0;
                    for (uint32_t s = 0; s < smp->length; s++) {
                        int16_t v;
                        if (is_delta) {
                            last += raw16[s];
                            v = last;
                        } else if (is_signed) {
                            v = raw16[s];
                        } else {
                            v = (int16_t)(raw16[s] - 32768);
                        }
                        smp->data[s] = (float)v / 32768.0f;
                    }
                    free(raw16);
                }
            }
        }
    }

    /* Patterns */
    for (int p = 0; p < song->pattern_count && p < IT_MAX_PATTERNS; p++) {
        if (pat_offsets[p] == 0) {
            /* 64-row empty pattern */
            it_pattern_alloc(&song->patterns[p], 64);
            continue;
        }
        fseek(f, pat_offsets[p], SEEK_SET);
        uint16_t pat_len = it_read_u16le(f);
        uint16_t pat_rows = it_read_u16le(f);
        it_read_u32le(f); /* 4 reserved bytes */

        it_pattern_alloc(&song->patterns[p], pat_rows);

        uint8_t* pdata = (uint8_t*)malloc(pat_len);
        if (!pdata) continue;
        it_read_bytes(f, pdata, pat_len);

        size_t offset = 0;
        uint8_t last_mask[IT_MAX_CHANNELS] = {0};
        uint8_t last_note[IT_MAX_CHANNELS] = {0};
        uint8_t last_inst[IT_MAX_CHANNELS] = {0};
        uint8_t last_vol[IT_MAX_CHANNELS] = {0};
        uint8_t last_eff[IT_MAX_CHANNELS] = {0};
        uint8_t last_par[IT_MAX_CHANNELS] = {0};

        int row = 0;
        while (row < pat_rows && offset < pat_len) {
            uint8_t b = it_mem_u8(pdata, &offset, pat_len);
            if (b == 0) {
                row++;
                continue;
            }
            int ch = (b - 1) & 63;
            uint8_t mask;
            if (b & 128) {
                mask = it_mem_u8(pdata, &offset, pat_len);
                last_mask[ch] = mask;
            } else {
                mask = last_mask[ch];
            }

            ITCell* cell = it_pattern_get_cell(&song->patterns[p], row, ch);
            if (mask & 1) {
                cell->note = it_mem_u8(pdata, &offset, pat_len);
                last_note[ch] = cell->note;
            }
            if (mask & 2) {
                cell->instrument = it_mem_u8(pdata, &offset, pat_len);
                last_inst[ch] = cell->instrument;
            }
            if (mask & 4) {
                cell->volume = it_mem_u8(pdata, &offset, pat_len);
                last_vol[ch] = cell->volume;
            }
            if (mask & 8) {
                cell->effect = it_mem_u8(pdata, &offset, pat_len);
                cell->param = it_mem_u8(pdata, &offset, pat_len);
                last_eff[ch] = cell->effect;
                last_par[ch] = cell->param;
            }
            if (mask & 16)  cell->note = last_note[ch];
            if (mask & 32)  cell->instrument = last_inst[ch];
            if (mask & 64)  cell->volume = last_vol[ch];
            if (mask & 128) {
                cell->effect = last_eff[ch];
                cell->param = last_par[ch];
            }
        }
        free(pdata);
    }

    fclose(f);
    return true;
}

bool it_save_file(const ITSong* song, const char* filepath) {
    FILE* f = fopen(filepath, "wb");
    if (!f) return false;

    /* Write IMPM magic */
    it_write_u32le(f, 0x4D504D49);

    /* Song Name (26 bytes) */
    char name_buf[26] = {0};
    strncpy(name_buf, song->name, 25);
    it_write_bytes(f, name_buf, 26);

    it_write_u16le(f, song->hilight_rows);
    it_write_u16le(f, song->order_count);
    it_write_u16le(f, song->instrument_count);
    it_write_u16le(f, song->sample_count);
    it_write_u16le(f, song->pattern_count);
    it_write_u16le(f, song->cwt ? song->cwt : 0x0214);
    it_write_u16le(f, song->cmwt ? song->cmwt : 0x0214);
    it_write_u16le(f, song->flags);

    uint16_t special = song->special;
    if (song->message && strlen(song->message) > 0) special |= 1;
    it_write_u16le(f, special);

    it_write_u8(f, song->global_vol);
    it_write_u8(f, song->mix_vol);
    it_write_u8(f, song->initial_speed);
    it_write_u8(f, song->initial_tempo);
    it_write_u8(f, song->panning_separation);
    it_write_u8(f, song->pitch_wheel_depth);

    uint16_t msg_len = (song->message) ? (uint16_t)strlen(song->message) : 0;
    it_write_u16le(f, msg_len);
    /* Placeholder for message offset */
    long msg_offset_pos = ftell(f);
    it_write_u32le(f, 0);
    it_write_u32le(f, 0); /* Reserved */

    /* Channel Pan and Channel Vol (64 bytes each) */
    uint8_t pans[64], vols[64];
    for (int i = 0; i < 64; i++) {
        pans[i] = song->channel_pan[i] | (song->channel_mute[i] ? 128 : 0);
        vols[i] = song->channel_vol[i];
    }
    it_write_bytes(f, pans, 64);
    it_write_bytes(f, vols, 64);

    /* Orders */
    it_write_bytes(f, song->orders, song->order_count);

    /* Reserve space for offset tables */
    long inst_offset_table_pos = ftell(f);
    for (int i = 0; i < song->instrument_count; i++) it_write_u32le(f, 0);
    long smp_offset_table_pos = ftell(f);
    for (int i = 0; i < song->sample_count; i++) it_write_u32le(f, 0);
    long pat_offset_table_pos = ftell(f);
    for (int i = 0; i < song->pattern_count; i++) it_write_u32le(f, 0);

    /* Song message */
    uint32_t msg_actual_offset = 0;
    if (msg_len > 0) {
        msg_actual_offset = (uint32_t)ftell(f);
        it_write_bytes(f, song->message, msg_len);
    }

    /* Write Instruments */
    uint32_t* inst_offsets = (uint32_t*)calloc(song->instrument_count, sizeof(uint32_t));
    for (int i = 0; i < song->instrument_count; i++) {
        const ITInstrument* inst = &song->instruments[i + 1];
        inst_offsets[i] = (uint32_t)ftell(f);
        it_write_u32le(f, 0x49504D49); /* "IMPI" */
        char fname[13] = {0};
        strncpy(fname, inst->filename, 12);
        it_write_bytes(f, fname, 12);
        it_write_u8(f, 0);
        it_write_u8(f, inst->nna);
        it_write_u8(f, inst->dct);
        it_write_u8(f, inst->dca);
        it_write_u16le(f, inst->fadeout);
        it_write_u8(f, (uint8_t)inst->pitch_pan_separation);
        it_write_u8(f, inst->pitch_pan_center);
        it_write_u8(f, inst->global_vol);
        it_write_u8(f, inst->default_pan);
        it_write_u8(f, inst->random_vol_variation);
        it_write_u8(f, inst->random_pan_variation);
        it_write_u16le(f, 0x0214);
        it_write_u8(f, 1);
        it_write_u8(f, 0);

        char iname[26] = {0};
        strncpy(iname, inst->name, 25);
        it_write_bytes(f, iname, 26);
        it_write_u8(f, inst->filter_cutoff);
        it_write_u8(f, inst->filter_resonance);
        it_write_u8(f, inst->midi_channel);
        it_write_u8(f, inst->midi_program);
        it_write_u16le(f, inst->midi_bank);

        for (int k = 0; k < 120; k++) {
            it_write_u8(f, inst->keyboard_note[k]);
            it_write_u8(f, inst->keyboard_sample[k]);
        }

        const ITEnvelope* envs[3] = { &inst->vol_env, &inst->pan_env, &inst->pitch_env };
        for (int e = 0; e < 3; e++) {
            it_write_u8(f, envs[e]->flags);
            it_write_u8(f, envs[e]->num_nodes);
            it_write_u8(f, envs[e]->loop_start);
            it_write_u8(f, envs[e]->loop_end);
            it_write_u8(f, envs[e]->susloop_start);
            it_write_u8(f, envs[e]->susloop_end);
            for (int n = 0; n < 25; n++) {
                it_write_u8(f, (uint8_t)envs[e]->nodes[n].y);
                it_write_u16le(f, envs[e]->nodes[n].tick);
            }
            it_write_u8(f, 0);
        }
    }

    /* Reserve Sample Headers */
    uint32_t* smp_offsets = (uint32_t*)calloc(song->sample_count, sizeof(uint32_t));
    long* smp_header_data_offset_pos = (long*)calloc(song->sample_count, sizeof(long));

    for (int i = 0; i < song->sample_count; i++) {
        const ITSample* smp = &song->samples[i + 1];
        smp_offsets[i] = (uint32_t)ftell(f);
        it_write_u32le(f, 0x53504D49); /* "IMPS" */
        char sfname[13] = {0};
        strncpy(sfname, smp->filename, 12);
        it_write_bytes(f, sfname, 12);
        it_write_u8(f, 0);
        it_write_u8(f, smp->global_vol);
        it_write_u8(f, (smp->flags | SMP_FLAG_16BIT) & ~SMP_FLAG_COMPRESSED); /* Save 16-bit PCM */
        it_write_u8(f, smp->default_vol);

        char sname[26] = {0};
        strncpy(sname, smp->name, 25);
        it_write_bytes(f, sname, 26);
        it_write_u8(f, 1); /* signed PCM */
        it_write_u8(f, smp->default_pan);
        it_write_u32le(f, smp->length);
        it_write_u32le(f, smp->loop_start);
        it_write_u32le(f, smp->loop_end);
        it_write_u32le(f, smp->c5_speed);
        it_write_u32le(f, smp->susloop_start);
        it_write_u32le(f, smp->susloop_end);

        smp_header_data_offset_pos[i] = ftell(f);
        it_write_u32le(f, 0); /* Data offset to patch */

        it_write_u8(f, smp->vibrato_speed);
        it_write_u8(f, smp->vibrato_depth);
        it_write_u8(f, smp->vibrato_rate);
        it_write_u8(f, smp->vibrato_type);
    }

    /* Write Patterns */
    uint32_t* pat_offsets = (uint32_t*)calloc(song->pattern_count, sizeof(uint32_t));
    for (int p = 0; p < song->pattern_count; p++) {
        const ITPattern* pat = &song->patterns[p];
        pat_offsets[p] = (uint32_t)ftell(f);
        long pat_header_pos = ftell(f);
        it_write_u16le(f, 0); /* Packed length placeholder */
        it_write_u16le(f, pat->rows);
        it_write_u32le(f, 0); /* Reserved */

        long pat_data_start = ftell(f);
        for (int r = 0; r < pat->rows; r++) {
            for (int ch = 0; ch < IT_MAX_CHANNELS; ch++) {
                const ITCell* c = &pat->data[r * IT_MAX_CHANNELS + ch];
                if (!c->note && !c->instrument && !c->volume && !c->effect && !c->param) continue;

                uint8_t mask = 0;
                if (c->note) mask |= 1;
                if (c->instrument) mask |= 2;
                if (c->volume) mask |= 4;
                if (c->effect || c->param) mask |= 8;

                it_write_u8(f, (uint8_t)((ch + 1) | 128));
                it_write_u8(f, mask);
                if (mask & 1) it_write_u8(f, c->note);
                if (mask & 2) it_write_u8(f, c->instrument);
                if (mask & 4) it_write_u8(f, c->volume);
                if (mask & 8) {
                    it_write_u8(f, c->effect);
                    it_write_u8(f, c->param);
                }
            }
            it_write_u8(f, 0); /* End of row */
        }
        long pat_data_end = ftell(f);
        uint16_t packed_len = (uint16_t)(pat_data_end - pat_data_start);
        fseek(f, pat_header_pos, SEEK_SET);
        it_write_u16le(f, packed_len);
        fseek(f, pat_data_end, SEEK_SET);
    }

    /* Write Sample Data and patch sample headers */
    for (int i = 0; i < song->sample_count; i++) {
        const ITSample* smp = &song->samples[i + 1];
        if (smp->length > 0 && smp->data) {
            uint32_t data_offset = (uint32_t)ftell(f);
            /* Patch offset in sample header */
            fseek(f, smp_header_data_offset_pos[i], SEEK_SET);
            it_write_u32le(f, data_offset);
            fseek(f, data_offset, SEEK_SET);

            /* Write 16-bit signed PCM */
            for (uint32_t s = 0; s < smp->length; s++) {
                float val = smp->data[s];
                int16_t v = (int16_t)IT_CLAMP(val * 32767.0f, -32768.0f, 32767.0f);
                it_write_u16le(f, (uint16_t)v);
            }
        }
    }

    /* Patch Header Offsets */
    if (msg_actual_offset > 0) {
        fseek(f, msg_offset_pos, SEEK_SET);
        it_write_u32le(f, msg_actual_offset);
    }

    fseek(f, inst_offset_table_pos, SEEK_SET);
    for (int i = 0; i < song->instrument_count; i++) it_write_u32le(f, inst_offsets[i]);

    fseek(f, smp_offset_table_pos, SEEK_SET);
    for (int i = 0; i < song->sample_count; i++) it_write_u32le(f, smp_offsets[i]);

    fseek(f, pat_offset_table_pos, SEEK_SET);
    for (int i = 0; i < song->pattern_count; i++) it_write_u32le(f, pat_offsets[i]);

    free(inst_offsets);
    free(smp_offsets);
    free(pat_offsets);
    free(smp_header_data_offset_pos);

    fclose(f);
    return true;
}

/* Fallback stubs for other module formats */
bool it_load_s3m(ITSong* song, const char* filepath);
bool it_load_mod(ITSong* song, const char* filepath);
bool it_load_xm(ITSong* song, const char* filepath);

bool it_load_module_any(ITSong* song, const char* filepath) {
    if (!song || !filepath) return false;
    FILE* f = fopen(filepath, "rb");
    if (!f) return false;

    uint8_t magic[4] = {0};
    fread(magic, 1, 4, f);
    fclose(f);

    if (memcmp(magic, "IMPM", 4) == 0) {
        return it_load_file(song, filepath);
    }
    if (memcmp(magic, "Extended Module:", 16) == 0) {
        return it_load_xm(song, filepath);
    }

    /* Check S3M by extension or header */
    const char* ext = strrchr(filepath, '.');
    if (ext) {
        if (SDL_strcasecmp(ext, ".s3m") == 0) return it_load_s3m(song, filepath);
        if (SDL_strcasecmp(ext, ".xm") == 0)  return it_load_xm(song, filepath);
        if (SDL_strcasecmp(ext, ".mod") == 0) return it_load_mod(song, filepath);
        if (SDL_strcasecmp(ext, ".it") == 0)  return it_load_file(song, filepath);
    }

    return it_load_file(song, filepath);
}
