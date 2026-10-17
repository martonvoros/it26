#include "util.h"

static const int8_t s_fine_sine[256] = {
    0,   2,   3,   5,   6,   8,   9,  11,  12,  14,  16,  17,  19,  20,  22,  23,
   24,  26,  27,  29,  30,  32,  33,  34,  36,  37,  38,  39,  41,  42,  43,  44,
   45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  56,  57,  58,  59,
   59,  60,  60,  61,  61,  62,  62,  62,  63,  63,  63,  64,  64,  64,  64,  64,
   64,  64,  64,  64,  64,  64,  63,  63,  63,  62,  62,  62,  61,  61,  60,  60,
   59,  59,  58,  57,  56,  56,  55,  54,  53,  52,  51,  50,  49,  48,  47,  46,
   45,  44,  43,  42,  41,  39,  38,  37,  36,  34,  33,  32,  30,  29,  27,  26,
   24,  23,  22,  20,  19,  17,  16,  14,  12,  11,   9,   8,   6,   5,   3,   2,
    0,  -2,  -3,  -5,  -6,  -8,  -9, -11, -12, -14, -16, -17, -19, -20, -22, -23,
  -24, -26, -27, -29, -30, -32, -33, -34, -36, -37, -38, -39, -41, -42, -43, -44,
  -45, -46, -47, -48, -49, -50, -51, -52, -53, -54, -55, -56, -56, -57, -58, -59,
  -59, -60, -60, -61, -61, -62, -62, -62, -63, -63, -63, -64, -64, -64, -64, -64,
  -64, -64, -64, -64, -64, -64, -63, -63, -63, -62, -62, -62, -61, -61, -60, -60,
  -59, -59, -58, -57, -56, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, -46,
  -45, -44, -43, -42, -41, -39, -38, -37, -36, -34, -33, -32, -30, -29, -27, -26,
  -24, -23, -22, -20, -19, -17, -16, -14, -12, -11,  -9,  -8,  -6,  -5,  -3,  -2
};

static const int8_t s_fine_ramp[256] = {
   64,  63,  63,  62,  62,  61,  61,  60,  60,  59,  59,  58,  58,  57,  57,  56,
   56,  55,  55,  54,  54,  53,  53,  52,  52,  51,  51,  50,  50,  49,  49,  48,
   48,  47,  47,  46,  46,  45,  45,  44,  44,  43,  43,  42,  42,  41,  41,  40,
   40,  39,  39,  38,  38,  37,  37,  36,  36,  35,  35,  34,  34,  33,  33,  32,
   32,  31,  31,  30,  30,  29,  29,  28,  28,  27,  27,  26,  26,  25,  25,  24,
   24,  23,  23,  22,  22,  21,  21,  20,  20,  19,  19,  18,  18,  17,  17,  16,
   16,  15,  15,  14,  14,  13,  13,  12,  12,  11,  11,  10,  10,   9,   9,   8,
    8,   7,   7,   6,   6,   5,   5,   4,   4,   3,   3,   2,   2,   1,   1,   0,
    0,  -1,  -1,  -2,  -2,  -3,  -3,  -4,  -4,  -5,  -5,  -6,  -6,  -7,  -7,  -8,
   -8,  -9,  -9, -10, -10, -11, -11, -12, -12, -13, -13, -14, -14, -15, -15, -16,
  -16, -17, -17, -18, -18, -19, -19, -20, -20, -21, -21, -22, -22, -23, -23, -24,
  -24, -25, -25, -26, -26, -27, -27, -28, -28, -29, -29, -30, -30, -31, -31, -32,
  -32, -33, -33, -34, -34, -35, -35, -36, -36, -37, -37, -38, -38, -39, -39, -40,
  -40, -41, -41, -42, -42, -43, -43, -44, -44, -45, -45, -46, -46, -47, -47, -48,
  -48, -49, -49, -50, -50, -51, -51, -52, -52, -53, -53, -54, -54, -55, -55, -56,
  -56, -57, -57, -58, -58, -59, -59, -60, -60, -61, -61, -62, -62, -63, -63, -64
};

uint8_t it_read_u8(FILE* f) {
    int c = fgetc(f);
    return (c == EOF) ? 0 : (uint8_t)c;
}

uint16_t it_read_u16le(FILE* f) {
    uint8_t b0 = it_read_u8(f);
    uint8_t b1 = it_read_u8(f);
    return (uint16_t)(b0 | (b1 << 8));
}

uint32_t it_read_u32le(FILE* f) {
    uint8_t b0 = it_read_u8(f);
    uint8_t b1 = it_read_u8(f);
    uint8_t b2 = it_read_u8(f);
    uint8_t b3 = it_read_u8(f);
    return (uint32_t)(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

int it_read_bytes(FILE* f, void* dst, size_t size) {
    return (int)fread(dst, 1, size, f);
}

void it_write_u8(FILE* f, uint8_t v) {
    fputc(v, f);
}

void it_write_u16le(FILE* f, uint16_t v) {
    fputc(v & 0xFF, f);
    fputc((v >> 8) & 0xFF, f);
}

void it_write_u32le(FILE* f, uint32_t v) {
    fputc(v & 0xFF, f);
    fputc((v >> 8) & 0xFF, f);
    fputc((v >> 16) & 0xFF, f);
    fputc((v >> 24) & 0xFF, f);
}

int it_write_bytes(FILE* f, const void* src, size_t size) {
    return (int)fwrite(src, 1, size, f);
}

uint8_t it_mem_u8(const uint8_t* buf, size_t* offset, size_t max_size) {
    if (*offset >= max_size) return 0;
    return buf[(*offset)++];
}

uint16_t it_mem_u16le(const uint8_t* buf, size_t* offset, size_t max_size) {
    uint8_t b0 = it_mem_u8(buf, offset, max_size);
    uint8_t b1 = it_mem_u8(buf, offset, max_size);
    return (uint16_t)(b0 | (b1 << 8));
}

uint32_t it_mem_u32le(const uint8_t* buf, size_t* offset, size_t max_size) {
    uint8_t b0 = it_mem_u8(buf, offset, max_size);
    uint8_t b1 = it_mem_u8(buf, offset, max_size);
    uint8_t b2 = it_mem_u8(buf, offset, max_size);
    uint8_t b3 = it_mem_u8(buf, offset, max_size);
    return (uint32_t)(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

float it_note_to_freq(int note, uint32_t c5_speed) {
    /* Note ranges from 1 (C-0) to 120 (B-9) */
    /* C-5 is note 61 (5*12 + 1) */
    if (note < 1 || note > 120) return 0.0f;
    if (c5_speed == 0) c5_speed = 8363 * 4;
    int diff = note - 61; /* semitones relative to C-5 */
    return (float)c5_speed * powf(2.0f, (float)diff / 12.0f);
}

float it_linear_slide_factor(int slide) {
    /* 2^(slide / 768) */
    return powf(2.0f, (float)slide / 768.0f);
}

int8_t it_get_vibrato_table_val(uint8_t type, uint8_t pos) {
    switch (type & 3) {
        case 0: return s_fine_sine[pos];
        case 1: return s_fine_ramp[pos];
        case 2: return (pos < 128) ? 64 : 0;
        case 3: return (int8_t)((rand() % 129) - 64);
        default: return 0;
    }
}

void it_safe_copy_string(char* dst, const char* src, size_t max_len) {
    if (!dst || max_len == 0) return;
    if (!src) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, max_len - 1);
    dst[max_len - 1] = '\0';
}

char* it_load_file_to_memory(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return NULL; }
    char* buf = (char*)malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, sz, f);
    buf[rd] = '\0';
    fclose(f);
    if (out_size) *out_size = rd;
    return buf;
}
