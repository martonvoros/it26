#include "page_songvars.h"
#include "song.h"
#include "player.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"

static int s_ch_offset = 0;

void it_page_songvars_init(void) {
    s_ch_offset = 0;
}

void it_page_songvars_render(void) {
    ITSong* song = it_player_get_song();
    if (!song) return;

    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();

    int panel_w = (screen_w - 36) / 2;
    int panel_y = 62;
    int panel_h = screen_h - panel_y - 28;

    /* Left Panel: General Song Properties */
    it_widget_panel(10, panel_y, panel_w, panel_h, "General Properties [F12]");

    int lx = 26;
    int ly = panel_y + 36;

    /* Song Title */
    it_gfx_draw_text(lx, ly, "Song Title:", IT_COL_GRAY);
    it_widget_text_input(lx + 120, ly - 4, 300, 24, song->name, sizeof(song->name));
    ly += 32;

    /* Initial Speed */
    it_gfx_draw_text(lx, ly, "Initial Speed:", IT_COL_GRAY);
    int speed_val = song->initial_speed;
    if (it_widget_spinner(lx + 120, ly - 4, 120, 24, " ticks", &speed_val, 1, 32)) {
        song->initial_speed = (uint8_t)speed_val;
    }
    ly += 32;

    /* Initial Tempo */
    it_gfx_draw_text(lx, ly, "Initial Tempo:", IT_COL_GRAY);
    int tempo_val = song->initial_tempo;
    if (it_widget_spinner(lx + 120, ly - 4, 120, 24, " BPM", &tempo_val, 32, 255)) {
        song->initial_tempo = (uint8_t)tempo_val;
    }
    ly += 32;

    /* Global Volume */
    it_gfx_draw_text(lx, ly, "Global Volume:", IT_COL_GRAY);
    int gvol = song->global_vol;
    if (it_widget_slider(lx + 120, ly - 2, 200, 20, NULL, &gvol, 0, 128)) {
        song->global_vol = (uint8_t)gvol;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%d / 128", gvol);
    it_gfx_draw_text(lx + 330, ly, buf, IT_COL_WHITE);
    ly += 32;

    /* Mix Volume */
    it_gfx_draw_text(lx, ly, "Mix Volume:", IT_COL_GRAY);
    int mvol = song->mix_vol;
    if (it_widget_slider(lx + 120, ly - 2, 200, 20, NULL, &mvol, 0, 128)) {
        song->mix_vol = (uint8_t)mvol;
    }
    snprintf(buf, sizeof(buf), "%d / 128", mvol);
    it_gfx_draw_text(lx + 330, ly, buf, IT_COL_WHITE);
    ly += 32;

    /* Panning Separation */
    it_gfx_draw_text(lx, ly, "Pan Separation:", IT_COL_GRAY);
    int pan_sep = song->panning_separation;
    if (it_widget_slider(lx + 120, ly - 2, 200, 20, NULL, &pan_sep, 0, 128)) {
        song->panning_separation = (uint8_t)pan_sep;
    }
    snprintf(buf, sizeof(buf), "%d", pan_sep);
    it_gfx_draw_text(lx + 330, ly, buf, IT_COL_WHITE);
    ly += 32;

    /* Row Highlight */
    it_gfx_draw_text(lx, ly, "Highlight Rows:", IT_COL_GRAY);
    int minor_hi = (int)(song->hilight_rows & 0xFF);
    int major_hi = (int)((song->hilight_rows >> 8) & 0xFF);
    if (it_widget_spinner(lx + 120, ly - 4, 80, 24, "", &minor_hi, 1, 64)) {
        song->hilight_rows = (uint16_t)((major_hi << 8) | (minor_hi & 0xFF));
    }
    it_gfx_draw_text(lx + 210, ly, "Major:", IT_COL_GRAY);
    if (it_widget_spinner(lx + 260, ly - 4, 80, 24, "", &major_hi, 1, 128)) {
        song->hilight_rows = (uint16_t)((major_hi << 8) | (minor_hi & 0xFF));
    }

    /* Right Panel: Playback & Compatibility Flags */
    int rx = 10 + panel_w + 10;
    it_widget_panel(rx, panel_y, panel_w, panel_h, "Tracker Flags & Compatibility");

    int rlx = rx + 20;
    int rly = panel_y + 36;

    bool linear = (song->flags & SONG_FLAG_LINEAR_SLIDES) != 0;
    if (it_widget_checkbox(rlx, rly, "Linear Slides (instead of Amiga slides)", &linear)) {
        if (linear) song->flags |= SONG_FLAG_LINEAR_SLIDES;
        else song->flags &= ~SONG_FLAG_LINEAR_SLIDES;
    }
    rly += 32;

    bool old_fx = (song->flags & SONG_FLAG_OLD_EFFECTS) != 0;
    if (it_widget_checkbox(rlx, rly, "Old Effects Compatibility Mode", &old_fx)) {
        if (old_fx) song->flags |= SONG_FLAG_OLD_EFFECTS;
        else song->flags &= ~SONG_FLAG_OLD_EFFECTS;
    }
    rly += 32;

    bool link_g = (song->flags & SONG_FLAG_LINK_G) != 0;
    if (it_widget_checkbox(rlx, rly, "Link Effect G memory with Portamento", &link_g)) {
        if (link_g) song->flags |= SONG_FLAG_LINK_G;
        else song->flags &= ~SONG_FLAG_LINK_G;
    }
    rly += 32;

    bool inst_mode = (song->flags & SONG_FLAG_INSTRUMENTS) != 0;
    if (it_widget_checkbox(rlx, rly, "Use Instruments (Envelopes, NNA, Keymaps)", &inst_mode)) {
        if (inst_mode) song->flags |= SONG_FLAG_INSTRUMENTS;
        else song->flags &= ~SONG_FLAG_INSTRUMENTS;
    }
    rly += 32;

    bool stereo = (song->flags & SONG_FLAG_STEREO) != 0;
    if (it_widget_checkbox(rlx, rly, "Stereo Master Output", &stereo)) {
        if (stereo) song->flags |= SONG_FLAG_STEREO;
        else song->flags &= ~SONG_FLAG_STEREO;
    }
    rly += 40;

    /* Message summary button */
    if (it_widget_button(rlx, rly, 260, 30, "View / Edit Song Message [Shift+F9]", false)) {
        it_ui_set_page(PAGE_MESSAGE);
    }

    /* Bottom Panel: Channel Volumes & Panning (16 channels at a time) */
    int bpanel_y = 44 + panel_h + 12;
    int bpanel_h = screen_h - bpanel_y - 20;
    it_widget_panel(10, bpanel_y, screen_w - 20, bpanel_h, "Initial Channel Volumes & Panning");

    /* Channel bank selector */
    char bank_str[48];
    snprintf(bank_str, sizeof(bank_str), "Channels %02d-%02d", s_ch_offset + 1, s_ch_offset + 16);
    it_gfx_draw_text_bold(26, bpanel_y + 30, bank_str, IT_COL_YELLOW);

    if (it_widget_button(220, bpanel_y + 26, 90, 24, "< Prev 16", s_ch_offset > 0)) {
        if (s_ch_offset >= 16) s_ch_offset -= 16;
    }
    if (it_widget_button(320, bpanel_y + 26, 90, 24, "Next 16 >", s_ch_offset < 48)) {
        if (s_ch_offset + 16 < IT_MAX_CHANNELS) s_ch_offset += 16;
    }

    int col_w = (screen_w - 64) / 16;
    for (int i = 0; i < 16; i++) {
        int ch = s_ch_offset + i;
        if (ch >= IT_MAX_CHANNELS) break;
        int cx = 24 + i * col_w;
        int cy = bpanel_y + 60;

        char ch_title[16];
        snprintf(ch_title, sizeof(ch_title), "Ch %02d", ch + 1);
        it_gfx_draw_text_bold(cx, cy, ch_title, song->channel_mute[ch] ? IT_COL_GRAY : IT_COL_CYAN);

        /* Vol slider */
        int ch_vol = song->channel_vol[ch];
        if (it_widget_slider(cx, cy + 24, col_w - 8, 18, NULL, &ch_vol, 0, 64)) {
            song->channel_vol[ch] = (uint8_t)ch_vol;
        }

        /* Pan slider */
        int ch_pan = song->channel_pan[ch];
        if (it_widget_slider(cx, cy + 54, col_w - 8, 18, NULL, &ch_pan, 0, 64)) {
            song->channel_pan[ch] = (uint8_t)ch_pan;
        }

        /* Mute button */
        if (it_widget_button(cx, cy + 84, col_w - 8, 22, song->channel_mute[ch] ? "MUTED" : "ON", song->channel_mute[ch])) {
            song->channel_mute[ch] = !song->channel_mute[ch];
            it_player_set_channel_mute(ch, song->channel_mute[ch]);
        }
    }
}
