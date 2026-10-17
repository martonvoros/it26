#include "ui.h"
#include "gfx.h"
#include "widgets.h"
#include "dialog.h"
#include "player.h"
#include "midi.h"
#include "midi_learn.h"
#include "midi_profiles.h"
#include "config.h"

#include "page_help.h"
#include "page_pattern.h"
#include "page_orders.h"
#include "page_samples.h"
#include "page_instruments.h"
#include "page_info.h"
#include "page_files.h"
#include "page_songvars.h"
#include "page_settings.h"
#include "page_message.h"
#include "page_wizard.h"

#include <stdio.h>
#include <stdarg.h>
#include <math.h>

static void render_header(void);
static void render_menu_bar(void);
static void render_status_bar(void);

static ITUIState s_ui_state;
static uint32_t s_bootscreen_start = 0;

bool it_ui_init(void) {
    memset(&s_ui_state, 0, sizeof(s_ui_state));
    s_ui_state.current_page = PAGE_BOOTSCREEN;
    s_bootscreen_start = SDL_GetTicks();
    s_ui_state.edit_mode = true;
    s_ui_state.current_octave = 4;
    s_ui_state.current_instrument = 1;
    s_ui_state.edit_step = 1;
    s_ui_state.request_quit = false;

    it_page_wizard_init();
    it_page_help_init();
    it_page_pattern_init();
    it_page_orders_init();
    it_page_samples_init();
    it_page_instruments_init();
    it_page_info_init();
    it_page_files_init();
    it_page_songvars_init();
    it_page_settings_init();
    it_page_message_init();

    it_ui_set_status("IT26 ready. 655+ MIDI controllers loaded.");
    return true;
}

void it_ui_shutdown(void) {
}

ITUIState* it_ui_get_state(void) {
    return &s_ui_state;
}

void it_ui_set_page(ITPageId page) {
    if (page >= 0 && page < PAGE_COUNT) {
        s_ui_state.current_page = page;
    }
}

ITPageId it_ui_get_page(void) {
    return s_ui_state.current_page;
}

void it_ui_set_status(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(s_ui_state.status_msg, sizeof(s_ui_state.status_msg), fmt, args);
    va_end(args);
    s_ui_state.status_timer = SDL_GetTicks() + 4000;
}

void it_ui_handle_event(const SDL_Event* e) {
    if (!e) return;

    if (e->type == SDL_EVENT_QUIT) {
        s_ui_state.request_quit = true;
        return;
    }

    /* Any user interaction during bootscreen transitions straight to tracker/help */
    if (s_ui_state.current_page == PAGE_BOOTSCREEN) {
        if (e->type == SDL_EVENT_KEY_DOWN || e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            ITConfig* cfg = it_config_get();
            it_ui_set_page(cfg->show_help_on_startup ? PAGE_HELP : PAGE_PATTERN);
            return;
        }
    }

    if (e->type == SDL_EVENT_MOUSE_MOTION) {
        it_gfx_screen_to_logical((int)e->motion.x, (int)e->motion.y, &s_ui_state.mouse_x, &s_ui_state.mouse_y);
    } else if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        it_gfx_screen_to_logical((int)e->button.x, (int)e->button.y, &s_ui_state.mouse_x, &s_ui_state.mouse_y);
        if (e->button.button == SDL_BUTTON_LEFT) {
            s_ui_state.mouse_down = true;
            s_ui_state.mouse_clicked = true;
            s_ui_state.mouse_down_x = s_ui_state.mouse_x;
            s_ui_state.mouse_down_y = s_ui_state.mouse_y;

            uint32_t now = SDL_GetTicks();
            if (now - s_ui_state.last_click_time < 350 &&
                abs(s_ui_state.mouse_x - s_ui_state.last_click_x) < 8 &&
                abs(s_ui_state.mouse_y - s_ui_state.last_click_y) < 8) {
                s_ui_state.mouse_double_clicked = true;
            }
            s_ui_state.last_click_time = now;
            s_ui_state.last_click_x = s_ui_state.mouse_x;
            s_ui_state.last_click_y = s_ui_state.mouse_y;
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            s_ui_state.mouse_rdown = true;
            s_ui_state.mouse_rclicked = true;
        }
    } else if (e->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        it_gfx_screen_to_logical((int)e->button.x, (int)e->button.y, &s_ui_state.mouse_x, &s_ui_state.mouse_y);
        if (e->button.button == SDL_BUTTON_LEFT) {
            s_ui_state.mouse_down = false;
            s_ui_state.mouse_released = true;
            s_ui_state.active_drag_id = 0;
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            s_ui_state.mouse_rdown = false;
            s_ui_state.mouse_rreleased = true;
        }
    } else if (e->type == SDL_EVENT_MOUSE_WHEEL) {
        s_ui_state.wheel_x += (int)e->wheel.x;
        s_ui_state.wheel_y += (int)e->wheel.y;
    } else if (e->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode sym = e->key.key;
        uint16_t mod = (uint16_t)e->key.mod;

        /* Check modal dialog first */
        if (it_dialog_is_active()) {
            if (sym == SDLK_ESCAPE) it_dialog_close(false);
            if (sym == SDLK_RETURN) it_dialog_close(true);
            return;
        }

        /* If splash screen is active, any key skips directly to tracker */
        if (s_ui_state.current_page == PAGE_BOOTSCREEN) {
            ITConfig* cfg = it_config_get();
            it_ui_set_page(cfg->show_help_on_startup ? PAGE_WIZARD : PAGE_PATTERN);
            return;
        }

        /* Alt+Enter = Fullscreen */
        if (sym == SDLK_RETURN && (mod & SDL_KMOD_ALT)) {
            it_gfx_toggle_fullscreen();
            return;
        }

        /* Global Function Keys */
        if (sym == SDLK_F1) {
            it_ui_set_page(PAGE_HELP);
            return;
        } else if (sym == SDLK_F2) {
            it_ui_set_page(PAGE_PATTERN);
            return;
        } else if (sym == SDLK_F3) {
            it_ui_set_page(PAGE_SAMPLES);
            return;
        } else if (sym == SDLK_F4) {
            it_ui_set_page(PAGE_INSTRUMENTS);
            return;
        } else if (sym == SDLK_F5) {
            it_player_play_song();
            it_ui_set_status("Playing song from order 0 [F5]");
            return;
        } else if (sym == SDLK_F6) {
            it_player_play_pattern(it_player_get_pattern(), it_page_pattern_get_cursor_row());
            it_ui_set_status("Playing current pattern [F6]");
            return;
        } else if (sym == SDLK_F7) {
            it_ui_set_page(PAGE_ORDERS);
            return;
        } else if (sym == SDLK_F8) {
            it_player_stop();
            it_ui_set_status("Playback stopped [F8]");
            return;
        } else if (sym == SDLK_F9) {
            if (mod & SDL_KMOD_SHIFT) {
                it_ui_set_page(PAGE_MESSAGE);
            } else {
                it_ui_set_page(PAGE_INFO);
            }
            return;
        } else if (sym == SDLK_F10) {
            it_ui_set_page(PAGE_FILES);
            return;
        } else if (sym == SDLK_F11) {
            it_ui_set_page(PAGE_SETTINGS);
            return;
        } else if (sym == SDLK_F12) {
            it_ui_set_page(PAGE_SONGVARS);
            return;
        }

        /* Escape key behavior */
        if (sym == SDLK_ESCAPE) {
            if (it_midi_learn_is_learning()) {
                it_midi_learn_cancel();
                it_ui_set_status("MIDI Learn cancelled.");
                return;
            }
            if (s_ui_state.current_page != PAGE_PATTERN) {
                it_ui_set_page(PAGE_PATTERN);
                return;
            } else {
                it_player_stop();
                return;
            }
        }

        /* Octave shift */
        if (sym == SDLK_SLASH || sym == SDLK_COMMA || sym == SDLK_LESS) {
            if (s_ui_state.current_octave > 0) {
                s_ui_state.current_octave--;
                it_ui_set_status("Base Octave: %d", s_ui_state.current_octave);
            }
            return;
        } else if (sym == SDLK_ASTERISK || sym == SDLK_PERIOD || sym == SDLK_GREATER) {
            if (s_ui_state.current_octave < 9) {
                s_ui_state.current_octave++;
                it_ui_set_status("Base Octave: %d", s_ui_state.current_octave);
            }
            return;
        }

        /* Space toggles edit mode when on pattern page */
        if (sym == SDLK_SPACE && s_ui_state.current_page == PAGE_PATTERN) {
            s_ui_state.edit_mode = !s_ui_state.edit_mode;
            it_ui_set_status("Edit Mode: %s", s_ui_state.edit_mode ? "ON [Ready to enter notes]" : "OFF [Navigation only]");
            return;
        }

        /* Forward to current page handlers */
        if (s_ui_state.current_page == PAGE_PATTERN) {
            it_page_pattern_handle_key(sym, mod);
        } else if (s_ui_state.current_page == PAGE_WIZARD) {
            it_page_wizard_handle_key(sym, mod);
        }
    }
}

void it_ui_update(void) {
    /* Clear one-frame flags at end of frame */
    s_ui_state.mouse_clicked = false;
    s_ui_state.mouse_released = false;
    s_ui_state.mouse_rclicked = false;
    s_ui_state.mouse_rreleased = false;
    s_ui_state.mouse_double_clicked = false;
    s_ui_state.click_consumed = false;
    s_ui_state.wheel_x = 0;
    s_ui_state.wheel_y = 0;

    if (s_ui_state.status_timer > 0 && SDL_GetTicks() > s_ui_state.status_timer) {
        s_ui_state.status_msg[0] = '\0';
        s_ui_state.status_timer = 0;
    }
}

static void render_bootscreen(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();
    uint32_t now = SDL_GetTicks();
    uint32_t elapsed = now - s_bootscreen_start;
    ITUIState* ui = it_ui_get_state();

    /* Render underlying tracker page in background */
    render_header();
    render_menu_bar();
    it_page_pattern_render();
    render_status_bar();

    /* Translucent deep-dark vignette overlay over the whole screen */
    it_gfx_fill_rect(0, 0, screen_w, screen_h, (ITColor){ 4, 8, 16, 220 });

    /* Subtle CRT scanline overlay */
    for (int y = 0; y < screen_h; y += 4) {
        it_gfx_fill_rect(0, y, screen_w, 1, (ITColor){ 0, 0, 0, 35 });
    }

    /* Compact Modal Splash Window: 540 x 365 px ("pici splash képernyő") */
    int card_w = 540;
    int card_h = 365;
    if (card_w > screen_w - 40) card_w = screen_w - 40;
    if (card_h > screen_h - 40) card_h = screen_h - 40;
    int card_x = (screen_w - card_w) / 2;
    int card_y = (screen_h - card_h) / 2;

    /* Animated neon pulse glow around card */
    float pulse = 0.5f + 0.5f * sinf((float)elapsed * 0.005f);
    it_gfx_fill_rect(card_x - 5, card_y - 5, card_w + 10, card_h + 10, (ITColor){ 0, 180, 255, (uint8_t)(20 + 25 * pulse) });
    it_gfx_fill_rect(card_x - 2, card_y - 2, card_w + 4, card_h + 4, (ITColor){ 0, 220, 255, (uint8_t)(45 + 35 * pulse) });

    /* Card background panel */
    it_gfx_fill_rect(card_x, card_y, card_w, card_h, (ITColor){ 10, 15, 25, 255 });
    it_gfx_draw_rect(card_x, card_y, card_w, card_h, (ITColor){ 45, 65, 95, 255 });

    /* Neon corner brackets */
    int c_len = 16;
    ITColor corner_col = { (uint8_t)(40 + 40 * pulse), (uint8_t)(220 + 35 * pulse), 255, 255 };
    it_gfx_draw_line(card_x, card_y, card_x + c_len, card_y, corner_col);
    it_gfx_draw_line(card_x, card_y, card_x, card_y + c_len, corner_col);
    it_gfx_draw_line(card_x + card_w - c_len, card_y, card_x + card_w - 1, card_y, corner_col);
    it_gfx_draw_line(card_x + card_w - 1, card_y, card_x + card_w - 1, card_y + c_len, corner_col);
    it_gfx_draw_line(card_x, card_y + card_h - 1, card_x + c_len, card_y + card_h - 1, corner_col);
    it_gfx_draw_line(card_x, card_y + card_h - c_len, card_x, card_y + card_h - 1, corner_col);
    it_gfx_draw_line(card_x + card_w - c_len, card_y + card_h - 1, card_x + card_w - 1, card_y + card_h - 1, corner_col);
    it_gfx_draw_line(card_x + card_w - 1, card_y + card_h - c_len, card_x + card_w - 1, card_y + card_h - 1, corner_col);

    /* --- ANIMATED LOGO FRAME --- */
    int logo_x = card_x + 16;
    int logo_y = card_y + 16;
    int logo_w = card_w - 32;
    int logo_h = 168;

    /* Subtle breath zoom animation on logo */
    float logo_scale = 1.0f + 0.025f * sinf((float)elapsed * 0.0055f);
    int cx = logo_x + logo_w / 2;
    int cy = logo_y + logo_h / 2;
    int cur_w = (int)((float)logo_w * logo_scale);
    int cur_h = (int)((float)logo_h * logo_scale);
    int draw_x = cx - cur_w / 2;
    int draw_y = cy - cur_h / 2;

    it_gfx_fill_rect(logo_x, logo_y, logo_w, logo_h, (ITColor){ 4, 8, 14, 255 });
    it_gfx_set_clip(logo_x, logo_y, logo_w, logo_h);
    it_gfx_draw_bootscreen(draw_x, draw_y, cur_w, cur_h);

    /* Animated light shimmer sweep passing diagonally across logo */
    float sweep_t = fmodf((float)elapsed * 0.00065f, 1.0f);
    int beam_x = logo_x + (int)(sweep_t * (float)(logo_w + 140)) - 70;
    for (int dx = -24; dx <= 24; dx++) {
        int px = beam_x + dx;
        if (px >= logo_x && px < logo_x + logo_w) {
            int alpha = 55 - abs(dx) * 2;
            if (alpha > 0) {
                it_gfx_fill_rect(px, logo_y, 1, logo_h, (ITColor){ 130, 235, 255, (uint8_t)alpha });
            }
        }
    }

    /* Cybernetic laser scanner line sweeping vertically */
    float scan_t = 0.5f + 0.5f * sinf((float)elapsed * 0.0035f);
    int scan_y = logo_y + (int)(scan_t * (float)(logo_h - 4));
    it_gfx_fill_rect(logo_x, scan_y, logo_w, 2, (ITColor){ 0, 255, 240, 180 });
    it_gfx_fill_rect(logo_x, scan_y - 2, logo_w, 6, (ITColor){ 0, 230, 255, 50 });

    it_gfx_reset_clip();
    it_gfx_draw_rect(logo_x, logo_y, logo_w, logo_h, (ITColor){ 36, 54, 82, 255 });

    /* --- LOGO LABELS & CREDITS --- */
    int text_y = logo_y + logo_h + 12;
    it_gfx_draw_text_bold(card_x + 18, text_y, "IT26 - 64-BIT MODERN WORKSTATION", IT_COL_WHITE);
    it_gfx_draw_text_bold(card_x + 18, text_y + 19, "Modern Mod & Port created by Marton Voros aka C41", IT_COL_YELLOW);
    it_gfx_draw_text(card_x + 18, text_y + 38, "Original Impulse Tracker (C) Jeffrey Lim (1995-2024)", IT_COL_CYAN_DIM);

    /* --- REAL-TIME AUDIO EQUALIZER BARS (Syncing with 138 BPM Trance) --- */
    int eq_x = card_x + card_w - 150;
    int eq_y = text_y + 2;
    int eq_h = 42;
    for (int b = 0; b < 13; b++) {
        const ITChannelVis* vis = it_mixer_get_channel_vis(b);
        float vu = vis ? vis->vu_level : 0.0f;
        if (vu < 0.05f) {
            vu = 0.18f + 0.15f * sinf((float)elapsed * 0.009f + (float)b * 0.48f);
        }
        int bar_h = (int)(IT_CLAMP(vu, 0.0f, 1.0f) * (float)eq_h);
        int bx = eq_x + b * 10;
        int by = eq_y + eq_h - bar_h;
        it_gfx_fill_rect(bx, by, 7, bar_h, (vu > 0.65f) ? IT_COL_YELLOW : IT_COL_CYAN);
    }

    /* --- ANIMATED PROGRESS BAR --- */
    int bar_y = card_y + card_h - 70;
    int bar_w = card_w - 36;
    int bar_x = card_x + 18;
    it_gfx_fill_rect(bar_x, bar_y, bar_w, 6, (ITColor){ 16, 24, 36, 255 });
    it_gfx_draw_rect(bar_x, bar_y, bar_w, 6, (ITColor){ 38, 54, 80, 255 });

    float prog = (float)elapsed / 2200.0f;
    if (prog > 1.0f) prog = 1.0f;
    int fill_w = (int)(prog * (float)bar_w);
    if (fill_w > 0) {
        it_gfx_fill_rect(bar_x, bar_y, fill_w, 6, IT_COL_CYAN);
        if (fill_w < bar_w) {
            it_gfx_fill_rect(bar_x + fill_w - 3, bar_y, 3, 6, IT_COL_WHITE);
        }
    }

    /* --- STATUS TICKER & ENTER BUTTON --- */
    const char* boot_msg = "IT26 INITIALIZING 32-BIT SVF TRANCE ENGINE... [OK]";
    if (elapsed > 400 && elapsed <= 1000) {
        boot_msg = "LOADING 655+ HARDWARE MIDI CONTROLLER PROFILES... [OK]";
    } else if (elapsed > 1000 && elapsed <= 1700) {
        boot_msg = "06. TRANCE AFTER TRANQUILITY [138 BPM]... [LIVE]";
    } else if (elapsed > 1700) {
        boot_msg = "READY. PRESS SPACE OR CLICK ANYWHERE TO START!";
    }

    int foot_y = bar_y + 14;
    it_gfx_draw_text_bold(bar_x, foot_y + 4, boot_msg, (elapsed > 1700) ? IT_COL_YELLOW : IT_COL_WHITE);

    bool enter_clicked = it_widget_button(card_x + card_w - 170, foot_y, 152, 26, "Enter Tracker [Space]", true);
    bool screen_clicked = (ui->mouse_clicked && !ui->click_consumed && elapsed > 200);

    if (enter_clicked || screen_clicked || (elapsed > 2500)) {
        ITConfig* cfg = it_config_get();
        it_ui_set_page(cfg->show_help_on_startup ? PAGE_WIZARD : PAGE_PATTERN);
    }
}

static void render_header(void) {
    int screen_w = it_gfx_get_logical_width();
    ITSong* song = it_player_get_song();
    ITUIState* ui = it_ui_get_state();

    int hdr_h = 38; /* Ultra-compact, pixel-perfect 2-line header */

    /* Background panel */
    it_gfx_fill_rect(0, 0, screen_w, hdr_h, (ITColor){ 18, 24, 34, 255 });
    it_gfx_draw_line(0, hdr_h - 1, screen_w, hdr_h - 1, (ITColor){ 42, 52, 70, 255 });

    /* --- LINE 1 (y = 3): Song Title, Orders, Patterns, Speed, Tempo, Volumes --- */
    int y1 = 3;
    char title_buf[48];
    const char* sname = (song && strlen(song->name) > 0) ? song->name : "Trance After Tranquility";
    snprintf(title_buf, sizeof(title_buf), "Song: %-22.22s", sname);
    it_gfx_draw_text_bold(10, y1, title_buf, IT_COL_WHITE);

    int cur_ord = it_player_get_order();
    int max_orders = song ? song->order_count : 0;
    int cur_pat = it_player_get_pattern();
    int cur_row = it_player_get_row();

    char pos_buf[48];
    snprintf(pos_buf, sizeof(pos_buf), "Ord:%03d/%03d Pat:%02d Row:%02d", cur_ord, max_orders, cur_pat, cur_row);
    it_gfx_draw_text(350, y1, pos_buf, IT_COL_CYAN);

    char spd_buf[36];
    snprintf(spd_buf, sizeof(spd_buf), "Spd:%02d Tmp:%3d", it_player_get_speed(), it_player_get_tempo());
    it_gfx_draw_text_bold(640, y1, spd_buf, IT_COL_YELLOW);

    char vol_buf[36];
    snprintf(vol_buf, sizeof(vol_buf), "GVol:%3d MVol:%2d", (song ? song->global_vol : 128), (song ? song->mix_vol : 48));
    it_gfx_draw_text(795, y1, vol_buf, IT_COL_WHITE);

    int active_voices = 0;
    for (int v = 0; v < IT_MAX_VIRTUAL_CHANNELS; v++) {
        const ITVoice* voice = it_mixer_get_voice(v);
        if (voice && voice->active) active_voices++;
    }
    char v_buf[24];
    snprintf(v_buf, sizeof(v_buf), "Vce:%02d/64", active_voices);
    it_gfx_draw_text(965, y1, v_buf, IT_COL_GRAY);

    bool playing = it_player_is_playing();
    it_gfx_draw_text_bold(screen_w - 105, y1, playing ? "[PLAYING]" : "[STOPPED]", playing ? IT_COL_GREEN : IT_COL_GRAY);

    /* --- LINE 2 (y = 19): Instrument, Octave, Step, Mode, Master VU, MIDI --- */
    int y2 = 19;
    char inst_buf[48];
    int cur_ins = ui->current_instrument;
    char iname[26] = {0};
    if (song) {
        if ((song->flags & SONG_FLAG_INSTRUMENTS) && cur_ins >= 1 && cur_ins <= song->instrument_count) {
            if (strlen(song->instruments[cur_ins].name) > 0) {
                strncpy(iname, song->instruments[cur_ins].name, 25);
            } else if (cur_ins <= song->sample_count && strlen(song->samples[cur_ins].name) > 0) {
                strncpy(iname, song->samples[cur_ins].name, 25);
            }
        } else if (cur_ins >= 1 && cur_ins <= song->sample_count) {
            if (strlen(song->samples[cur_ins].name) > 0) {
                strncpy(iname, song->samples[cur_ins].name, 25);
            }
        }
    }
    if (strlen(iname) == 0) {
        snprintf(iname, sizeof(iname), "Sample %02d", cur_ins);
    }
    snprintf(inst_buf, sizeof(inst_buf), "Ins:%02d-%-16.16s", cur_ins, iname);
    it_gfx_draw_text_bold(10, y2, inst_buf, IT_COL_YELLOW);

    /* Clickable [-] and [+] for Instrument */
    if (it_widget_button(272, y2, 17, 16, "-", false)) {
        if (ui->current_instrument > 1) ui->current_instrument--;
    }
    if (it_widget_button(292, y2, 17, 16, "+", false)) {
        int max_i = song ? (song->instrument_count ? song->instrument_count : song->sample_count) : 99;
        if (ui->current_instrument < max_i) ui->current_instrument++;
    }

    /* Octave */
    char oct_buf[16];
    snprintf(oct_buf, sizeof(oct_buf), "Oct:%d", ui->current_octave);
    it_gfx_draw_text(324, y2, oct_buf, IT_COL_WHITE);
    if (it_widget_button(372, y2, 17, 16, "-", false)) {
        if (ui->current_octave > 0) ui->current_octave--;
    }
    if (it_widget_button(392, y2, 17, 16, "+", false)) {
        if (ui->current_octave < 9) ui->current_octave++;
    }

    /* Edit Step */
    char step_buf[16];
    snprintf(step_buf, sizeof(step_buf), "Step:%d", ui->edit_step);
    it_gfx_draw_text(424, y2, step_buf, IT_COL_CYAN);

    /* Mode Badge */
    it_gfx_draw_text_bold(500, y2, ui->edit_mode ? "[EDIT MODE]" : "[VIEW MODE]", ui->edit_mode ? IT_COL_GREEN : IT_COL_GRAY);

    /* Master Stereo VU Meter */
    float vu_l = 0.0f, vu_r = 0.0f;
    it_mixer_get_vu_levels(&vu_l, &vu_r);
    int vu_w = 90;
    int vu_x = 625;
    it_gfx_fill_rect(vu_x, y2 + 2, vu_w, 4, (ITColor){ 12, 16, 22, 255 });
    it_gfx_fill_rect(vu_x, y2 + 8, vu_w, 4, (ITColor){ 12, 16, 22, 255 });
    int fill_l = (int)(IT_CLAMP(vu_l, 0.0f, 1.0f) * (float)vu_w);
    int fill_r = (int)(IT_CLAMP(vu_r, 0.0f, 1.0f) * (float)vu_w);
    if (fill_l > 0) it_gfx_fill_rect(vu_x, y2 + 2, fill_l, 4, (fill_l > vu_w * 0.85f) ? IT_COL_RED : IT_COL_GREEN);
    if (fill_r > 0) it_gfx_fill_rect(vu_x, y2 + 8, fill_r, 4, (fill_r > vu_w * 0.85f) ? IT_COL_RED : IT_COL_GREEN);

    /* MIDI Status */
    ITMidiMessage last_msg;
    uint32_t age_ms = 999999;
    bool has_msg = it_midi_get_last_message(&last_msg, &age_ms);
    bool midi_active = (has_msg && age_ms < 300);
    it_gfx_draw_text_bold(735, y2, "MIDI:[● 655 Profiles]", midi_active ? IT_COL_GREEN : IT_COL_CYAN_DIM);
}

static void render_menu_bar(void) {
    int screen_w = it_gfx_get_logical_width();
    int bar_y = 38;
    int bar_h = 24;

    it_gfx_fill_rect(0, bar_y, screen_w, bar_h, (ITColor){ 24, 30, 42, 255 });
    it_gfx_draw_line(0, bar_y + bar_h - 1, screen_w, bar_y + bar_h - 1, (ITColor){ 45, 55, 75, 255 });

    int bx = 6;
    int by = bar_y + 1;
    int bh = 21;
    int gap = 2;

    int bw = (screen_w >= 1000) ? 58 : 50;
    int play_w = (screen_w >= 1000) ? 76 : 68;

    /* IT26 Brand Badge - Clicking opens About page */
    bool is_about_active = (s_ui_state.current_page == PAGE_HELP && it_page_help_get_tab() == 5);
    if (it_widget_button(bx, by, 42, bh, "IT26", is_about_active)) {
        it_page_help_set_tab(5);
        it_ui_set_page(PAGE_HELP);
    }
    bx += 42 + gap;

    /* MODERN WIZARD BUTTON - Eye-catching gold/cyan badge for modern users */
    if (it_widget_button(bx, by, 80, bh, "✦ WIZARD", s_ui_state.current_page == PAGE_WIZARD)) {
        it_ui_set_page(PAGE_WIZARD);
    }
    bx += 80 + gap + 2;

    /* Navigation buttons */
    if (it_widget_button(bx, by, bw, bh, "F1 Help", s_ui_state.current_page == PAGE_HELP)) it_ui_set_page(PAGE_HELP);
    bx += bw + gap;
    if (it_widget_button(bx, by, bw, bh, "F2 Pat", s_ui_state.current_page == PAGE_PATTERN)) it_ui_set_page(PAGE_PATTERN);
    bx += bw + gap;
    if (it_widget_button(bx, by, bw, bh, "F3 Smp", s_ui_state.current_page == PAGE_SAMPLES)) it_ui_set_page(PAGE_SAMPLES);
    bx += bw + gap;
    if (it_widget_button(bx, by, bw, bh, "F4 Ins", s_ui_state.current_page == PAGE_INSTRUMENTS)) it_ui_set_page(PAGE_INSTRUMENTS);
    bx += bw + gap + 3;

    /* Transport buttons with crisp vector icons */
    bool playing = it_player_is_playing();
    if (it_widget_button(bx, by, play_w, bh, "▶ Play F5", playing)) it_player_play_song();
    bx += play_w + gap;
    if (it_widget_button(bx, by, play_w, bh, "■ Stop F8", !playing)) it_player_stop();
    bx += play_w + gap + 3;

    if (it_widget_button(bx, by, bw, bh, "F7 Ord", s_ui_state.current_page == PAGE_ORDERS)) it_ui_set_page(PAGE_ORDERS);
    bx += bw + gap;
    if (it_widget_button(bx, by, bw, bh, "F9 Info", s_ui_state.current_page == PAGE_INFO)) it_ui_set_page(PAGE_INFO);
    bx += bw + gap;
    if (it_widget_button(bx, by, bw + 4, bh, "F10 Disk", s_ui_state.current_page == PAGE_FILES)) it_ui_set_page(PAGE_FILES);
    bx += bw + 4 + gap;
    if (it_widget_button(bx, by, bw + 4, bh, "F11 MIDI", s_ui_state.current_page == PAGE_SETTINGS)) it_ui_set_page(PAGE_SETTINGS);
    bx += bw + 4 + gap;
    if (it_widget_button(bx, by, bw, bh, "F12 Var", s_ui_state.current_page == PAGE_SONGVARS)) it_ui_set_page(PAGE_SONGVARS);

    /* Right side: Explicit About & License buttons */
    int r_x = screen_w - 74;
    bool is_license_active = (s_ui_state.current_page == PAGE_HELP && it_page_help_get_tab() == 6);
    if (it_widget_button(r_x, by, 68, bh, "License", is_license_active)) {
        it_page_help_set_tab(6);
        it_ui_set_page(PAGE_HELP);
    }
    r_x -= 62;
    if (it_widget_button(r_x, by, 58, bh, "About", is_about_active)) {
        it_page_help_set_tab(5);
        it_ui_set_page(PAGE_HELP);
    }
}

static void render_status_bar(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();
    int bar_y = screen_h - 26;

    it_gfx_fill_rect(0, bar_y, screen_w, 26, IT_COL_PANEL_DARK);
    it_gfx_draw_line(0, bar_y, screen_w, bar_y, IT_COL_PANEL_BORDER);

    /* Status message or shortcut hints */
    if (strlen(s_ui_state.status_msg) > 0) {
        it_gfx_draw_text_bold(12, bar_y + 5, s_ui_state.status_msg, IT_COL_YELLOW);
    } else {
        const char* hint = "F1:Help | F2:Pattern | F5:Play | F8:Stop | Space:Edit | F11:MIDI Hub (650+ Models & Remapping)";
        it_gfx_draw_text(12, bar_y + 5, hint, IT_COL_GRAY);
    }

    /* Edit mode badge */
    const char* edit_text = s_ui_state.edit_mode ? "[EDIT MODE]" : "[VIEW MODE]";
    it_gfx_draw_text_bold(screen_w - 230, bar_y + 5, edit_text, s_ui_state.edit_mode ? IT_COL_GREEN : IT_COL_GRAY);

    /* Master volume peak indicator */
    float vu_l = 0.0f, vu_r = 0.0f;
    it_mixer_get_vu_levels(&vu_l, &vu_r);
    int vu_w = 80;
    int vu_x = screen_w - 95;
    int vu_y = bar_y + 7;
    it_gfx_fill_rect(vu_x, vu_y, vu_w, 10, IT_COL_PANEL_DARK);

    int fill_l = (int)(vu_l * (float)vu_w);
    if (fill_l > vu_w) fill_l = vu_w;
    int fill_r = (int)(vu_r * (float)vu_w);
    if (fill_r > vu_w) fill_r = vu_w;

    it_gfx_fill_rect(vu_x, vu_y, fill_l, 4, (fill_l > vu_w * 0.85f) ? IT_COL_RED : IT_COL_GREEN);
    it_gfx_fill_rect(vu_x, vu_y + 6, fill_r, 4, (fill_r > vu_w * 0.85f) ? IT_COL_RED : IT_COL_GREEN);
}

void it_ui_render(void) {
    if (s_ui_state.current_page == PAGE_BOOTSCREEN) {
        render_bootscreen();
        return;
    }

    /* Classic Top Header & Dedicated Menu Bar */
    render_header();
    render_menu_bar();

    /* Active page */
    switch (s_ui_state.current_page) {
        case PAGE_WIZARD:
            it_page_wizard_render();
            break;
        case PAGE_HELP:
            it_page_help_render();
            break;
        case PAGE_PATTERN:
            it_page_pattern_render();
            break;
        case PAGE_ORDERS:
            it_page_orders_render();
            break;
        case PAGE_SAMPLES:
            it_page_samples_render();
            break;
        case PAGE_INSTRUMENTS:
            it_page_instruments_render();
            break;
        case PAGE_INFO:
            it_page_info_render();
            break;
        case PAGE_FILES:
            it_page_files_render();
            break;
        case PAGE_SONGVARS:
            it_page_songvars_render();
            break;
        case PAGE_SETTINGS:
            it_page_settings_render();
            break;
        case PAGE_MESSAGE:
            it_page_message_render();
            break;
        default:
            break;
    }

    /* Bottom status bar */
    render_status_bar();

    /* Active modal dialog overlay if open */
    if (it_dialog_is_active()) {
        it_dialog_render();
    }
}
