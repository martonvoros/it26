#include "page_pattern.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "player.h"
#include "mixer.h"

static int s_cursor_row = 0;
static int s_cursor_ch = 0;
static int s_cursor_field = 0; /* 0=note, 1=inst, 2=vol, 3=eff, 4=param */
static int s_first_visible_ch = 0;
static bool s_follow_song = true;

void it_page_pattern_init(void) {
    s_cursor_row = 0;
    s_cursor_ch = 0;
    s_cursor_field = 0;
    s_first_visible_ch = 0;
    s_follow_song = true;
}

int it_page_pattern_get_cursor_channel(void) {
    return s_cursor_ch;
}

int it_page_pattern_get_cursor_row(void) {
    return s_cursor_row;
}

static int map_key_to_note(SDL_Keycode key, int base_octave) {
    int semitone = -1;
    int oct_offset = 0;

    switch (key) {
        /* Lower Octave */
        case SDLK_Z: semitone = 0;  break; /* C  */
        case SDLK_S: semitone = 1;  break; /* C# */
        case SDLK_X: semitone = 2;  break; /* D  */
        case SDLK_D: semitone = 3;  break; /* D# */
        case SDLK_C: semitone = 4;  break; /* E  */
        case SDLK_V: semitone = 5;  break; /* F  */
        case SDLK_G: semitone = 6;  break; /* F# */
        case SDLK_B: semitone = 7;  break; /* G  */
        case SDLK_H: semitone = 8;  break; /* G# */
        case SDLK_N: semitone = 9;  break; /* A  */
        case SDLK_J: semitone = 10; break; /* A# */
        case SDLK_M: semitone = 11; break; /* B  */

        /* Upper Octave */
        case SDLK_Q: semitone = 0;  oct_offset = 1; break;
        case SDLK_2: semitone = 1;  oct_offset = 1; break;
        case SDLK_W: semitone = 2;  oct_offset = 1; break;
        case SDLK_3: semitone = 3;  oct_offset = 1; break;
        case SDLK_E: semitone = 4;  oct_offset = 1; break;
        case SDLK_R: semitone = 5;  oct_offset = 1; break;
        case SDLK_5: semitone = 6;  oct_offset = 1; break;
        case SDLK_T: semitone = 7;  oct_offset = 1; break;
        case SDLK_6: semitone = 8;  oct_offset = 1; break;
        case SDLK_Y: semitone = 9;  oct_offset = 1; break;
        case SDLK_7: semitone = 10; oct_offset = 1; break;
        case SDLK_U: semitone = 11; oct_offset = 1; break;
        default: return IT_NOTE_NONE;
    }

    int oct = base_octave + oct_offset;
    if (oct < 0) oct = 0;
    if (oct > 9) oct = 9;

    return 1 + oct * 12 + semitone;
}

void it_page_pattern_handle_key(SDL_Keycode key, uint16_t mod) {
    ITUIState* ui = it_ui_get_state();
    ITSong* song = it_player_get_song();
    if (!song) return;

    int pat_idx = it_player_get_pattern();
    if (pat_idx < 0 || pat_idx >= IT_MAX_PATTERNS) return;
    ITPattern* pat = &song->patterns[pat_idx];
    if (!pat->data) return;

    switch (key) {
        case SDLK_UP:
            if (s_cursor_row > 0) s_cursor_row--;
            break;
        case SDLK_DOWN:
            if (s_cursor_row < pat->rows - 1) s_cursor_row++;
            break;
        case SDLK_LEFT:
            if (s_cursor_field > 0) s_cursor_field--;
            else if (s_cursor_ch > 0) {
                s_cursor_ch--;
                s_cursor_field = 4;
            }
            break;
        case SDLK_RIGHT:
            if (s_cursor_field < 4) s_cursor_field++;
            else if (s_cursor_ch < IT_MAX_CHANNELS - 1) {
                s_cursor_ch++;
                s_cursor_field = 0;
            }
            break;
        case SDLK_PAGEUP:
            s_cursor_row = IT_MAX(0, s_cursor_row - 16);
            break;
        case SDLK_PAGEDOWN:
            s_cursor_row = IT_MIN(pat->rows - 1, s_cursor_row + 16);
            break;
        case SDLK_HOME:
            s_cursor_row = 0;
            break;
        case SDLK_END:
            s_cursor_row = pat->rows - 1;
            break;
        case SDLK_TAB:
            if (mod & SDL_KMOD_SHIFT) {
                if (s_cursor_ch > 0) s_cursor_ch--;
            } else {
                if (s_cursor_ch < IT_MAX_CHANNELS - 1) s_cursor_ch++;
            }
            break;
        case SDLK_DELETE:
        case SDLK_BACKSPACE: {
            if (ui->edit_mode) {
                ITCell* cell = it_pattern_get_cell(pat, s_cursor_row, s_cursor_ch);
                if (cell) {
                    if (s_cursor_field == 0) cell->note = IT_NOTE_NONE;
                    else if (s_cursor_field == 1) cell->instrument = 0;
                    else if (s_cursor_field == 2) cell->volume = 0;
                    else if (s_cursor_field == 3 || s_cursor_field == 4) { cell->effect = 0; cell->param = 0; }
                }
            }
            break;
        }
        case SDLK_GRAVE: { /* Note cut */
            if (ui->edit_mode) {
                ITCell* cell = it_pattern_get_cell(pat, s_cursor_row, s_cursor_ch);
                if (cell) {
                    cell->note = IT_NOTE_CUT;
                    s_cursor_row = IT_MIN(pat->rows - 1, s_cursor_row + ui->edit_step);
                }
            }
            break;
        }
        case SDLK_1: { /* Note off */
            if (ui->edit_mode && s_cursor_field == 0) {
                ITCell* cell = it_pattern_get_cell(pat, s_cursor_row, s_cursor_ch);
                if (cell) {
                    cell->note = IT_NOTE_OFF;
                    s_cursor_row = IT_MIN(pat->rows - 1, s_cursor_row + ui->edit_step);
                }
            }
            break;
        }
        default: {
            int note = map_key_to_note(key, ui->current_octave);
            if (note != IT_NOTE_NONE) {
                /* Play note preview immediately */
                it_player_trigger_note(s_cursor_ch, note, ui->current_instrument, 64);

                if (ui->edit_mode) {
                    ITCell* cell = it_pattern_get_cell(pat, s_cursor_row, s_cursor_ch);
                    if (cell) {
                        cell->note = (uint8_t)note;
                        cell->instrument = (uint8_t)ui->current_instrument;
                        s_cursor_row = IT_MIN(pat->rows - 1, s_cursor_row + ui->edit_step);
                    }
                }
            }
            break;
        }
    }
}

static void toggle_channel_solo(ITSong* song, int ch) {
    if (!song || ch < 0 || ch >= IT_MAX_CHANNELS) return;
    bool is_currently_solo = true;
    for (int i = 0; i < IT_MAX_CHANNELS; i++) {
        if (i == ch && song->channel_mute[i]) is_currently_solo = false;
        if (i != ch && !song->channel_mute[i]) is_currently_solo = false;
    }
    if (is_currently_solo) {
        /* Unmute all channels */
        for (int i = 0; i < IT_MAX_CHANNELS; i++) it_player_set_channel_mute(i, false);
    } else {
        /* Mute all channels except ch */
        for (int i = 0; i < IT_MAX_CHANNELS; i++) it_player_set_channel_mute(i, (i != ch));
    }
}

void it_page_pattern_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();
    ITUIState* ui = it_ui_get_state();
    ITSong* song = it_player_get_song();
    if (!song) return;

    int pat_idx = it_player_get_pattern();
    if (pat_idx < 0 || pat_idx >= IT_MAX_PATTERNS) return;
    ITPattern* pat = &song->patterns[pat_idx];
    if (!pat->data || pat->rows == 0) return;

    /* If playing and follow song is on, follow the player row */
    if (it_player_is_playing() && s_follow_song) {
        s_cursor_row = it_player_get_row();
    }

    /* Pattern layout dimensions */
    int row_header_w = 46;
    int cell_w = 122;
    int row_h = 20;

    int grid_x = 6;
    int hdr_y = 62;
    int hdr_h = 22;
    int grid_y = 85;
    int grid_w = screen_w - 12;
    int grid_h = screen_h - grid_y - 28;

    int visible_rows = grid_h / row_h;
    int visible_channels = (grid_w - row_header_w - 16) / cell_w;
    if (visible_channels > IT_MAX_CHANNELS) visible_channels = IT_MAX_CHANNELS;
    if (visible_channels < 1) visible_channels = 1;

    /* Mouse Wheel Navigation */
    if (ui->wheel_y != 0) {
        if (SDL_GetModState() & SDL_KMOD_SHIFT) {
            s_first_visible_ch = IT_CLAMP(s_first_visible_ch - ui->wheel_y * 2, 0, IT_MAX_CHANNELS - visible_channels);
        } else {
            s_cursor_row = IT_CLAMP(s_cursor_row - ui->wheel_y * 4, 0, pat->rows - 1);
        }
    }

    /* Keep cursor channel in visible range */
    if (s_cursor_ch < s_first_visible_ch) s_first_visible_ch = s_cursor_ch;
    if (s_cursor_ch >= s_first_visible_ch + visible_channels) s_first_visible_ch = s_cursor_ch - visible_channels + 1;
    if (s_first_visible_ch < 0) s_first_visible_ch = 0;

    /* Center cursor row vertically */
    int center_offset = visible_rows / 2;
    int start_row = s_cursor_row - center_offset;
    if (start_row < 0) start_row = 0;
    if (start_row + visible_rows > pat->rows) start_row = IT_MAX(0, pat->rows - visible_rows);

    /* --- Top Channel Headers & Controls --- */
    it_gfx_fill_rect(grid_x, hdr_y, grid_w, hdr_h, IT_COL_PANEL_DARK);
    it_gfx_draw_rect(grid_x, hdr_y, grid_w, hdr_h, IT_COL_PANEL_BORDER);

    it_gfx_draw_text(grid_x + 8, hdr_y + 4, "Row", IT_COL_GRAY);

    for (int i = 0; i < visible_channels; i++) {
        int ch = s_first_visible_ch + i;
        if (ch >= IT_MAX_CHANNELS) break;

        int cx = grid_x + row_header_w + i * cell_w;
        bool is_cur = (ch == s_cursor_ch);
        bool is_muted = song->channel_mute[ch];

        /* Header background */
        it_gfx_fill_rect(cx, hdr_y + 1, cell_w - 1, hdr_h - 2, is_cur ? (ITColor){ 16, 32, 52, 255 } : IT_COL_PANEL);
        it_gfx_draw_line(cx + cell_w - 1, hdr_y, cx + cell_w - 1, hdr_y + hdr_h, IT_COL_PANEL_BORDER);

        /* Channel label */
        char ch_buf[16];
        snprintf(ch_buf, sizeof(ch_buf), "Ch %02d", ch + 1);
        ITColor hcol = is_cur ? IT_COL_CYAN : (is_muted ? IT_COL_DARK_GRAY : IT_COL_WHITE);
        it_gfx_draw_text_bold(cx + 6, hdr_y + 4, ch_buf, hcol);

        /* Mute Button [M] */
        int m_x = cx + cell_w - 44;
        int m_y = hdr_y + 3;
        if (it_widget_button(m_x, m_y, 18, 18, "M", is_muted)) {
            it_player_set_channel_mute(ch, !is_muted);
        }

        /* Solo Button [S] */
        int s_x = cx + cell_w - 24;
        int s_y = hdr_y + 3;
        if (it_widget_button(s_x, s_y, 18, 18, "S", false)) {
            toggle_channel_solo(song, ch);
        }

        /* Channel click selects cursor channel */
        if (ui->mouse_clicked && !ui->click_consumed &&
            ui->mouse_x >= cx && ui->mouse_x < cx + cell_w - 46 &&
            ui->mouse_y >= hdr_y && ui->mouse_y < hdr_y + hdr_h) {
            s_cursor_ch = ch;
            ui->click_consumed = true;
        }

        /* Double-click header solos channel */
        if (ui->mouse_double_clicked && !ui->click_consumed &&
            ui->mouse_x >= cx && ui->mouse_x < cx + cell_w &&
            ui->mouse_y >= hdr_y && ui->mouse_y < hdr_y + hdr_h) {
            toggle_channel_solo(song, ch);
            ui->click_consumed = true;
        }
    }

    /* --- Pattern Grid Background & Frame --- */
    it_gfx_fill_rect(grid_x, grid_y, grid_w, grid_h, IT_COL_PANEL);
    it_gfx_draw_rect(grid_x, grid_y, grid_w, grid_h, IT_COL_PANEL_BORDER);

    /* Mouse Click & Drag to Position Cursor */
    int grid_content_w = visible_channels * cell_w;
    if ((ui->mouse_clicked || (ui->mouse_down && ui->active_drag_id == 888)) && !ui->click_consumed &&
        ui->mouse_x >= grid_x + row_header_w && ui->mouse_x < grid_x + row_header_w + grid_content_w &&
        ui->mouse_y >= grid_y && ui->mouse_y < grid_y + grid_h) {

        int click_r_offset = (ui->mouse_y - grid_y) / row_h;
        int target_r = start_row + click_r_offset;
        if (target_r >= 0 && target_r < pat->rows) {
            s_cursor_row = target_r;
        }

        int click_ch_offset = (ui->mouse_x - (grid_x + row_header_w)) / cell_w;
        int target_c = s_first_visible_ch + click_ch_offset;
        if (target_c >= 0 && target_c < IT_MAX_CHANNELS) {
            s_cursor_ch = target_c;
        }

        int rel_x = (ui->mouse_x - (grid_x + row_header_w)) % cell_w;
        if (rel_x < 38) s_cursor_field = 0;       /* Note */
        else if (rel_x < 62) s_cursor_field = 1;  /* Instrument */
        else if (rel_x < 92) s_cursor_field = 2;  /* Volume */
        else s_cursor_field = 3;                  /* Effect */

        if (ui->mouse_clicked) {
            ui->click_consumed = true;
            ui->active_drag_id = 888;
        }
    }

    /* Right Click to Clear/Cut Cell in Edit Mode */
    if (ui->mouse_rclicked && !ui->click_consumed &&
        ui->mouse_x >= grid_x + row_header_w && ui->mouse_x < grid_x + row_header_w + grid_content_w &&
        ui->mouse_y >= grid_y && ui->mouse_y < grid_y + grid_h) {

        int click_r_offset = (ui->mouse_y - grid_y) / row_h;
        int target_r = start_row + click_r_offset;
        int click_ch_offset = (ui->mouse_x - (grid_x + row_header_w)) / cell_w;
        int target_c = s_first_visible_ch + click_ch_offset;

        if (target_r >= 0 && target_r < pat->rows && target_c >= 0 && target_c < IT_MAX_CHANNELS) {
            s_cursor_row = target_r;
            s_cursor_ch = target_c;
            if (ui->edit_mode) {
                ITCell* cell = it_pattern_get_cell(pat, target_r, target_c);
                if (cell) {
                    cell->note = 0;
                    cell->instrument = 0;
                    cell->volume = 0;
                    cell->effect = 0;
                    cell->param = 0;
                }
            }
        }
        ui->click_consumed = true;
    }

    int play_row = it_player_is_playing() ? it_player_get_row() : -1;

    /* Render Rows */
    for (int r = 0; r < visible_rows; r++) {
        int row_idx = start_row + r;
        if (row_idx >= pat->rows) break;

        int ry = grid_y + r * row_h;
        bool is_cursor_row = (row_idx == s_cursor_row);
        bool is_play_row = (row_idx == play_row);

        /* Row background highlights */
        if (is_cursor_row) {
            it_gfx_fill_rect(grid_x + 1, ry, grid_w - 2, row_h, IT_COL_ROW_HILIGHT2);
        } else if (is_play_row) {
            it_gfx_fill_rect(grid_x + 1, ry, grid_w - 2, row_h, (ITColor){ 30, 55, 20, 255 });
        } else if (row_idx % 16 == 0) {
            it_gfx_fill_rect(grid_x + 1, ry, grid_w - 2, row_h, IT_COL_ROW_HILIGHT1);
        } else if (row_idx % 4 == 0) {
            it_gfx_fill_rect(grid_x + 1, ry, grid_w - 2, row_h, (ITColor){ 14, 20, 34, 255 });
        }

        /* 3-Digit Row Number */
        char row_str[16];
        snprintf(row_str, sizeof(row_str), "%03d", row_idx);
        ITColor row_col = is_cursor_row ? IT_COL_YELLOW : ((row_idx % 16 == 0) ? IT_COL_YELLOW : ((row_idx % 4 == 0) ? IT_COL_CYAN : IT_COL_GRAY));
        it_gfx_draw_text(grid_x + 8, ry + 2, row_str, row_col);

        /* Render Cells for this Row */
        for (int i = 0; i < visible_channels; i++) {
            int ch = s_first_visible_ch + i;
            if (ch >= IT_MAX_CHANNELS) break;

            int cx = grid_x + row_header_w + i * cell_w;
            const ITCell* c = it_pattern_get_cell(pat, row_idx, ch);
            if (!c) continue;

            /* Vertical Divider Line between Channels */
            it_gfx_draw_line(cx + cell_w - 1, ry, cx + cell_w - 1, ry + row_h, (ITColor){ 24, 36, 52, 255 });

            /* Active Cell Outline */
            if (is_cursor_row && ch == s_cursor_ch) {
                it_gfx_draw_rect(cx + 1, ry + 1, cell_w - 3, row_h - 2, IT_COL_CYAN);
            }

            /* Draw Note */
            const char* n_str = it_note_to_string(c->note);
            ITColor n_col = (c->note > 0 && c->note <= 120) ? IT_COL_WHITE :
                            ((c->note == IT_NOTE_CUT || c->note == IT_NOTE_OFF) ? IT_COL_RED : (ITColor){ 45, 62, 82, 255 });
            it_gfx_draw_text(cx + 4, ry + 2, n_str, n_col);

            /* Draw Instrument */
            char inst_str[4];
            if (c->instrument > 0) snprintf(inst_str, sizeof(inst_str), "%02d", c->instrument);
            else strcpy(inst_str, "..");
            it_gfx_draw_text(cx + 40, ry + 2, inst_str, (c->instrument > 0) ? IT_COL_YELLOW : (ITColor){ 45, 62, 82, 255 });

            /* Draw Volume */
            char vol_str[6];
            if (c->volume > 0 && c->volume <= 65) snprintf(vol_str, sizeof(vol_str), "v%02d", c->volume - 1);
            else if (c->volume >= 128 && c->volume <= 192) snprintf(vol_str, sizeof(vol_str), "p%02d", c->volume - 128);
            else strcpy(vol_str, "...");
            it_gfx_draw_text(cx + 64, ry + 2, vol_str, (c->volume > 0) ? IT_COL_GREEN : (ITColor){ 45, 62, 82, 255 });

            /* Draw Effect & Param */
            char eff_str[6];
            if (c->effect || c->param) {
                char eff_ch = c->effect ? (c->effect < 27 ? 'A' + c->effect - 1 : '?') : '.';
                snprintf(eff_str, sizeof(eff_str), "%c%02X", eff_ch, c->param);
            } else {
                strcpy(eff_str, "...");
            }
            it_gfx_draw_text(cx + 92, ry + 2, eff_str, (c->effect || c->param) ? IT_COL_AMBER : (ITColor){ 45, 62, 82, 255 });

            /* High-contrast Field Focus Box */
            if (is_cursor_row && ch == s_cursor_ch) {
                int cur_x = cx + 4;
                int cur_w = 32;
                if (s_cursor_field == 1) { cur_x = cx + 38; cur_w = 22; }
                else if (s_cursor_field == 2) { cur_x = cx + 62; cur_w = 26; }
                else if (s_cursor_field >= 3) { cur_x = cx + 90; cur_w = 28; }

                ITColor cur_col = ui->edit_mode ? IT_COL_RED : IT_COL_CYAN;
                it_gfx_draw_rect(cur_x - 2, ry + 1, cur_w + 4, row_h - 2, cur_col);
            }
        }
    }

    /* Vertical Scrollbar */
    it_widget_scrollbar(grid_x + grid_w - 14, grid_y, 14, grid_h, pat->rows, visible_rows, &start_row);
}
