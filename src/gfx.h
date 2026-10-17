#ifndef IT_GFX_H
#define IT_GFX_H

#include "common.h"

typedef struct {
    uint8_t r, g, b, a;
} ITColor;

/* Predefined IT Theme Colors */
#define IT_COL_BG           ((ITColor){ 12,  18,  32, 255 })
#define IT_COL_PANEL        ((ITColor){ 18,  26,  44, 255 })
#define IT_COL_PANEL_DARK   ((ITColor){ 10,  14,  24, 255 })
#define IT_COL_PANEL_BORDER ((ITColor){ 36,  52,  84, 255 })
#define IT_COL_CYAN         ((ITColor){  0, 229, 255, 255 })
#define IT_COL_CYAN_DIM     ((ITColor){  0, 140, 160, 255 })
#define IT_COL_YELLOW       ((ITColor){ 255, 213,  79, 255 })
#define IT_COL_PANEL_BG     ((ITColor){ 12,  18,  32, 255 })
#define IT_COL_ORANGE       ((ITColor){ 255, 109,   0, 255 })
#define IT_COL_AMBER        ((ITColor){ 255, 179,   0, 255 })
#define IT_COL_GREEN        ((ITColor){   0, 230, 118, 255 })
#define IT_COL_RED          ((ITColor){ 255,  82,  82, 255 })
#define IT_COL_WHITE        ((ITColor){ 255, 255, 255, 255 })
#define IT_COL_GRAY         ((ITColor){ 144, 164, 174, 255 })
#define IT_COL_DARK_GRAY    ((ITColor){  69,  90, 100, 255 })
#define IT_COL_SELECTION    ((ITColor){  30, 136, 229, 120 })
#define IT_COL_ROW_HILIGHT1 ((ITColor){  24,  36,  60, 255 })
#define IT_COL_ROW_HILIGHT2 ((ITColor){  32,  48,  80, 255 })

bool it_gfx_init(SDL_Window* window, SDL_Renderer* renderer);
void it_gfx_shutdown(void);

void it_gfx_begin_frame(void);
void it_gfx_end_frame(void);

void it_gfx_set_scale(float scale);
float it_gfx_get_scale(void);

int it_gfx_get_width(void);
int it_gfx_get_height(void);
int it_gfx_get_logical_width(void);
int it_gfx_get_logical_height(void);

void it_gfx_set_clip(int x, int y, int w, int h);
void it_gfx_reset_clip(void);

void it_gfx_fill_rect(int x, int y, int w, int h, ITColor col);
void it_gfx_draw_rect(int x, int y, int w, int h, ITColor col);
void it_gfx_draw_line(int x1, int y1, int x2, int y2, ITColor col);

void it_gfx_draw_text(int x, int y, const char* text, ITColor col);
void it_gfx_draw_text_bold(int x, int y, const char* text, ITColor col);
int  it_gfx_text_width(const char* text);
int  it_gfx_font_height(void);
void it_gfx_screen_to_logical(int sx, int sy, int* lx, int* ly);
void it_gfx_set_ui_scale(float scale);
float it_gfx_get_ui_scale(void);
void it_gfx_toggle_fullscreen(void);
void it_gfx_handle_resize(void);

bool it_gfx_load_bootscreen(const char* filepath);
void it_gfx_draw_bootscreen(int x, int y, int w, int h);

#endif /* IT_GFX_H */
