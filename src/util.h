#ifndef IT_UTIL_H
#define IT_UTIL_H

#include "common.h"

uint8_t  it_read_u8(FILE* f);
uint16_t it_read_u16le(FILE* f);
uint32_t it_read_u32le(FILE* f);
int      it_read_bytes(FILE* f, void* dst, size_t size);

void it_write_u8(FILE* f, uint8_t v);
void it_write_u16le(FILE* f, uint16_t v);
void it_write_u32le(FILE* f, uint32_t v);
int  it_write_bytes(FILE* f, const void* src, size_t size);

uint8_t  it_mem_u8(const uint8_t* buf, size_t* offset, size_t max_size);
uint16_t it_mem_u16le(const uint8_t* buf, size_t* offset, size_t max_size);
uint32_t it_mem_u32le(const uint8_t* buf, size_t* offset, size_t max_size);

float it_note_to_freq(int note, uint32_t c5_speed);
float it_linear_slide_factor(int slide);
int8_t it_get_vibrato_table_val(uint8_t type, uint8_t pos);

void it_safe_copy_string(char* dst, const char* src, size_t max_len);
char* it_load_file_to_memory(const char* path, size_t* out_size);

#endif /* IT_UTIL_H */
