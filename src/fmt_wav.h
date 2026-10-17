#ifndef IT_FMT_WAV_H
#define IT_FMT_WAV_H

#include "song.h"

bool it_wav_load_sample(ITSample* smp, const char* filepath);
bool it_wav_save_sample(const ITSample* smp, const char* filepath);
bool it_wav_write_header(FILE* f, uint32_t sample_rate, uint16_t channels, uint16_t bits, uint32_t data_bytes);

#endif /* IT_FMT_WAV_H */
