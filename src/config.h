#ifndef IT_CONFIG_H
#define IT_CONFIG_H

#include "common.h"

typedef struct {
    float ui_scale;             /* 0.0 = auto, 1.0, 1.25, 1.5, 2.0, 2.5, 3.0 */
    uint32_t sample_rate;       /* 44100, 48000, 96000 */
    int audio_device_index;
    int midi_device_index;
    float master_volume;        /* 0.0 .. 1.0 */
    bool show_help_on_startup;  /* true by default */
    bool fullscreen;
    int window_width;
    int window_height;
    char last_dir[512];
} ITConfig;

void it_config_init_defaults(ITConfig* cfg);
bool it_config_load(ITConfig* cfg);
bool it_config_save(const ITConfig* cfg);

ITConfig* it_config_get(void);

#endif /* IT_CONFIG_H */
