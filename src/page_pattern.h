#ifndef IT_PAGE_PATTERN_H
#define IT_PAGE_PATTERN_H

#include "common.h"

void it_page_pattern_init(void);
void it_page_pattern_handle_key(SDL_Keycode key, uint16_t mod);
void it_page_pattern_render(void);

int it_page_pattern_get_cursor_channel(void);
int it_page_pattern_get_cursor_row(void);

#endif /* IT_PAGE_PATTERN_H */
