#ifndef IT_UI_H
#define IT_UI_H

#include "common.h"
#include "song.h"

typedef enum {
    PAGE_BOOTSCREEN = 0,
    PAGE_WIZARD,
    PAGE_HELP,
    PAGE_PATTERN,
    PAGE_ORDERS,
    PAGE_SAMPLES,
    PAGE_INSTRUMENTS,
    PAGE_INFO,
    PAGE_FILES,
    PAGE_SONGVARS,
    PAGE_SETTINGS,
    PAGE_MESSAGE,
    PAGE_COUNT
} ITPageId;

typedef struct {
    ITPageId current_page;
    bool edit_mode;
    int current_octave;      /* 0..9, default 4 or 5 */
    int current_instrument;  /* 1..99 */
    int edit_step;           /* 0..16, default 1 */

    /* Mouse state in logical coordinates */
    int mouse_x;
    int mouse_y;
    bool mouse_down;           /* Left button held */
    bool mouse_clicked;        /* Left button clicked this frame */
    bool mouse_released;       /* Left button released this frame */
    bool mouse_rdown;          /* Right button held */
    bool mouse_rclicked;       /* Right button clicked this frame */
    bool mouse_rreleased;      /* Right button released this frame */
    bool mouse_double_clicked; /* Double click this frame */
    int wheel_x;
    int wheel_y;
    bool click_consumed;       /* Set true when a widget or grid consumes click */

    /* Mouse drag and timing tracking */
    int mouse_down_x;
    int mouse_down_y;
    uint32_t last_click_time;
    int last_click_x;
    int last_click_y;
    int active_drag_id;        /* Active dragging slider or scrollbar ID */

    /* Status message bar at bottom */
    char status_msg[128];
    uint32_t status_timer;

    bool request_quit;
} ITUIState;

bool it_ui_init(void);
void it_ui_shutdown(void);

void it_ui_handle_event(const SDL_Event* e);
void it_ui_update(void);
void it_ui_render(void);

void it_ui_set_page(ITPageId page);
ITPageId it_ui_get_page(void);

ITUIState* it_ui_get_state(void);
void it_ui_set_status(const char* fmt, ...);

#endif /* IT_UI_H */
