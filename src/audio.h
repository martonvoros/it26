#ifndef IT_AUDIO_H
#define IT_AUDIO_H

#include "common.h"
#include "song.h"

bool it_audio_init(uint32_t sample_rate);
void it_audio_shutdown(void);
void it_audio_lock(void);
void it_audio_unlock(void);

uint32_t it_audio_get_sample_rate(void);
int it_audio_get_device_count(void);
const char* it_audio_get_device_name(int index);

#endif /* IT_AUDIO_H */
