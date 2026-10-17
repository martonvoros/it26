#include "page_orders.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "player.h"

static int s_selected_order = 0;
static int s_scroll_order = 0;

void it_page_orders_init(void) {
    s_selected_order = 0;
    s_scroll_order = 0;
}

void it_page_orders_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();
    ITUIState* ui = it_ui_get_state();
    ITSong* song = it_player_get_song();
    if (!song) return;

    int panel_y = 62;
    int panel_h = screen_h - panel_y - 28;
    it_widget_panel(10, panel_y, screen_w - 20, panel_h, "ORDER LIST & PLAYBACK SEQUENCE [F7]");

    /* Controls Header */
    int cur_order = it_player_get_order();
    char buf[128];
    snprintf(buf, sizeof(buf), "Active Order: %03d | Selected: %03d | Total Orders: %d",
             cur_order, s_selected_order, song->order_count);
    it_gfx_draw_text(24, panel_y + 30, buf, IT_COL_WHITE);

    if (it_widget_button(screen_w - 360, panel_y + 26, 160, 24, "Play from Selected", true)) {
        it_player_play_from_order(s_selected_order);
    }
    if (it_widget_button(screen_w - 190, panel_y + 26, 160, 24, "Jump to Pattern", false)) {
        if (s_selected_order < song->order_count) {
            int pat = song->orders[s_selected_order];
            if (pat < IT_MAX_PATTERNS) {
                it_ui_set_page(PAGE_PATTERN);
            }
        }
    }

    /* Grid of orders (16 columns across rows) */
    int grid_x = 24;
    int grid_y = panel_y + 60;
    int cols = 16;
    int cell_w = (screen_w - 60) / cols;
    int cell_h = 28;

    int playing_order = it_player_is_playing() ? it_player_get_order() : -1;

    for (int i = 0; i < 128; i++) {
        int r = i / cols;
        int c = i % cols;
        int cx = grid_x + c * cell_w;
        int cy = grid_y + r * cell_h;

        bool is_sel = (i == s_selected_order);
        bool is_play = (i == playing_order);

        ITColor bg = is_sel ? IT_COL_ROW_HILIGHT2 : (is_play ? (ITColor){ 40, 70, 30, 255 } : IT_COL_PANEL_DARK);
        it_gfx_fill_rect(cx, cy, cell_w - 4, cell_h - 4, bg);
        it_gfx_draw_rect(cx, cy, cell_w - 4, cell_h - 4, is_sel ? IT_COL_CYAN : IT_COL_PANEL_BORDER);

        uint8_t pat = (i < song->order_count) ? song->orders[i] : IT_ORDER_END;
        char pat_str[16];
        if (pat == IT_ORDER_END) strcpy(pat_str, "---");
        else if (pat == IT_ORDER_SKIP) strcpy(pat_str, "+++");
        else snprintf(pat_str, sizeof(pat_str), "%02d", pat);

        char ord_idx_str[8];
        snprintf(ord_idx_str, sizeof(ord_idx_str), "%03d", i);
        it_gfx_draw_text(cx + 4, cy + 1, ord_idx_str, IT_COL_DARK_GRAY);
        it_gfx_draw_text_bold(cx + 4, cy + 12, pat_str, (pat < 254) ? IT_COL_YELLOW : IT_COL_RED);

        /* Mouse click */
        if (ui->mouse_clicked && !ui->click_consumed &&
            ui->mouse_x >= cx && ui->mouse_x < cx + cell_w &&
            ui->mouse_y >= cy && ui->mouse_y < cy + cell_h) {
            s_selected_order = i;
            ui->click_consumed = true;
        }

        /* Double-click plays from order */
        if (ui->mouse_double_clicked && !ui->click_consumed &&
            ui->mouse_x >= cx && ui->mouse_x < cx + cell_w &&
            ui->mouse_y >= cy && ui->mouse_y < cy + cell_h) {
            s_selected_order = i;
            it_player_play_from_order(i);
            ui->click_consumed = true;
        }
    }

    /* Bottom modifier buttons */
    int bot_y = screen_h - 60;
    it_gfx_draw_text(24, bot_y + 4, "Selected Order Controls:", IT_COL_GRAY);
    if (it_widget_button(210, bot_y, 80, 24, "Pattern -", false)) {
        if (song->orders[s_selected_order] > 0 && song->orders[s_selected_order] < 254) {
            song->orders[s_selected_order]--;
        }
    }
    if (it_widget_button(296, bot_y, 80, 24, "Pattern +", false)) {
        if (song->orders[s_selected_order] < 253) {
            song->orders[s_selected_order]++;
        }
    }
    if (it_widget_button(382, bot_y, 110, 24, "Set End (---)", false)) {
        song->orders[s_selected_order] = IT_ORDER_END;
    }
    if (it_widget_button(498, bot_y, 110, 24, "Set Skip (+++)", false)) {
        song->orders[s_selected_order] = IT_ORDER_SKIP;
    }
}
