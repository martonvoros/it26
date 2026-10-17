#include "gfx.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern const unsigned char font_regular[];
extern const unsigned int font_regular_size;
extern const unsigned char font_bold[];
extern const unsigned int font_bold_size;

#define ATLAS_W 1024
#define ATLAS_H 1024
#define FONT_SIZE 15.0f

static SDL_Window* s_window = NULL;
static SDL_Renderer* s_renderer = NULL;
static float s_ui_scale = 1.15f;

static stbtt_bakedchar s_cdata_regular[96]; /* ASCII 32..127 */
static stbtt_bakedchar s_cdata_bold[96];
static SDL_Texture* s_tex_regular = NULL;
static SDL_Texture* s_tex_bold = NULL;
static SDL_Texture* s_bootscreen_tex = NULL;

static int s_char_width = 9;
static int s_char_height = 17;

static SDL_Texture* create_font_texture(const unsigned char* font_data, stbtt_bakedchar* cdata) {
    uint8_t* temp_bitmap = (uint8_t*)calloc(ATLAS_W * ATLAS_H, 1);
    if (!temp_bitmap) return NULL;

    stbtt_BakeFontBitmap(font_data, 0, FONT_SIZE, temp_bitmap, ATLAS_W, ATLAS_H, 32, 96, cdata);

    /* Convert A8 to RGBA32 (white with alpha) */
    uint32_t* rgba = (uint32_t*)malloc(ATLAS_W * ATLAS_H * sizeof(uint32_t));
    if (!rgba) {
        free(temp_bitmap);
        return NULL;
    }

    for (int i = 0; i < ATLAS_W * ATLAS_H; i++) {
        /* Crisp 1-bit bitmap threshold: ZERO anti-aliasing / smoothing for authentic sharp DOS typography */
        uint32_t a = (temp_bitmap[i] >= 85) ? 255 : 0;
        rgba[i] = (a << 24) | 0x00FFFFFF;
    }
    free(temp_bitmap);

    SDL_Texture* tex = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ATLAS_W, ATLAS_H);
    if (tex) {
        SDL_UpdateTexture(tex, NULL, rgba, ATLAS_W * sizeof(uint32_t));
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    }
    free(rgba);
    return tex;
}

bool it_gfx_init(SDL_Window* window, SDL_Renderer* renderer) {
    s_window = window;
    s_renderer = renderer;

    s_tex_regular = create_font_texture(font_regular, s_cdata_regular);
    s_tex_bold = create_font_texture(font_bold, s_cdata_bold);

    if (!s_tex_regular) {
        SDL_Log("Failed to create regular font texture");
        return false;
    }

    /* Monospace character pitch */
    s_char_width = (int)(s_cdata_regular['0' - 32].xadvance);
    if (s_char_width <= 0) s_char_width = 11;
    s_char_height = 20;

    return true;
}

void it_gfx_shutdown(void) {
    if (s_tex_regular) {
        SDL_DestroyTexture(s_tex_regular);
        s_tex_regular = NULL;
    }
    if (s_tex_bold) {
        SDL_DestroyTexture(s_tex_bold);
        s_tex_bold = NULL;
    }
    if (s_bootscreen_tex) {
        SDL_DestroyTexture(s_bootscreen_tex);
        s_bootscreen_tex = NULL;
    }
}

void it_gfx_set_scale(float scale) {
    if (scale >= 0.5f && scale <= 4.0f) {
        s_ui_scale = scale;
    }
}

float it_gfx_get_scale(void) {
    return s_ui_scale;
}

int it_gfx_get_width(void) {
    int w = 1280, h = 800;
    if (s_window) SDL_GetWindowSize(s_window, &w, &h);
    return w;
}

int it_gfx_get_height(void) {
    int w = 1280, h = 800;
    if (s_window) SDL_GetWindowSize(s_window, &w, &h);
    return h;
}

int it_gfx_get_logical_width(void) {
    return (int)((float)it_gfx_get_width() / s_ui_scale);
}

int it_gfx_get_logical_height(void) {
    return (int)((float)it_gfx_get_height() / s_ui_scale);
}

void it_gfx_begin_frame(void) {
    SDL_SetRenderDrawColor(s_renderer, IT_COL_BG.r, IT_COL_BG.g, IT_COL_BG.b, 255);
    SDL_RenderClear(s_renderer);
}

void it_gfx_end_frame(void) {
    SDL_RenderPresent(s_renderer);
}

void it_gfx_set_clip(int x, int y, int w, int h) {
    SDL_Rect r;
    r.x = (int)((float)x * s_ui_scale);
    r.y = (int)((float)y * s_ui_scale);
    r.w = (int)((float)w * s_ui_scale);
    r.h = (int)((float)h * s_ui_scale);
    SDL_SetRenderClipRect(s_renderer, &r);
}

void it_gfx_reset_clip(void) {
    SDL_SetRenderClipRect(s_renderer, NULL);
}

void it_gfx_fill_rect(int x, int y, int w, int h, ITColor col) {
    SDL_SetRenderDrawColor(s_renderer, col.r, col.g, col.b, col.a);
    SDL_FRect r;
    r.x = (float)x * s_ui_scale;
    r.y = (float)y * s_ui_scale;
    r.w = (float)w * s_ui_scale;
    r.h = (float)h * s_ui_scale;
    SDL_RenderFillRect(s_renderer, &r);
}

void it_gfx_draw_rect(int x, int y, int w, int h, ITColor col) {
    SDL_SetRenderDrawColor(s_renderer, col.r, col.g, col.b, col.a);
    SDL_FRect r;
    r.x = (float)x * s_ui_scale;
    r.y = (float)y * s_ui_scale;
    r.w = (float)w * s_ui_scale;
    r.h = (float)h * s_ui_scale;
    SDL_RenderRect(s_renderer, &r);
}

void it_gfx_draw_line(int x1, int y1, int x2, int y2, ITColor col) {
    SDL_SetRenderDrawColor(s_renderer, col.r, col.g, col.b, col.a);
    SDL_RenderLine(s_renderer, (float)x1 * s_ui_scale, (float)y1 * s_ui_scale,
                               (float)x2 * s_ui_scale, (float)y2 * s_ui_scale);
}

static uint32_t it_utf8_next(const unsigned char** p) {
    if (!p || !*p || !**p) return 0;
    unsigned char c = **p;
    (*p)++;
    if (c < 0x80) {
        return c;
    } else if ((c & 0xE0) == 0xC0) {
        if (**p && (**p & 0xC0) == 0x80) {
            unsigned char c2 = **p;
            (*p)++;
            return ((c & 0x1F) << 6) | (c2 & 0x3F);
        }
    } else if ((c & 0xF0) == 0xE0) {
        if (**p && (**p & 0xC0) == 0x80) {
            unsigned char c2 = **p;
            (*p)++;
            if (**p && (**p & 0xC0) == 0x80) {
                unsigned char c3 = **p;
                (*p)++;
                return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            }
        }
    } else if ((c & 0xF8) == 0xF0) {
        if (**p && (**p & 0xC0) == 0x80) {
            unsigned char c2 = **p;
            (*p)++;
            if (**p && (**p & 0xC0) == 0x80) {
                unsigned char c3 = **p;
                (*p)++;
                if (**p && (**p & 0xC0) == 0x80) {
                    unsigned char c4 = **p;
                    (*p)++;
                    return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
                }
            }
        }
    }
    return ' ';
}

static void draw_text_internal(int x, int y, const char* text, ITColor col, SDL_Texture* tex, stbtt_bakedchar* cdata) {
    if (!text || !tex) return;

    SDL_SetTextureColorMod(tex, col.r, col.g, col.b);
    SDL_SetTextureAlphaMod(tex, col.a);

    float cx = (float)x;
    float cy = (float)y + 13.0f; /* Baseline offset for 15.0f font */
    const unsigned char* p = (const unsigned char*)text;

    while (*p) {
        if (*p == '\n') {
            cy += (float)s_char_height;
            cx = (float)x;
            p++;
            continue;
        }

        uint32_t cp = it_utf8_next(&p);
        if (cp == 0) break;

        /* Play triangle: ▶ (0x25B6) or ► (0x25BA) */
        if (cp == 0x25B6 || cp == 0x25BA) {
            int sz = 8;
            int px = (int)cx + 1;
            int py = y + (s_char_height - sz) / 2;
            for (int i = 0; i < sz; i++) {
                int h = (i * sz) / (sz * 2);
                it_gfx_draw_line(px + i, py + (sz / 2) - h, px + i, py + (sz / 2) + h, col);
            }
            cx += (float)s_char_width;
            continue;
        }

        /* Stop square: ■ (0x25A0) */
        if (cp == 0x25A0) {
            int sz = 7;
            int px = (int)cx + 1;
            int py = y + (s_char_height - sz) / 2;
            it_gfx_fill_rect(px, py, sz, sz, col);
            cx += (float)s_char_width;
            continue;
        }

        /* Circle / Dot: ● (0x25CF) */
        if (cp == 0x25CF) {
            int r = 2;
            int pcx = (int)cx + s_char_width / 2;
            int pcy = y + s_char_height / 2;
            it_gfx_fill_rect(pcx - r, pcy - r, r * 2 + 1, r * 2 + 1, col);
            cx += (float)s_char_width;
            continue;
        }

        /* Lightning / Bolt: ⚡ (0x26A1) */
        if (cp == 0x26A1) {
            int px = (int)cx + 2;
            int py = y + 4;
            it_gfx_draw_line(px + 4, py, px + 1, py + 5, col);
            it_gfx_draw_line(px + 1, py + 5, px + 5, py + 5, col);
            it_gfx_draw_line(px + 5, py + 5, px + 2, py + 11, col);
            cx += (float)s_char_width;
            continue;
        }

        /* Clean mapping for Latin accented characters */
        if (cp >= 0xC0 && cp <= 0xC5) cp = 'A';
        else if (cp >= 0xE0 && cp <= 0xE5) cp = 'a';
        else if (cp >= 0xC8 && cp <= 0xCB) cp = 'E';
        else if (cp >= 0xE8 && cp <= 0xEB) cp = 'e';
        else if (cp >= 0xCC && cp <= 0xCF) cp = 'I';
        else if (cp >= 0xEC && cp <= 0xEF) cp = 'i';
        else if (cp >= 0xD2 && cp <= 0xD6) cp = 'O';
        else if (cp >= 0xF2 && cp <= 0xF6) cp = 'o';
        else if (cp >= 0xD9 && cp <= 0xDC) cp = 'U';
        else if (cp >= 0xF9 && cp <= 0xFC) cp = 'u';

        if (cp < 32 || cp > 126) cp = ' '; /* Clean space instead of ugly '?' */

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(cdata, ATLAS_W, ATLAS_H, cp - 32, &cx, &cy, &q, 1);

        SDL_FRect src, dst;
        src.x = q.s0 * ATLAS_W;
        src.y = q.t0 * ATLAS_H;
        src.w = (q.s1 - q.s0) * ATLAS_W;
        src.h = (q.t1 - q.t0) * ATLAS_H;

        dst.x = q.x0 * s_ui_scale;
        dst.y = q.y0 * s_ui_scale;
        dst.w = (q.x1 - q.x0) * s_ui_scale;
        dst.h = (q.y1 - q.y0) * s_ui_scale;

        SDL_RenderTexture(s_renderer, tex, &src, &dst);
    }
}

void it_gfx_draw_text(int x, int y, const char* text, ITColor col) {
    draw_text_internal(x, y, text, col, s_tex_regular, s_cdata_regular);
}

void it_gfx_draw_text_bold(int x, int y, const char* text, ITColor col) {
    draw_text_internal(x, y, text, col, s_tex_bold ? s_tex_bold : s_tex_regular,
                                        s_tex_bold ? s_cdata_bold : s_cdata_regular);
}

int it_gfx_text_width(const char* text) {
    if (!text) return 0;
    int len = 0;
    const unsigned char* p = (const unsigned char*)text;
    while (*p && *p != '\n') {
        it_utf8_next(&p);
        len++;
    }
    return len * s_char_width;
}

int it_gfx_font_height(void) {
    return s_char_height;
}

void it_gfx_screen_to_logical(int sx, int sy, int* lx, int* ly) {
    if (lx) *lx = (int)((float)sx / s_ui_scale);
    if (ly) *ly = (int)((float)sy / s_ui_scale);
}

void it_gfx_set_ui_scale(float scale) {
    if (scale >= 0.5f && scale <= 4.0f) {
        s_ui_scale = scale;
    }
}

float it_gfx_get_ui_scale(void) {
    return s_ui_scale;
}

void it_gfx_toggle_fullscreen(void) {
    if (!s_window) return;
    Uint32 flags = SDL_GetWindowFlags(s_window);
    if (flags & SDL_WINDOW_FULLSCREEN) {
        SDL_SetWindowFullscreen(s_window, false);
    } else {
        SDL_SetWindowFullscreen(s_window, true);
    }
}

void it_gfx_handle_resize(void) {
    /* Layout coordinates scale automatically via it_gfx_get_logical_width/height */
}

bool it_gfx_load_bootscreen(const char* filepath) {
    if (!filepath || !s_renderer) return false;
    if (s_bootscreen_tex) {
        SDL_DestroyTexture(s_bootscreen_tex);
        s_bootscreen_tex = NULL;
    }

    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(filepath, &w, &h, &channels, 4);
    if (!data) return false;

    SDL_Texture* tex = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, w, h);
    if (tex) {
        SDL_UpdateTexture(tex, NULL, data, w * 4);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        s_bootscreen_tex = tex;
    }
    stbi_image_free(data);
    return (s_bootscreen_tex != NULL);
}

void it_gfx_draw_bootscreen(int x, int y, int w, int h) {
    if (!s_bootscreen_tex || !s_renderer) return;
    SDL_FRect dst;
    dst.x = (float)x * s_ui_scale;
    dst.y = (float)y * s_ui_scale;
    dst.w = (float)w * s_ui_scale;
    dst.h = (float)h * s_ui_scale;
    SDL_RenderTexture(s_renderer, s_bootscreen_tex, NULL, &dst);
}


