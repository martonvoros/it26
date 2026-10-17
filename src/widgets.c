#include "widgets.h"
#include "ui.h"

bool it_widget_button(int x, int y, int w, int h, const char* label, bool primary) {
    ITUIState* ui = it_ui_get_state();
    bool hovered = (ui->mouse_x >= x && ui->mouse_x < x + w &&
                    ui->mouse_y >= y && ui->mouse_y < y + h);
    bool pressed = hovered && ui->mouse_down;
    bool clicked = hovered && ui->mouse_clicked && !ui->click_consumed;

    if (clicked) {
        ui->click_consumed = true;
    }

    ITColor bg, hi_col, lo_col, text_col;

    if (primary) {
        if (pressed) {
            bg = (ITColor){ 18, 30, 44, 255 };
            hi_col = (ITColor){ 12, 20, 30, 255 };
            lo_col = (ITColor){ 60, 130, 170, 255 };
        } else if (hovered) {
            bg = (ITColor){ 34, 52, 74, 255 };
            hi_col = (ITColor){ 80, 160, 210, 255 };
            lo_col = (ITColor){ 12, 20, 30, 255 };
        } else {
            bg = (ITColor){ 28, 44, 62, 255 };
            hi_col = (ITColor){ 60, 130, 170, 255 };
            lo_col = (ITColor){ 12, 20, 30, 255 };
        }
        text_col = hovered ? IT_COL_YELLOW : IT_COL_CYAN;
    } else {
        if (pressed) {
            bg = (ITColor){ 22, 26, 34, 255 };
            hi_col = (ITColor){ 12, 16, 22, 255 };
            lo_col = (ITColor){ 75, 90, 116, 255 };
        } else if (hovered) {
            bg = (ITColor){ 46, 56, 72, 255 };
            hi_col = (ITColor){ 110, 130, 165, 255 };
            lo_col = (ITColor){ 12, 16, 22, 255 };
        } else {
            bg = (ITColor){ 36, 42, 54, 255 };
            hi_col = (ITColor){ 75, 90, 116, 255 };
            lo_col = (ITColor){ 12, 16, 22, 255 };
        }
        text_col = hovered ? IT_COL_YELLOW : (ITColor){ 220, 225, 235, 255 };
    }

    /* Base Fill */
    it_gfx_fill_rect(x, y, w, h, bg);

    /* 1-Pixel Crisp DOS Bevel */
    it_gfx_draw_line(x, y, x + w - 1, y, hi_col);
    it_gfx_draw_line(x, y, x, y + h - 1, hi_col);
    it_gfx_draw_line(x + w - 1, y, x + w - 1, y + h - 1, lo_col);
    it_gfx_draw_line(x, y + h - 1, x + w - 1, y + h - 1, lo_col);

    if (label) {
        int tw = it_gfx_text_width(label);
        int tx = x + (w - tw) / 2 + (pressed ? 1 : 0);
        int ty = y + (h - it_gfx_font_height()) / 2 + (pressed ? 1 : 0);
        if (tx < x + 3) tx = x + 3;
        it_gfx_draw_text(tx, ty, label, text_col);
    }

    return clicked;
}

bool it_widget_tab(int x, int y, int w, int h, const char* label, bool selected) {
    ITUIState* ui = it_ui_get_state();
    bool hovered = (ui->mouse_x >= x && ui->mouse_x < x + w &&
                    ui->mouse_y >= y && ui->mouse_y < y + h);
    bool clicked = hovered && ui->mouse_clicked && !ui->click_consumed;

    if (clicked) {
        ui->click_consumed = true;
    }

    ITColor bg = selected ? (ITColor){ 34, 42, 56, 255 } : (hovered ? (ITColor){ 26, 32, 44, 255 } : (ITColor){ 18, 22, 30, 255 });
    it_gfx_fill_rect(x, y, w, h, bg);

    /* Top and sides bevel */
    ITColor hi = selected ? IT_COL_CYAN : (hovered ? (ITColor){ 90, 110, 140, 255 } : (ITColor){ 50, 60, 80, 255 });
    it_gfx_draw_line(x, y, x + w - 1, y, hi);
    it_gfx_draw_line(x, y, x, y + h - 1, hi);
    it_gfx_draw_line(x + w - 1, y, x + w - 1, y + h - 1, (ITColor){ 14, 18, 24, 255 });

    if (selected) {
        it_gfx_fill_rect(x + 1, y + h - 2, w - 2, 2, (ITColor){ 34, 42, 56, 255 }); /* Blend into panel below */
    } else {
        it_gfx_draw_line(x, y + h - 1, x + w - 1, y + h - 1, (ITColor){ 50, 60, 80, 255 });
    }

    ITColor text_col = selected ? IT_COL_YELLOW : (hovered ? IT_COL_WHITE : (ITColor){ 160, 175, 195, 255 });
    if (label) {
        int tw = it_gfx_text_width(label);
        int tx = x + (w - tw) / 2;
        int ty = y + (h - it_gfx_font_height()) / 2;
        if (tx < x + 3) tx = x + 3;
        it_gfx_draw_text(tx, ty, label, text_col);
    }

    return clicked;
}

bool it_widget_checkbox(int x, int y, const char* label, bool* value) {
    if (!value) return false;
    ITUIState* ui = it_ui_get_state();
    int size = 16;
    int tw = label ? it_gfx_text_width(label) : 0;
    int total_w = size + 8 + tw;
    int total_h = IT_MAX(size, it_gfx_font_height());

    bool hovered = (ui->mouse_x >= x && ui->mouse_x < x + total_w &&
                    ui->mouse_y >= y && ui->mouse_y < y + total_h);
    bool clicked = hovered && ui->mouse_clicked && !ui->click_consumed;

    if (clicked) {
        ui->click_consumed = true;
        *value = !(*value);
    }

    int box_y = y + (total_h - size) / 2;
    /* Inset DOS checkbox */
    it_gfx_fill_rect(x, box_y, size, size, (ITColor){ 14, 18, 24, 255 });
    it_gfx_draw_line(x, box_y, x + size - 1, box_y, (ITColor){ 10, 14, 18, 255 });
    it_gfx_draw_line(x, box_y, x, box_y + size - 1, (ITColor){ 10, 14, 18, 255 });
    it_gfx_draw_line(x + size - 1, box_y, x + size - 1, box_y + size - 1, (ITColor){ 60, 75, 100, 255 });
    it_gfx_draw_line(x, box_y + size - 1, x + size - 1, box_y + size - 1, (ITColor){ 60, 75, 100, 255 });

    if (*value) {
        /* Crisp DOS 'X' checkmark */
        it_gfx_draw_line(x + 3, box_y + 3, x + size - 4, box_y + size - 4, IT_COL_YELLOW);
        it_gfx_draw_line(x + 4, box_y + 3, x + size - 3, box_y + size - 4, IT_COL_YELLOW);
        it_gfx_draw_line(x + size - 4, box_y + 3, x + 3, box_y + size - 4, IT_COL_YELLOW);
        it_gfx_draw_line(x + size - 3, box_y + 3, x + 4, box_y + size - 4, IT_COL_YELLOW);
    }

    if (label) {
        it_gfx_draw_text(x + size + 8, y + (total_h - it_gfx_font_height()) / 2, label,
                         hovered ? IT_COL_YELLOW : (ITColor){ 200, 210, 225, 255 });
    }

    return clicked;
}

bool it_widget_slider(int x, int y, int w, int h, const char* label, int* value, int min_val, int max_val) {
    if (!value || max_val <= min_val) return false;
    ITUIState* ui = it_ui_get_state();

    int bar_y = y + h / 2 - 3;
    int bar_h = 6;

    bool hovered = (ui->mouse_x >= x && ui->mouse_x < x + w &&
                    ui->mouse_y >= y && ui->mouse_y < y + h);
    bool changed = false;

    int slider_id = (x << 16) | (y & 0xFFFF);
    if (slider_id == 0) slider_id = 1;

    /* Start drag on click */
    if (hovered && ui->mouse_clicked && !ui->click_consumed) {
        ui->click_consumed = true;
        ui->active_drag_id = slider_id;
    }

    /* Process drag while mouse is down */
    if (ui->active_drag_id == slider_id) {
        if (ui->mouse_down) {
            float frac = (float)(ui->mouse_x - x) / (float)w;
            frac = IT_CLAMP(frac, 0.0f, 1.0f);
            int new_val = min_val + (int)(frac * (float)(max_val - min_val) + 0.5f);
            if (new_val != *value) {
                *value = new_val;
                changed = true;
            }
        } else {
            ui->active_drag_id = 0;
        }
    }

    /* Mouse wheel adjust */
    if (hovered && ui->wheel_y != 0) {
        int step = (max_val - min_val) / 20;
        if (step < 1) step = 1;
        int new_val = *value + ui->wheel_y * step;
        new_val = IT_CLAMP(new_val, min_val, max_val);
        if (new_val != *value) {
            *value = new_val;
            changed = true;
        }
    }

    /* Track background */
    it_gfx_fill_rect(x, bar_y, w, bar_h, IT_COL_PANEL_DARK);
    it_gfx_draw_rect(x, bar_y, w, bar_h, IT_COL_PANEL_BORDER);

    /* Filled track */
    float frac = (float)(*value - min_val) / (float)(max_val - min_val);
    int fill_w = (int)(frac * (float)w);
    it_gfx_fill_rect(x, bar_y, fill_w, bar_h, IT_COL_CYAN);

    /* Thumb handle */
    int thumb_x = x + fill_w - 6;
    bool is_active = (ui->active_drag_id == slider_id) || hovered;
    it_gfx_fill_rect(thumb_x, y + 2, 12, h - 4, is_active ? IT_COL_WHITE : IT_COL_CYAN);
    it_gfx_draw_rect(thumb_x, y + 2, 12, h - 4, IT_COL_PANEL_BORDER);

    if (label) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s: %d", label, *value);
        it_gfx_draw_text(x + w + 10, y + (h - it_gfx_font_height()) / 2, buf, IT_COL_WHITE);
    }

    return changed;
}

bool it_widget_spinner(int x, int y, int w, int h, const char* label, int* value, int min_val, int max_val) {
    if (!value) return false;
    ITUIState* ui = it_ui_get_state();
    int btn_w = 24;
    int val_w = w - btn_w * 2;

    bool changed = false;
    if (it_widget_button(x, y, btn_w, h, "-", false)) {
        if (*value > min_val) { (*value)--; changed = true; }
    }

    /* Mouse wheel on spinner */
    if (ui->wheel_y != 0 && ui->mouse_x >= x && ui->mouse_x < x + w && ui->mouse_y >= y && ui->mouse_y < y + h) {
        if (ui->wheel_y > 0 && *value < max_val) { (*value)++; changed = true; }
        else if (ui->wheel_y < 0 && *value > min_val) { (*value)--; changed = true; }
    }

    char val_str[32];
    snprintf(val_str, sizeof(val_str), "%d", *value);
    it_gfx_fill_rect(x + btn_w, y, val_w, h, IT_COL_PANEL_DARK);
    it_gfx_draw_rect(x + btn_w, y, val_w, h, IT_COL_PANEL_BORDER);
    int tw = it_gfx_text_width(val_str);
    it_gfx_draw_text(x + btn_w + (val_w - tw) / 2, y + (h - it_gfx_font_height()) / 2, val_str, IT_COL_WHITE);

    if (it_widget_button(x + btn_w + val_w, y, btn_w, h, "+", false)) {
        if (*value < max_val) { (*value)++; changed = true; }
    }

    if (label) {
        it_gfx_draw_text(x + w + 10, y + (h - it_gfx_font_height()) / 2, label, IT_COL_GRAY);
    }

    return changed;
}

void it_widget_panel(int x, int y, int w, int h, const char* title) {
    it_gfx_fill_rect(x, y, w, h, IT_COL_PANEL);
    it_gfx_draw_rect(x, y, w, h, IT_COL_PANEL_BORDER);

    if (title) {
        int hdr_h = 24;
        it_gfx_fill_rect(x, y, w, hdr_h, IT_COL_PANEL_DARK);
        it_gfx_draw_line(x, y + hdr_h, x + w, y + hdr_h, IT_COL_PANEL_BORDER);
        it_gfx_draw_text_bold(x + 10, y + (hdr_h - it_gfx_font_height()) / 2, title, IT_COL_CYAN);
    }
}

void it_widget_scrollbar(int x, int y, int w, int h, int total_items, int visible_items, int* scroll_pos) {
    if (!scroll_pos || total_items <= visible_items) return;

    it_gfx_fill_rect(x, y, w, h, IT_COL_PANEL_DARK);
    it_gfx_draw_rect(x, y, w, h, IT_COL_PANEL_BORDER);

    int max_scroll = total_items - visible_items;
    float thumb_ratio = (float)visible_items / (float)total_items;
    int thumb_h = IT_MAX((int)(thumb_ratio * (float)h), 20);

    float pos_ratio = (float)*scroll_pos / (float)max_scroll;
    int thumb_y = y + (int)(pos_ratio * (float)(h - thumb_h));

    ITUIState* ui = it_ui_get_state();
    bool hovered = (ui->mouse_x >= x && ui->mouse_x < x + w &&
                    ui->mouse_y >= y && ui->mouse_y < y + h);

    int scrollbar_id = (x << 16) | (y & 0xFFFF);
    if (scrollbar_id == 0) scrollbar_id = 2;

    if (hovered && ui->mouse_clicked && !ui->click_consumed) {
        ui->click_consumed = true;
        ui->active_drag_id = scrollbar_id;
    }

    if (ui->active_drag_id == scrollbar_id) {
        if (ui->mouse_down) {
            float rel_y = (float)(ui->mouse_y - y - thumb_h / 2) / (float)(h - thumb_h);
            rel_y = IT_CLAMP(rel_y, 0.0f, 1.0f);
            *scroll_pos = (int)(rel_y * (float)max_scroll + 0.5f);
        } else {
            ui->active_drag_id = 0;
        }
    }

    bool is_active = (ui->active_drag_id == scrollbar_id) || hovered;
    it_gfx_fill_rect(x + 1, thumb_y, w - 2, thumb_h, is_active ? IT_COL_CYAN : IT_COL_GRAY);
}

bool it_widget_text_input(int x, int y, int w, int h, char* text, size_t max_len) {
    if (!text || max_len == 0) return false;

    ITUIState* ui = it_ui_get_state();
    bool hovered = (ui->mouse_x >= x && ui->mouse_x < x + w &&
                    ui->mouse_y >= y && ui->mouse_y < y + h);
    bool clicked = hovered && ui->mouse_clicked && !ui->click_consumed;
    if (clicked) {
        ui->click_consumed = true;
    }

    it_gfx_fill_rect(x, y, w, h, IT_COL_PANEL_DARK);
    it_gfx_draw_rect(x, y, w, h, hovered ? IT_COL_CYAN : IT_COL_PANEL_BORDER);

    int ty = y + (h - it_gfx_font_height()) / 2;
    it_gfx_draw_text(x + 6, ty, text, IT_COL_WHITE);

    return clicked;
}


