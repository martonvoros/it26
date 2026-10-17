#include "page_files.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "fmt_it.h"
#include "player.h"
#include "demo_songs.h"
#include "export.h"

#define MAX_DIR_ENTRIES 128

typedef struct {
    char name[128];
    bool is_dir;
} FileEntry;

static FileEntry s_entries[MAX_DIR_ENTRIES];
static int s_num_entries = 0;
static int s_selected_idx = 0;
static char s_current_path[512] = ".";
static int s_tab = 1; /* Default to Modern Genre Demos & Beats! */
static int s_selected_genre = 0;
static int s_genre_scroll = 0;

static SDL_EnumerationResult SDLCALL enum_dir_cb(void* userdata, const char* dirname, const char* fname) {
    if (s_num_entries >= MAX_DIR_ENTRIES) return SDL_ENUM_SUCCESS;
    if (strcmp(fname, ".") == 0) return SDL_ENUM_SUCCESS;

    strncpy(s_entries[s_num_entries].name, fname, sizeof(s_entries[s_num_entries].name) - 1);

    char full[1024];
    snprintf(full, sizeof(full), "%s/%s", dirname, fname);
    SDL_PathInfo info;
    if (SDL_GetPathInfo(full, &info) && info.type == SDL_PATHTYPE_DIRECTORY) {
        s_entries[s_num_entries].is_dir = true;
    } else {
        s_entries[s_num_entries].is_dir = false;
    }

    s_num_entries++;
    return SDL_ENUM_CONTINUE;
}

static void refresh_dir(void) {
    s_num_entries = 0;
    s_selected_idx = 0;

    /* Add parent directory entry if not at root */
    if (strcmp(s_current_path, ".") != 0) {
        strcpy(s_entries[0].name, "..");
        s_entries[0].is_dir = true;
        s_num_entries = 1;
    }

    SDL_EnumerateDirectory(s_current_path, enum_dir_cb, NULL);
}

void it_page_files_init(void) {
    strcpy(s_current_path, ".");
    refresh_dir();
    s_tab = 1;
    s_selected_genre = 0;
    s_genre_scroll = 0;
}

static void render_disk_tab(int screen_w, int screen_h, ITUIState* ui) {
    int panel_y = 94;
    int panel_h = screen_h - panel_y - 28;
    it_widget_panel(10, panel_y, screen_w - 20, panel_h, "Local File Browser (.IT / .S3M / .XM / .MOD / .WAV)");

    /* Current Path Display */
    char path_buf[600];
    snprintf(path_buf, sizeof(path_buf), "Location: %s", s_current_path);
    it_gfx_draw_text(24, panel_y + 30, path_buf, IT_COL_YELLOW);

    /* Quick Shortcut: Load official TUTE.IT */
    if (it_widget_button(screen_w - 360, panel_y + 26, 330, 26, "Load Tutorial Song (TUTE.IT)", true)) {
        ITSong* song = it_player_get_song();
        if (song) {
            if (it_load_file(song, "reference/impulse-tracker/Tutorial/TUTE.IT")) {
                it_ui_set_status("Loaded Jeffrey Lim's official TUTE.IT!");
                it_ui_set_page(PAGE_PATTERN);
                it_player_play_song();
            }
        }
    }

    /* File List Box */
    int list_x = 24;
    int list_y = panel_y + 58;
    int list_w = screen_w - 48;
    int list_h = panel_h - 110;

    it_gfx_fill_rect(list_x, list_y, list_w, list_h, IT_COL_PANEL_DARK);
    it_gfx_draw_rect(list_x, list_y, list_w, list_h, IT_COL_PANEL_BORDER);

    int item_h = 22;
    int visible_items = list_h / item_h;

    /* Mouse wheel on file list */
    static int s_file_scroll = 0;
    if (ui->wheel_y != 0 && ui->mouse_x >= list_x && ui->mouse_x < list_x + list_w &&
        ui->mouse_y >= list_y && ui->mouse_y < list_y + list_h) {
        s_file_scroll = IT_CLAMP(s_file_scroll - ui->wheel_y * 2, 0, IT_MAX(0, s_num_entries - visible_items));
    }

    for (int idx = 0; idx < visible_items; idx++) {
        int i = s_file_scroll + idx;
        if (i >= s_num_entries) break;

        int iy = list_y + idx * item_h;
        bool is_sel = (i == s_selected_idx);

        if (is_sel) {
            it_gfx_fill_rect(list_x + 2, iy + 1, list_w - 18, item_h - 2, IT_COL_ROW_HILIGHT2);
        }

        const FileEntry* e = &s_entries[i];
        char row_str[160];
        snprintf(row_str, sizeof(row_str), "%s %s", e->is_dir ? "[DIR]" : "     ", e->name);

        ITColor col = e->is_dir ? IT_COL_CYAN : (is_sel ? IT_COL_YELLOW : IT_COL_WHITE);
        it_gfx_draw_text(list_x + 10, iy + 2, row_str, col);

        /* Single Click */
        if (ui->mouse_clicked && !ui->click_consumed &&
            ui->mouse_x >= list_x && ui->mouse_x < list_x + list_w - 18 &&
            ui->mouse_y >= iy && ui->mouse_y < iy + item_h) {
            s_selected_idx = i;
            ui->click_consumed = true;
        }

        /* Double Click to Open Directory or Load File */
        if (ui->mouse_double_clicked && !ui->click_consumed &&
            ui->mouse_x >= list_x && ui->mouse_x < list_x + list_w - 18 &&
            ui->mouse_y >= iy && ui->mouse_y < iy + item_h) {
            s_selected_idx = i;
            ui->click_consumed = true;
            if (e->is_dir) {
                if (strcmp(e->name, "..") == 0) {
                    char* last_slash = strrchr(s_current_path, '/');
                    if (!last_slash) last_slash = strrchr(s_current_path, '\\');
                    if (last_slash && last_slash != s_current_path) *last_slash = '\0';
                    else strcpy(s_current_path, ".");
                } else {
                    char next_path[1024];
                    snprintf(next_path, sizeof(next_path), "%s/%s", s_current_path, e->name);
                    strncpy(s_current_path, next_path, sizeof(s_current_path) - 1);
                }
                refresh_dir();
            } else {
                char full_file[1024];
                snprintf(full_file, sizeof(full_file), "%s/%s", s_current_path, e->name);
                ITSong* song = it_player_get_song();
                if (song && it_load_module_any(song, full_file)) {
                    it_ui_set_status("Loaded module: %s", e->name);
                    it_ui_set_page(PAGE_PATTERN);
                    it_player_play_song();
                }
            }
        }
    }

    /* Scrollbar */
    it_widget_scrollbar(list_x + list_w - 14, list_y, 12, list_h, s_num_entries, visible_items, &s_file_scroll);

    /* Action Buttons Below */
    int act_y = panel_y + panel_h - 44;

    if (it_widget_button(list_x, act_y, 160, 30, "Load Selected", true)) {
        if (s_selected_idx >= 0 && s_selected_idx < s_num_entries) {
            FileEntry* e = &s_entries[s_selected_idx];
            if (e->is_dir) {
                if (strcmp(e->name, "..") == 0) {
                    char* last_slash = strrchr(s_current_path, '/');
                    if (!last_slash) last_slash = strrchr(s_current_path, '\\');
                    if (last_slash && last_slash != s_current_path) *last_slash = '\0';
                    else strcpy(s_current_path, ".");
                } else {
                    char next_path[1024];
                    snprintf(next_path, sizeof(next_path), "%s/%s", s_current_path, e->name);
                    strncpy(s_current_path, next_path, sizeof(s_current_path) - 1);
                }
                refresh_dir();
            } else {
                char full_file[1024];
                snprintf(full_file, sizeof(full_file), "%s/%s", s_current_path, e->name);
                ITSong* song = it_player_get_song();
                if (song && it_load_module_any(song, full_file)) {
                    it_ui_set_status("Loaded module: %s", e->name);
                    it_ui_set_page(PAGE_PATTERN);
                    it_player_play_song();
                } else {
                    it_ui_set_status("Failed to load: %s", e->name);
                }
            }
        }
    }

    int btn_x = list_x + 160;
    if (it_widget_button(btn_x, act_y, 110, 30, "Save as .IT", false)) {
        ITSong* song = it_player_get_song();
        if (song) {
            char save_path[1024];
            snprintf(save_path, sizeof(save_path), "%s/my_song.it", s_current_path);
            if (it_save_file(song, save_path)) {
                it_ui_set_status("Saved module to: %s", save_path);
                refresh_dir();
            }
        }
    }
    btn_x += 114;
    if (it_widget_button(btn_x, act_y, 110, 30, "Export WAV", false)) {
        ITSong* song = it_player_get_song();
        if (song) {
            char wav_path[1024];
            snprintf(wav_path, sizeof(wav_path), "%s/export_song.wav", s_current_path);
            if (it_export_wav_song(song, wav_path)) {
                it_ui_set_status("Exported Full Song to WAV: %s", wav_path);
                refresh_dir();
            }
        }
    }
    btn_x += 114;
    if (it_widget_button(btn_x, act_y, 110, 30, "Export MIDI", false)) {
        ITSong* song = it_player_get_song();
        if (song) {
            char mid_path[1024];
            snprintf(mid_path, sizeof(mid_path), "%s/export_song.mid", s_current_path);
            if (it_export_midi_song(song, mid_path)) {
                it_ui_set_status("Exported Full Song to MIDI: %s", mid_path);
                refresh_dir();
            }
        }
    }
    btn_x += 114;
    if (it_widget_button(btn_x, act_y, 90, 30, "Refresh", false)) {
        refresh_dir();
    }
}

static void render_demos_tab(int screen_w, int screen_h, ITUIState* ui) {
    int list_w = 400;
    int panel_y = 94;
    int panel_h = screen_h - panel_y - 28;

    /* Left Panel: Genre List */
    it_widget_panel(10, panel_y, list_w, panel_h, "Select Genre (20 Real IT Styles)");

    int count = it_demo_get_genre_count();
    int item_h = 26;
    int visible_items = (panel_h - 40) / item_h;
    int ly = panel_y + 32;

    for (int i = 0; i < visible_items && (i + s_genre_scroll) < count; i++) {
        int idx = i + s_genre_scroll;
        const ITGenreInfo* info = it_demo_get_genre_info(idx);
        if (!info) continue;

        int iy = ly + i * item_h;
        bool is_sel = (idx == s_selected_genre);

        if (is_sel) {
            it_gfx_fill_rect(16, iy, list_w - 12, item_h - 2, IT_COL_ROW_HILIGHT2);
        }

        char line[128];
        snprintf(line, sizeof(line), "%02d. %s [%d BPM]", idx + 1, info->name, info->bpm);
        it_gfx_draw_text_bold(22, iy + 4, line, is_sel ? IT_COL_YELLOW : IT_COL_WHITE);

        if (ui->mouse_clicked && !ui->click_consumed &&
            ui->mouse_x >= 16 && ui->mouse_x < 16 + list_w - 12 &&
            ui->mouse_y >= iy && ui->mouse_y < iy + item_h) {
            s_selected_genre = idx;
            ui->click_consumed = true;
        }
    }

    if (ui->wheel_y != 0 && ui->mouse_x < list_w + 32) {
        s_genre_scroll -= ui->wheel_y;
        if (s_genre_scroll < 0) s_genre_scroll = 0;
        if (s_genre_scroll > count - visible_items) s_genre_scroll = count - visible_items;
    }

    /* Right Panel: Selected Genre Inspector & Load Buttons */
    int rx = 10 + list_w + 8;
    int rw = screen_w - rx - 10;
    it_widget_panel(rx, panel_y, rw, panel_h, "Genre Details & Instant Loader");

    const ITGenreInfo* sel_info = it_demo_get_genre_info(s_selected_genre);
    if (sel_info) {
        int ry = 114;
        char title[128];
        snprintf(title, sizeof(title), "GENRE: %s", sel_info->name);
        it_gfx_draw_text_bold(rx + 20, ry, title, IT_COL_CYAN);
        ry += 28;

        char tempo_str[64];
        snprintf(tempo_str, sizeof(tempo_str), "Tempo: %d BPM  |  Default Speed: %d ticks/row", sel_info->bpm, sel_info->speed);
        it_gfx_draw_text_bold(rx + 20, ry, tempo_str, IT_COL_YELLOW);
        ry += 28;

        it_gfx_draw_text(rx + 20, ry, "Style & Groove Description:", IT_COL_GRAY);
        ry += 20;
        it_gfx_draw_text(rx + 20, ry, sel_info->description, IT_COL_WHITE);
        ry += 32;

        it_gfx_draw_text(rx + 20, ry, "Sound Blaster 16+ Orchestration & Real Samples:", IT_COL_GRAY);
        ry += 20;
        it_gfx_draw_text(rx + 20, ry, sel_info->drum_kit_info, IT_COL_WHITE);
        ry += 40;

        it_gfx_draw_line(rx + 20, ry, rx + rw - 40, ry, IT_COL_PANEL_BORDER);
        ry += 24;

        /* BUTTON 1: Load and Play Complete Authentic IT Track */
        it_gfx_draw_text_bold(rx + 20, ry, "AUTHENTIC SOUND BLASTER 16+ TRACK (100% REAL SCENE MASTERPIECE)", IT_COL_YELLOW);
        ry += 20;
        it_gfx_draw_text(rx + 20, ry, "* Complete multi-channel arrangement: real drums, bassline, chords & melody.", IT_COL_WHITE);
        ry += 26;

        if (it_widget_button(rx + 20, ry, 420, 40, "[▶] LOAD & PLAY COMPLETE REAL TRACK", true)) {
            ITSong* song = it_player_get_song();
            if (song && it_demo_load_full_song(song, (ITGenreId)s_selected_genre)) {
                it_ui_set_status("Loaded %s (%s)!", sel_info->name, song->name);
                it_ui_set_page(PAGE_PATTERN);
                it_player_play_song();
            }
        }
        ry += 54;

        /* BUTTON 2: Open in Edit Mode */
        if (it_widget_button(rx + 20, ry, 420, 34, "[✎] Load & Open in Pattern Editor [Space to Edit]", false)) {
            ITSong* song = it_player_get_song();
            if (song && it_demo_load_full_song(song, (ITGenreId)s_selected_genre)) {
                it_ui_set_status("Loaded %s for editing!", sel_info->name);
                it_ui_set_page(PAGE_PATTERN);
                ui->edit_mode = true;
                it_player_play_song();
            }
        }
        ry += 48;

        /* BUTTON 3: Insert Pattern into active song */
        if (it_widget_button(rx + 20, ry, 420, 30, "+ Copy Authentic Pattern & Samples into Active Song", false)) {
            ITSong* song = it_player_get_song();
            if (song) {
                int cur_pat = it_player_get_pattern();
                it_demo_insert_pattern(song, cur_pat, (ITGenreId)s_selected_genre);
                it_ui_set_status("Copied %s pattern & samples into Pattern %02d!", sel_info->name, cur_pat);
                it_ui_set_page(PAGE_PATTERN);
            }
        }
    }
}

static void render_export_tab(int screen_w, int screen_h, ITUIState* ui) {
    (void)ui;
    int panel_y = 94;
    int panel_w = screen_w - 20;
    int panel_h = screen_h - panel_y - 28;

    it_widget_panel(10, panel_y, panel_w, panel_h, "Audio & MIDI Export Suite (WAV PCM 44.1kHz / Standard MIDI)");

    ITSong* song = it_player_get_song();
    int cur_pat = it_player_get_pattern();
    int col_w = (panel_w - 50) / 2;

    /* LEFT COLUMN: WAV AUDIO RENDERING */
    int lx = 25;
    int ly = panel_y + 36;
    it_gfx_draw_text_bold(lx, ly, "1. WAV AUDIO RENDERING (44.1kHz 16-BIT STEREO PCM)", IT_COL_YELLOW);
    ly += 22;
    it_gfx_draw_text(lx, ly, "Offline studio render with resonant filters, slides, and stereo panning.", IT_COL_GRAY);
    ly += 30;

    /* Box 1: Loop / Pattern */
    it_gfx_draw_rect(lx, ly, col_w, 140, IT_COL_PANEL_BORDER);
    it_gfx_draw_text_bold(lx + 16, ly + 14, "CURRENT PATTERN LOOP -> WAV", IT_COL_WHITE);
    char pat_info[128];
    snprintf(pat_info, sizeof(pat_info), "Pattern %02d (%d rows) at %d BPM, Speed %d",
             cur_pat, (song && cur_pat < IT_MAX_PATTERNS) ? song->patterns[cur_pat].rows : 64,
             it_player_get_tempo(), it_player_get_speed());
    it_gfx_draw_text(lx + 16, ly + 36, pat_info, IT_COL_CYAN);
    it_gfx_draw_text(lx + 16, ly + 56, "Target File: export_loop.wav", IT_COL_GRAY);

    if (it_widget_button(lx + 16, ly + 86, col_w - 32, 38, "[▶] RENDER PATTERN LOOP TO WAV", true)) {
        if (song) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/export_loop.wav", s_current_path);
            if (it_export_wav_loop(song, cur_pat, path)) {
                it_ui_set_status("SUCCESS: Exported Pattern %02d to WAV: %s", cur_pat, path);
                refresh_dir();
            } else {
                it_ui_set_status("ERROR: Failed to write WAV file to %s", path);
            }
        }
    }
    ly += 160;

    /* Box 2: Full Song */
    it_gfx_draw_rect(lx, ly, col_w, 140, IT_COL_PANEL_BORDER);
    it_gfx_draw_text_bold(lx + 16, ly + 14, "FULL PROJECT MASTER -> WAV", IT_COL_WHITE);
    char song_info[128];
    snprintf(song_info, sizeof(song_info), "All %d Orders in Sequence | Song: %.20s",
             song ? song->order_count : 0, song ? song->name : "Untitled");
    it_gfx_draw_text(lx + 16, ly + 36, song_info, IT_COL_CYAN);
    it_gfx_draw_text(lx + 16, ly + 56, "Target File: export_song.wav", IT_COL_GRAY);

    if (it_widget_button(lx + 16, ly + 86, col_w - 32, 38, "[▶] RENDER FULL SONG MASTER TO WAV", false)) {
        if (song) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/export_song.wav", s_current_path);
            if (it_export_wav_song(song, path)) {
                it_ui_set_status("SUCCESS: Exported Full Song Master to WAV: %s", path);
                refresh_dir();
            } else {
                it_ui_set_status("ERROR: Failed to write WAV file to %s", path);
            }
        }
    }

    /* RIGHT COLUMN: MIDI SEQUENCER EXPORT */
    int rx = lx + col_w + 20;
    int ry = panel_y + 36;
    it_gfx_draw_text_bold(rx, ry, "2. MIDI SEQUENCER EXPORT (STANDARD MIDI FILE FORMAT 1)", IT_COL_GREEN);
    ry += 22;
    it_gfx_draw_text(rx, ry, "Export tracks to import into FL Studio, Ableton Live, Logic Pro, Reaper, Cubase.", IT_COL_GRAY);
    ry += 30;

    /* Box 3: Loop / Pattern MIDI */
    it_gfx_draw_rect(rx, ry, col_w, 140, IT_COL_PANEL_BORDER);
    it_gfx_draw_text_bold(rx + 16, ry + 14, "CURRENT PATTERN LOOP -> MIDI (.MID)", IT_COL_WHITE);
    it_gfx_draw_text(rx + 16, ry + 36, "Exports 16 synchronous tracks with pitch & velocity data.", IT_COL_CYAN);
    it_gfx_draw_text(rx + 16, ry + 56, "Target File: export_loop.mid", IT_COL_GRAY);

    if (it_widget_button(rx + 16, ry + 86, col_w - 32, 38, "[♬] EXPORT PATTERN LOOP TO MIDI (.MID)", false)) {
        if (song) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/export_loop.mid", s_current_path);
            if (it_export_midi_loop(song, cur_pat, path)) {
                it_ui_set_status("SUCCESS: Exported Pattern %02d to MIDI: %s", cur_pat, path);
                refresh_dir();
            } else {
                it_ui_set_status("ERROR: Failed to write MIDI file to %s", path);
            }
        }
    }
    ry += 160;

    /* Box 4: Full Song MIDI */
    it_gfx_draw_rect(rx, ry, col_w, 140, IT_COL_PANEL_BORDER);
    it_gfx_draw_text_bold(rx + 16, ry + 14, "FULL PROJECT SONG -> MIDI (.MID)", IT_COL_WHITE);
    it_gfx_draw_text(rx + 16, ry + 36, "Complete arrangement timeline across all patterns and order list.", IT_COL_CYAN);
    it_gfx_draw_text(rx + 16, ry + 56, "Target File: export_song.mid", IT_COL_GRAY);

    if (it_widget_button(rx + 16, ry + 86, col_w - 32, 38, "[♬] EXPORT FULL SONG TO MIDI (.MID)", false)) {
        if (song) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/export_song.mid", s_current_path);
            if (it_export_midi_song(song, path)) {
                it_ui_set_status("SUCCESS: Exported Full Song to MIDI: %s", path);
                refresh_dir();
            } else {
                it_ui_set_status("ERROR: Failed to write MIDI file to %s", path);
            }
        }
    }
}

void it_page_files_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();
    ITUIState* ui = it_ui_get_state();

    /* Navigation Tabs */
    int tab_x = 10;
    int tab_y = 62;
    int tab_h = 28;

    if (it_widget_tab(tab_x, tab_y, 160, tab_h, "Local Disk Browser", s_tab == 0)) {
        s_tab = 0;
    }
    if (it_widget_tab(tab_x + 164, tab_y, 340, tab_h, "Real IT Genre Demos (20 Iconic Styles)", s_tab == 1)) {
        s_tab = 1;
    }
    if (it_widget_tab(tab_x + 164 + 344, tab_y, 260, tab_h, "Audio & MIDI Export (WAV / MID)", s_tab == 2)) {
        s_tab = 2;
    }

    /* Back to Tracker shortcut button */
    if (it_widget_button(screen_w - 180, tab_y, 170, tab_h, "Back to Tracker [Esc]", false)) {
        it_ui_set_page(PAGE_PATTERN);
    }

    if (s_tab == 0) {
        render_disk_tab(screen_w, screen_h, ui);
    } else if (s_tab == 1) {
        render_demos_tab(screen_w, screen_h, ui);
    } else {
        render_export_tab(screen_w, screen_h, ui);
    }
}
