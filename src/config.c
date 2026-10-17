#include "config.h"

static ITConfig s_config;

void it_config_init_defaults(ITConfig* cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(ITConfig));
    cfg->ui_scale = 1.5f;             /* 1.5x gives crisp, gorgeous readable modern resolution on 1080p/1440p */
    cfg->sample_rate = 48000;
    cfg->audio_device_index = 0;
    cfg->midi_device_index = 0;
    cfg->master_volume = 1.0f;
    cfg->show_help_on_startup = true; /* Shows detailed help on first startup */
    cfg->fullscreen = false;
    cfg->window_width = 1280;
    cfg->window_height = 800;
    strcpy(cfg->last_dir, ".");
}

bool it_config_load(ITConfig* cfg) {
    it_config_init_defaults(cfg);
    FILE* f = fopen("it_config.ini", "r");
    if (!f) return false;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64], val[192];
        if (sscanf(line, "%63[^=]=%191[^\r\n]", key, val) == 2) {
            if (strcmp(key, "ui_scale") == 0) cfg->ui_scale = (float)atof(val);
            else if (strcmp(key, "sample_rate") == 0) cfg->sample_rate = (uint32_t)atoi(val);
            else if (strcmp(key, "show_help") == 0) cfg->show_help_on_startup = (atoi(val) != 0);
            else if (strcmp(key, "fullscreen") == 0) cfg->fullscreen = (atoi(val) != 0);
            else if (strcmp(key, "window_w") == 0) cfg->window_width = atoi(val);
            else if (strcmp(key, "window_h") == 0) cfg->window_height = atoi(val);
            else if (strcmp(key, "last_dir") == 0) strncpy(cfg->last_dir, val, sizeof(cfg->last_dir) - 1);
        }
    }
    fclose(f);
    return true;
}

bool it_config_save(const ITConfig* cfg) {
    if (!cfg) return false;
    FILE* f = fopen("it_config.ini", "w");
    if (!f) return false;

    fprintf(f, "[ImpulseTracker]\n");
    fprintf(f, "ui_scale=%.2f\n", cfg->ui_scale);
    fprintf(f, "sample_rate=%u\n", cfg->sample_rate);
    fprintf(f, "show_help=%d\n", cfg->show_help_on_startup ? 1 : 0);
    fprintf(f, "fullscreen=%d\n", cfg->fullscreen ? 1 : 0);
    fprintf(f, "window_w=%d\n", cfg->window_width);
    fprintf(f, "window_h=%d\n", cfg->window_height);
    fprintf(f, "last_dir=%s\n", cfg->last_dir);

    fclose(f);
    return true;
}

ITConfig* it_config_get(void) {
    return &s_config;
}
