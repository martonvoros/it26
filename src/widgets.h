#ifndef IT_WIDGETS_H
#define IT_WIDGETS_H

#include "gfx.h"

typedef struct {
    int x, y, w, h;
    const char* label;
    bool hovered;
    bool active;
    bool primary;
} ITButton;

bool it_widget_button(int x, int y, int w, int h, const char* label, bool primary);
bool it_widget_tab(int x, int y, int w, int h, const char* label, bool selected);
bool it_widget_checkbox(int x, int y, const char* label, bool* value);
bool it_widget_slider(int x, int y, int w, int h, const char* label, int* value, int min_val, int max_val);
bool it_widget_spinner(int x, int y, int w, int h, const char* label, int* value, int min_val, int max_val);
void it_widget_panel(int x, int y, int w, int h, const char* title);
void it_widget_scrollbar(int x, int y, int w, int h, int total_items, int visible_items, int* scroll_pos);
bool it_widget_text_input(int x, int y, int w, int h, char* text, size_t max_len);

#endif /* IT_WIDGETS_H */
