#include "fmt_wav.h"
#include "util.h"

bool it_wav_write_header(FILE* f, uint32_t sample_rate, uint16_t channels, uint16_t bits, uint32_t data_bytes) {
    if (!f) return false;
    uint32_t riff_size = 36 + data_bytes;
    uint32_t byte_rate = sample_rate * channels * (bits / 8);
    uint16_t block_align = channels * (bits / 8);

    fwrite("RIFF", 1, 4, f);
    it_write_u32le(f, riff_size);
    fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f);
    it_write_u32le(f, 16); /* Subchunk1Size for PCM */
    it_write_u16le(f, 1);  /* AudioFormat 1 = PCM */
    it_write_u16le(f, channels);
    it_write_u32le(f, sample_rate);
    it_write_u32le(f, byte_rate);
    it_write_u16le(f, block_align);
    it_write_u16le(f, bits);
    fwrite("data", 1, 4, f);
    it_write_u32le(f, data_bytes);
    return true;
}

bool it_wav_load_sample(ITSample* smp, const char* filepath) {
    if (!smp || !filepath) return false;
    FILE* f = fopen(filepath, "rb");
    if (!f) return false;

    char tag[5] = {0};
    if (fread(tag, 1, 4, f) != 4 || memcmp(tag, "RIFF", 4) != 0) {
        fclose(f);
        return false;
    }
    it_read_u32le(f); /* File size */
    if (fread(tag, 1, 4, f) != 4 || memcmp(tag, "WAVE", 4) != 0) {
        fclose(f);
        return false;
    }

    uint16_t audio_fmt = 0;
    uint16_t channels = 1;
    uint32_t sample_rate = 44100;
    uint16_t bits = 16;
    uint32_t data_size = 0;
    long data_pos = 0;

    while (!feof(f)) {
        char chunk_id[5] = {0};
        if (fread(chunk_id, 1, 4, f) != 4) break;
        uint32_t chunk_sz = it_read_u32le(f);
        long next_chunk = ftell(f) + chunk_sz;

        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            audio_fmt = it_read_u16le(f);
            channels = it_read_u16le(f);
            sample_rate = it_read_u32le(f);
            it_read_u32le(f); /* byte rate */
            it_read_u16le(f); /* block align */
            bits = it_read_u16le(f);
        } else if (memcmp(chunk_id, "data", 4) == 0) {
            data_size = chunk_sz;
            data_pos = ftell(f);
            break;
        }
        fseek(f, next_chunk, SEEK_SET);
    }

    if (data_size == 0 || data_pos == 0) {
        fclose(f);
        return false;
    }

    fseek(f, data_pos, SEEK_SET);
    it_sample_free_data(smp);

    uint32_t bytes_per_sample = bits / 8;
    if (bytes_per_sample == 0) bytes_per_sample = 1;
    uint32_t num_frames = data_size / (channels * bytes_per_sample);

    smp->length = num_frames;
    smp->c5_speed = sample_rate;
    smp->default_vol = 64;
    smp->global_vol = 64;
    smp->default_pan = 0x80 | 32;
    smp->flags = SMP_FLAG_EXISTS | (bits >= 16 ? SMP_FLAG_16BIT : 0);

    const char* base_name = strrchr(filepath, '/');
    if (!base_name) base_name = strrchr(filepath, '\\');
    base_name = base_name ? base_name + 1 : filepath;
    strncpy(smp->name, base_name, 25);
    smp->name[25] = '\0';

    smp->data = (float*)malloc(num_frames * sizeof(float));
    if (!smp->data) {
        fclose(f);
        return false;
    }

    for (uint32_t i = 0; i < num_frames; i++) {
        float sample_val = 0.0f;
        if (audio_fmt == 3 && bits == 32) { /* IEEE float */
            float fval = 0.0f;
            fread(&fval, sizeof(float), 1, f);
            sample_val = fval;
        } else if (bits == 16) {
            int16_t s16 = (int16_t)it_read_u16le(f);
            sample_val = (float)s16 / 32768.0f;
        } else if (bits == 24) {
            uint8_t b0 = it_read_u8(f);
            uint8_t b1 = it_read_u8(f);
            uint8_t b2 = it_read_u8(f);
            int32_t s24 = (int32_t)((b0 << 8) | (b1 << 16) | (b2 << 24));
            sample_val = (float)s24 / 2147483648.0f;
        } else { /* 8-bit unsigned */
            uint8_t u8 = it_read_u8(f);
            sample_val = (float)((int8_t)(u8 - 128)) / 128.0f;
        }
        smp->data[i] = sample_val;

        /* Skip extra channels for now if stereo/multi */
        for (int c = 1; c < channels; c++) {
            if (bits == 16) it_read_u16le(f);
            else if (bits == 24) { it_read_u8(f); it_read_u8(f); it_read_u8(f); }
            else if (bits == 32) it_read_u32le(f);
            else it_read_u8(f);
        }
    }

    fclose(f);
    return true;
}

bool it_wav_save_sample(const ITSample* smp, const char* filepath) {
    if (!smp || !smp->data || smp->length == 0 || !filepath) return false;
    FILE* f = fopen(filepath, "wb");
    if (!f) return false;

    uint32_t data_bytes = smp->length * sizeof(int16_t);
    uint32_t srate = smp->c5_speed ? smp->c5_speed : 44100;
    it_wav_write_header(f, srate, 1, 16, data_bytes);

    for (uint32_t i = 0; i < smp->length; i++) {
        float val = smp->data[i];
        int16_t s16 = (int16_t)IT_CLAMP(val * 32767.0f, -32768.0f, 32767.0f);
        it_write_u16le(f, (uint16_t)s16);
    }

    fclose(f);
    return true;
}
