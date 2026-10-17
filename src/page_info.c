#include "page_info.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "player.h"
#include "mixer.h"

void it_page_info_init(void) {
}

void it_page_info_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();
    ITSong* song = it_player_get_song();
    if (!song) return;

    int panel_y = 62;
    int panel_h = screen_h - panel_y - 28;
    it_widget_panel(10, panel_y, screen_w - 20, panel_h, "64-CHANNEL REAL-TIME VISUALIZER & MODULE INFORMATION [F9]");

    /* Top Info Bar */
    int active_voices = 0;
    for (int i = 0; i < IT_MAX_VIRTUAL_CHANNELS; i++) {
        const ITVoice* v = it_mixer_get_voice(i);
        if (v && v->active) active_voices++;
    }

    char info_buf[256];
    snprintf(info_buf, sizeof(info_buf),
             "Song: %s | Tempo: %d BPM | Speed: %d | Order: %03d | Row: %02d | Active Voices: %d / %d",
             song->name, it_player_get_tempo(), it_player_get_speed(),
             it_player_get_order(), it_player_get_row(), active_voices, IT_MAX_VIRTUAL_CHANNELS);
    it_gfx_draw_text_bold(24, panel_y + 30, info_buf, IT_COL_YELLOW);

    /* 64 Oscilloscope Channel Boxes (8 columns x 8 rows) */
    int start_x = 24;
    int start_y = panel_y + 56;
    int cols = 8;
    int rows = 8;
    int box_w = (screen_w - 68) / cols;
    int box_h = (panel_h - 70) / rows;

    for (int ch = 0; ch < IT_MAX_CHANNELS; ch++) {
        int r = ch / cols;
        int c = ch % cols;
        int bx = start_x + c * box_w;
        int by = start_y + r * box_h;

        bool is_muted = song->channel_mute[ch];

        it_gfx_fill_rect(bx, by, box_w - 4, box_h - 4, is_muted ? IT_COL_PANEL_DARK : (ITColor){ 8, 14, 26, 255 });
        it_gfx_draw_rect(bx, by, box_w - 4, box_h - 4, is_muted ? IT_COL_DARK_GRAY : IT_COL_PANEL_BORDER);

        char ch_lbl[8];
        snprintf(ch_lbl, sizeof(ch_lbl), "%02d", ch + 1);
        it_gfx_draw_text(bx + 4, by + 2, ch_lbl, is_muted ? IT_COL_DARK_GRAY : IT_COL_GRAY);

        if (!is_muted) {
            const ITChannelVis* vis = it_mixer_get_channel_vis(ch);
            if (vis) {
                int mid_y = by + (box_h - 4) / 2;
                int inner_w = box_w - 20;
                float vu = vis->vu_level;

                /* Draw waveform mini-scope */
                for (int px = 0; px < inner_w; px++) {
                    int s_idx = (px * IT_SCOPE_BUFFER_SIZE) / inner_w;
                    float s_val = vis->scope[s_idx];
                    int dy = (int)(s_val * (float)((box_h - 8) / 2));
                    it_gfx_draw_line(bx + 16 + px, mid_y, bx + 16 + px, mid_y - dy,
                                     (vu > 0.7f) ? IT_COL_RED : ((vu > 0.3f) ? IT_COL_YELLOW : IT_COL_GREEN));
                }
            }
        }
    }
}
