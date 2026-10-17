#include "page_samples.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "player.h"

static int s_selected_smp = 1;

void it_page_samples_init(void) {
    s_selected_smp = 1;
}

void it_page_samples_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();
    ITSong* song = it_player_get_song();
    ITUIState* ui = it_ui_get_state();
    if (!song) return;

    int panel_y = 62;
    int panel_h = screen_h - panel_y - 28;

    /* Left Panel: Sample List */
    int list_w = 260;
    it_widget_panel(10, panel_y, list_w, panel_h, "SAMPLES (1..99) [F3]");

    static int s_smp_scroll = 0;
    int visible_items = (panel_h - 40) / 22;
    if (visible_items < 1) visible_items = 1;

    /* Mouse wheel on sample list */
    if (ui->wheel_y != 0 && ui->mouse_x >= 10 && ui->mouse_x < 10 + list_w &&
        ui->mouse_y >= panel_y && ui->mouse_y < panel_y + panel_h) {
        s_smp_scroll = IT_CLAMP(s_smp_scroll - ui->wheel_y * 2, 0, 99 - visible_items);
    }

    int list_y = panel_y + 30;
    for (int idx = 0; idx < visible_items; idx++) {
        int i = 1 + s_smp_scroll + idx;
        if (i > 99) break;

        int item_y = list_y + idx * 22;
        bool is_sel = (i == s_selected_smp);
        ITSample* s = &song->samples[i];

        if (is_sel) {
            it_gfx_fill_rect(14, item_y, list_w - 24, 20, IT_COL_ROW_HILIGHT2);
        }

        char item_str[48];
        snprintf(item_str, sizeof(item_str), "%02d: %s", i, (strlen(s->name) > 0) ? s->name : "<empty>");
        it_gfx_draw_text(18, item_y + 2, item_str, is_sel ? IT_COL_CYAN : ((s->length > 0) ? IT_COL_WHITE : IT_COL_DARK_GRAY));

        if (ui->mouse_clicked && !ui->click_consumed &&
            ui->mouse_x >= 14 && ui->mouse_x < 14 + list_w - 24 &&
            ui->mouse_y >= item_y && ui->mouse_y < item_y + 20) {
            s_selected_smp = i;
            ui->current_instrument = i;
            ui->click_consumed = true;
        }

        if (ui->mouse_double_clicked && !ui->click_consumed &&
            ui->mouse_x >= 14 && ui->mouse_x < 14 + list_w - 24 &&
            ui->mouse_y >= item_y && ui->mouse_y < item_y + 20) {
            s_selected_smp = i;
            ui->current_instrument = i;
            it_player_trigger_note(0, 61, i, 64);
            ui->click_consumed = true;
        }
    }

    /* Scrollbar for sample list */
    it_widget_scrollbar(10 + list_w - 12, list_y, 10, panel_h - 40, 99, visible_items, &s_smp_scroll);

    /* Right Panel: Sample Waveform & Controls */
    int edit_x = 10 + list_w + 6;
    int edit_w = screen_w - edit_x - 10;
    it_widget_panel(edit_x, panel_y, edit_w, panel_h, "SAMPLE WAVEFORM & PARAMETERS");

    ITSample* cur_smp = &song->samples[s_selected_smp];

    /* Sample Info Header */
    char hdr_info[128];
    snprintf(hdr_info, sizeof(hdr_info), "Sample %02d: %s | Length: %u frames | C5: %u Hz",
             s_selected_smp, cur_smp->name, cur_smp->length, cur_smp->c5_speed);
    it_gfx_draw_text_bold(edit_x + 16, panel_y + 30, hdr_info, IT_COL_YELLOW);

    /* Waveform Box */
    int wave_x = edit_x + 16;
    int wave_y = panel_y + 56;
    int wave_w = edit_w - 32;
    int wave_h = 160;

    it_gfx_fill_rect(wave_x, wave_y, wave_w, wave_h, IT_COL_PANEL_DARK);
    it_gfx_draw_rect(wave_x, wave_y, wave_w, wave_h, IT_COL_PANEL_BORDER);
    it_gfx_draw_line(wave_x, wave_y + wave_h / 2, wave_x + wave_w, wave_y + wave_h / 2, IT_COL_DARK_GRAY);

    if (cur_smp->data && cur_smp->length > 0) {
        float step = (float)cur_smp->length / (float)wave_w;
        int mid_y = wave_y + wave_h / 2;

        for (int px = 0; px < wave_w; px++) {
            uint32_t smp_idx = (uint32_t)((float)px * step);
            if (smp_idx < cur_smp->length) {
                float val = cur_smp->data[smp_idx];
                int dy = (int)(val * (float)(wave_h / 2 - 4));
                it_gfx_draw_line(wave_x + px, mid_y, wave_x + px, mid_y - dy, IT_COL_GREEN);
            }
        }

        /* Draw Loop Markers */
        if (cur_smp->flags & SMP_FLAG_LOOP) {
            int lx1 = wave_x + (int)(((float)cur_smp->loop_start / (float)cur_smp->length) * (float)wave_w);
            int lx2 = wave_x + (int)(((float)cur_smp->loop_end / (float)cur_smp->length) * (float)wave_w);
            it_gfx_draw_line(lx1, wave_y, lx1, wave_y + wave_h, IT_COL_CYAN);
            it_gfx_draw_line(lx2, wave_y, lx2, wave_y + wave_h, IT_COL_RED);
        }
    } else {
        it_gfx_draw_text(wave_x + wave_w / 2 - 60, wave_y + wave_h / 2 - 8, "No Sample Loaded", IT_COL_DARK_GRAY);
    }

    /* Controls Below Waveform */
    int ctrl_y = wave_y + wave_h + 20;

    /* Play Preview Button */
    if (it_widget_button(wave_x, ctrl_y, 160, 30, "Play Note C-5", true)) {
        it_player_trigger_note(0, 61, s_selected_smp, 64);
    }

    /* Generate Synthesizer Waveforms */
    it_gfx_draw_text(wave_x + 190, ctrl_y + 6, "Synth Generator:", IT_COL_GRAY);
    if (it_widget_button(wave_x + 320, ctrl_y, 90, 28, "Saw Lead", false)) {
        it_sample_init_synth(cur_smp, 0, "Synth Saw");
    }
    if (it_widget_button(wave_x + 420, ctrl_y, 90, 28, "Square", false)) {
        it_sample_init_synth(cur_smp, 1, "Square Bass");
    }
    if (it_widget_button(wave_x + 520, ctrl_y, 90, 28, "Sine", false)) {
        it_sample_init_synth(cur_smp, 2, "Sine Warm");
    }
    if (it_widget_button(wave_x + 620, ctrl_y, 90, 28, "808 Kick", false)) {
        it_sample_init_synth(cur_smp, 3, "808 Kick");
    }

    /* Parameters */
    int p_y = ctrl_y + 44;
    int vol = cur_smp->default_vol;
    if (it_widget_slider(wave_x, p_y, 220, 24, "Default Volume", &vol, 0, 64)) {
        cur_smp->default_vol = (uint8_t)vol;
    }

    int gvol = cur_smp->global_vol;
    if (it_widget_slider(wave_x + 360, p_y, 220, 24, "Global Volume", &gvol, 0, 64)) {
        cur_smp->global_vol = (uint8_t)gvol;
    }

    int speed_c5 = (int)cur_smp->c5_speed;
    if (it_widget_spinner(wave_x, p_y + 36, 180, 24, "C-5 Speed (Hz)", &speed_c5, 1000, 192000)) {
        cur_smp->c5_speed = (uint32_t)speed_c5;
    }

    bool loop_on = (cur_smp->flags & SMP_FLAG_LOOP) != 0;
    if (it_widget_checkbox(wave_x + 360, p_y + 36, "Enable Sample Loop", &loop_on)) {
        if (loop_on) cur_smp->flags |= SMP_FLAG_LOOP;
        else cur_smp->flags &= ~SMP_FLAG_LOOP;
    }
}
