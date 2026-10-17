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

void it_page_help_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();

    /* Tabs */
    int tab_x = 10;
    int tab_y = 62;
    int tab_w = 110;
    int tab_h = 28;

    int num_sections = it_help_get_section_count();
    for (int i = 0; i < num_sections; i++) {
        const ITHelpSection* sec = it_help_get_section(i);
        if (it_widget_tab(tab_x + i * (tab_w + 4), tab_y, tab_w, tab_h, sec->tab_title, s_active_tab == i)) {
            s_active_tab = i;
            s_scroll_y = 0;
        }
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

        /* Mouse Wheel Scroll in help */
        ITUIState* ui = it_ui_get_state();
        if (ui->wheel_y != 0) {
            s_scroll_y -= ui->wheel_y * 30;
            if (s_scroll_y < 0) s_scroll_y = 0;
            if (s_scroll_y > 400) s_scroll_y = 400;
        }
    }

    /* Bottom Footer Bar */
    int footer_y = screen_h - 40;
    it_gfx_fill_rect(0, footer_y, screen_w, 40, IT_COL_PANEL_DARK);

    ITConfig* cfg = it_config_get();
    it_widget_checkbox(20, footer_y + 10, "Show this Help on Startup", &cfg->show_help_on_startup);

    /* Load Official Demo Song button */
    if (it_widget_button(340, footer_y + 6, 260, 28, "Load Official Demo (TUTE.IT)", false)) {
        ITSong* song = it_player_get_song();
        if (song) {
            if (it_load_file(song, "reference/impulse-tracker/Tutorial/TUTE.IT")) {
                it_ui_set_status("Loaded Jeffrey Lim's official tutorial module (TUTE.IT)!");
                it_ui_set_page(PAGE_PATTERN);
                it_player_play_song();
            }
        }
    }

    if (it_widget_button(screen_w - 240, footer_y + 6, 220, 28, "Start Tracking Now [F2]", true)) {
        it_ui_set_page(PAGE_PATTERN);
    }
}
