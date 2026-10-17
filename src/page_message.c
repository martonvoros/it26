#include "page_message.h"
#include "song.h"
#include "player.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"

static int s_scroll_y = 0;

void it_page_message_init(void) {
    s_scroll_y = 0;
}

void it_page_message_render(void) {
    ITSong* song = it_player_get_song();
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();

    /* Header */
    it_gfx_fill_rect(0, 0, screen_w, 36, IT_COL_PANEL_DARK);
    it_gfx_draw_text_bold(16, 8, "SONG MESSAGE / COMMENTS VIEWER [Shift+F9]", IT_COL_CYAN);

    if (it_widget_button(screen_w - 180, 5, 164, 26, "Back to Tracker [Esc]", true)) {
        it_ui_set_page(PAGE_PATTERN);
    }

    int panel_x = 16;
    int panel_y = 62;
    int panel_w = screen_w - 32;
    int panel_h = screen_h - 96;

    it_widget_panel(panel_x, panel_y, panel_w, panel_h, "Song Text / Author Notes");

    it_gfx_set_clip(panel_x + 12, panel_y + 24, panel_w - 24, panel_h - 36);

    if (song && song->message && strlen(song->message) > 0) {
        it_gfx_draw_text(panel_x + 16, panel_y + 30 - s_scroll_y, song->message, IT_COL_WHITE);
    } else {
        it_gfx_draw_text(panel_x + 20, panel_y + 40, "No song message attached to this module.", IT_COL_GRAY);
        it_gfx_draw_text(panel_x + 20, panel_y + 70, "Many classic modules (.IT, .XM, .S3M) contain release notes, sample credits,", IT_COL_PANEL_BORDER);
        it_gfx_draw_text(panel_x + 20, panel_y + 90, "and greetings from the tracker scene here.", IT_COL_PANEL_BORDER);

        if (it_widget_button(panel_x + 20, panel_y + 130, 260, 30, "Insert Default IT Greetings", true)) {
            if (song) {
                if (song->message) free(song->message);
                const char* default_msg =
                    "=========================================================================\n"
                    "               IMPULSE TRACKER MODERN WORKSTATION 2026\n"
                    "=========================================================================\n\n"
                    "Original tracker created by Jeffrey Lim (1995-1997).\n"
                    "Ported to modern multi-platform 64-bit systems with:\n"
                    "  - High-DPI scalable TrueType font rendering\n"
                    "  - 32-bit floating point stereo mixing & resonant State Variable Filters\n"
                    "  - Native WASAPI / SDL3 audio output\n"
                    "  - Over 650 pre-mapped modern USB MIDI controllers\n"
                    "  - Interactive MIDI Learn & mapping studio\n\n"
                    "Greetings to the global demoscene, modland, and all tracker musicians!\n";
                song->message = (char*)malloc(strlen(default_msg) + 1);
                if (song->message) strcpy(song->message, default_msg);
            }
        }
    }

    it_gfx_reset_clip();

    /* Mouse scroll */
    ITUIState* ui = it_ui_get_state();
    if (ui->wheel_y != 0) {
        s_scroll_y -= ui->wheel_y * 24;
        if (s_scroll_y < 0) s_scroll_y = 0;
    }

    /* Bottom toolbar */
    int foot_y = screen_h - 40;
    it_gfx_fill_rect(0, foot_y, screen_w, 40, IT_COL_PANEL_DARK);

    if (it_widget_button(20, foot_y + 6, 160, 28, "Clear Message", false)) {
        if (song && song->message) {
            free(song->message);
            song->message = NULL;
        }
    }
}
