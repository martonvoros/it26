#include "page_help.h"
#include "help_text.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "config.h"
#include "fmt_it.h"
#include "player.h"
#include "midi.h"

static int s_active_tab = 0;
static int s_scroll_y = 0;

void it_page_help_init(void) {
    s_active_tab = 0;
    s_scroll_y = 0;
}

void it_page_help_set_tab(int tab_index) {
    if (tab_index >= 0 && tab_index < it_help_get_section_count()) {
        s_active_tab = tab_index;
        s_scroll_y = 0;
    }
}

int it_page_help_get_tab(void) {
    return s_active_tab;
}

void it_page_help_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();

    /* Dynamic Width Tabs */
    int tab_x = 10;
    int tab_y = 62;
    int tab_h = 28;

    int num_sections = it_help_get_section_count();
    int cur_tx = tab_x;
    for (int i = 0; i < num_sections; i++) {
        const ITHelpSection* sec = it_help_get_section(i);
        int tw = it_gfx_text_width(sec->tab_title) + 20;
        if (tw < 70) tw = 70;
        if (it_widget_tab(cur_tx, tab_y, tw, tab_h, sec->tab_title, s_active_tab == i)) {
            s_active_tab = i;
            s_scroll_y = 0;
        }
        cur_tx += tw + 4;
    }

    /* Close Button */
    if (it_widget_button(screen_w - 180, tab_y, 170, tab_h, "Close Help [Esc]", false)) {
        it_ui_set_page(PAGE_PATTERN);
    }

    /* Main Content Panel */
    int panel_x = 10;
    int panel_y = 94;
    int panel_w = screen_w - 20;
    int panel_h = screen_h - panel_y - 28;

    it_widget_panel(panel_x, panel_y, panel_w, panel_h, NULL);

    const ITHelpSection* cur_sec = it_help_get_section(s_active_tab);
    if (cur_sec) {
        it_gfx_set_clip(panel_x + 16, panel_y + 12, panel_w - 32, panel_h - 24);

        it_gfx_draw_text_bold(panel_x + 20, panel_y + 16 - s_scroll_y, cur_sec->section_title, IT_COL_YELLOW);
        it_gfx_draw_line(panel_x + 20, panel_y + 38 - s_scroll_y, panel_x + panel_w - 40, panel_y + 38 - s_scroll_y, IT_COL_PANEL_BORDER);

        it_gfx_draw_text(panel_x + 20, panel_y + 50 - s_scroll_y, cur_sec->content, IT_COL_WHITE);

        /* Extra interactive action buttons for MIDI setup & remapping tutorial */
        if (s_active_tab == 4) { /* MIDI Tutorial Tab */
            int btn_y = panel_y + 450 - s_scroll_y;
            it_gfx_draw_text_bold(panel_x + 20, btn_y, "LIVE CONTROLLER TEST & INSTANT REMAPPING:", IT_COL_YELLOW);
            btn_y += 24;

            /* Live incoming MIDI check */
            ITMidiMessage last_msg;
            uint32_t age_ms = 999999;
            bool has_msg = it_midi_get_last_message(&last_msg, &age_ms);
            bool active = has_msg && (age_ms < 300);

            char mon_str[128];
            if (has_msg && age_ms < 5000) {
                snprintf(mon_str, sizeof(mon_str), "Signal: [● MIDI IN] 0x%02X Data1=%d Data2=%d (Age: %u ms)",
                         last_msg.status, last_msg.data1, last_msg.data2, age_ms);
            } else {
                snprintf(mon_str, sizeof(mon_str), "Signal: [● MIDI IN] Ready - turn a knob or tap a pad on your controller");
            }
            it_gfx_draw_text_bold(panel_x + 20, btn_y, mon_str, active ? IT_COL_GREEN : IT_COL_GRAY);
            btn_y += 28;

            if (it_widget_button(panel_x + 20, btn_y, 280, 32, "Map Controller in MIDI Settings [F11]", true)) {
                it_ui_set_page(PAGE_SETTINGS);
            }
        }

        it_gfx_reset_clip();

        /* Calculate exact content height based on newline count */
        int num_lines = 1;
        for (const char* p = cur_sec->content; *p; p++) {
            if (*p == '\n') num_lines++;
        }
        int total_content_h = 60 + num_lines * 19;
        int max_scroll = total_content_h - (panel_h - 40);
        if (max_scroll < 0) max_scroll = 0;

        /* Mouse Wheel Scroll in help */
        ITUIState* ui = it_ui_get_state();
        if (ui->wheel_y != 0) {
            s_scroll_y -= ui->wheel_y * 36;
            if (s_scroll_y < 0) s_scroll_y = 0;
            if (s_scroll_y > max_scroll) s_scroll_y = max_scroll;
        }

        /* Scrollbar indicator on right side if scrollable */
        if (max_scroll > 0) {
            int sb_x = panel_x + panel_w - 14;
            int sb_y = panel_y + 14;
            int sb_h = panel_h - 28;
            it_gfx_fill_rect(sb_x, sb_y, 6, sb_h, (ITColor){ 16, 22, 32, 255 });
            float thumb_ratio = (float)(panel_h - 40) / (float)total_content_h;
            if (thumb_ratio < 0.1f) thumb_ratio = 0.1f;
            int thumb_h = (int)(thumb_ratio * (float)sb_h);
            float scroll_ratio = (float)s_scroll_y / (float)max_scroll;
            int thumb_y = sb_y + (int)(scroll_ratio * (float)(sb_h - thumb_h));
            it_gfx_fill_rect(sb_x, thumb_y, 6, thumb_h, IT_COL_CYAN_DIM);
        }
    }

    /* Bottom Footer Bar */
    int footer_y = screen_h - 40;
    it_gfx_fill_rect(0, footer_y, screen_w, 40, IT_COL_PANEL_DARK);

    ITConfig* cfg = it_config_get();
    it_widget_checkbox(20, footer_y + 10, "Show this Help on Startup", &cfg->show_help_on_startup);

    /* Quick jump buttons to About & License */
    if (it_widget_button(270, footer_y + 6, 120, 28, "About IT26", s_active_tab == 5)) {
        s_active_tab = 5;
        s_scroll_y = 0;
    }
    if (it_widget_button(396, footer_y + 6, 120, 28, "View License", s_active_tab == 6)) {
        s_active_tab = 6;
        s_scroll_y = 0;
    }

    /* Load Official Demo Song button */
    if (it_widget_button(524, footer_y + 6, 260, 28, "Load Official Demo (TUTE.IT)", false)) {
        ITSong* song = it_player_get_song();
        if (song) {
            if (it_load_file(song, "reference/impulse-tracker/Tutorial/TUTE.IT")) {
                it_ui_set_status("Loaded Jeffrey Lim's official tutorial module (TUTE.IT)!");
                it_ui_set_page(PAGE_PATTERN);
                it_player_play_song();
            }
        }
    }

    if (it_widget_button(screen_w - 220, footer_y + 6, 200, 28, "Start Tracking Now [F2]", true)) {
        it_ui_set_page(PAGE_PATTERN);
    }
}
