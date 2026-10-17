#include "dialog.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"

static bool s_active = false;
static bool s_is_confirm = false;
static char s_title[64];
static char s_message[256];
static ITDialogCallback s_callback = NULL;
static void* s_userdata = NULL;

void it_dialog_alert(const char* title, const char* message) {
    s_active = true;
    s_is_confirm = false;
    strncpy(s_title, title ? title : "Notice", sizeof(s_title) - 1);
    strncpy(s_message, message ? message : "", sizeof(s_message) - 1);
    s_callback = NULL;
    s_userdata = NULL;
}

void it_dialog_confirm(const char* title, const char* message, ITDialogCallback cb, void* userdata) {
    s_active = true;
    s_is_confirm = true;
    strncpy(s_title, title ? title : "Confirm", sizeof(s_title) - 1);
    strncpy(s_message, message ? message : "", sizeof(s_message) - 1);
    s_callback = cb;
    s_userdata = userdata;
}

bool it_dialog_is_active(void) {
    return s_active;
}

void it_dialog_close(bool confirmed) {
    if (!s_active) return;
    s_active = false;
    if (s_callback) s_callback(confirmed, s_userdata);
}

void it_dialog_render(void) {
    if (!s_active) return;

    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();

    /* Dim background */
    it_gfx_fill_rect(0, 0, screen_w, screen_h, (ITColor){ 0, 0, 0, 160 });

    int dw = 460;
    int dh = 180;
    int dx = (screen_w - dw) / 2;
    int dy = (screen_h - dh) / 2;

    it_widget_panel(dx, dy, dw, dh, s_title);
    it_gfx_draw_text(dx + 20, dy + 50, s_message, IT_COL_WHITE);

    if (s_is_confirm) {
        if (it_widget_button(dx + dw - 190, dy + dh - 40, 80, 26, "Cancel", false)) {
            s_active = false;
            if (s_callback) s_callback(false, s_userdata);
        }
        if (it_widget_button(dx + dw - 100, dy + dh - 40, 80, 26, "OK", true)) {
            s_active = false;
            if (s_callback) s_callback(true, s_userdata);
        }
    } else {
        if (it_widget_button(dx + (dw - 90) / 2, dy + dh - 40, 90, 26, "OK", true)) {
            s_active = false;
        }
    }
}
