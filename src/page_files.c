#include "page_files.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "fmt_it.h"
#include "player.h"
#include "demo_songs.h"

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

    if (it_widget_button(list_x + 180, act_y, 160, 30, "Save as .IT", false)) {
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

    if (it_widget_button(list_x + 360, act_y, 120, 30, "Refresh", false)) {
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

        it_gfx_draw_text(rx + 20, ry, "Synthesized Drum Kit Elements:", IT_COL_GRAY);
        ry += 20;
        it_gfx_draw_text(rx + 20, ry, sel_info->drum_kit_info, IT_COL_WHITE);
        ry += 40;

        it_gfx_draw_line(rx + 20, ry, rx + rw - 40, ry, IT_COL_PANEL_BORDER);
        ry += 24;

        /* BUTTON 1: Drums only (ready to write on) */
        it_gfx_draw_text_bold(rx + 20, ry, "OPTION A: DRUM GROOVE ONLY (READY TO WRITE OVER)", IT_COL_GREEN);
        ry += 22;
        it_gfx_draw_text(rx + 20, ry, "* Loads drum kit onto channels 1-4 with a tight looping groove.", IT_COL_WHITE);
        ry += 18;
        it_gfx_draw_text(rx + 20, ry, "* Channels 5-64 remain COMPLETELY OPEN for you to compose music on!", IT_COL_WHITE);
        ry += 26;

        if (it_widget_button(rx + 20, ry, 360, 36, "[+] LOAD DRUM BEAT & WRITE MUSIC", true)) {
            ITSong* song = it_player_get_song();
            if (song && it_demo_load_drum_groove(song, (ITGenreId)s_selected_genre)) {
                it_ui_set_status("Loaded %s drum groove! Compose on Channels 5-64.", sel_info->name);
                it_ui_set_page(PAGE_PATTERN);
                ui->edit_mode = true;
                it_player_play_song();
            }
        }
        ry += 52;

        /* BUTTON 2: Full Demo Song */
        it_gfx_draw_text_bold(rx + 20, ry, "OPTION B: FULL ARRANGED MUSICAL DEMO TRACK", IT_COL_YELLOW);
        ry += 22;
        it_gfx_draw_text(rx + 20, ry, "* Complete track with drums, resonant swept bassline, leads & arps.", IT_COL_WHITE);
        ry += 26;

        if (it_widget_button(rx + 20, ry, 360, 36, "[>] LOAD FULL REAL IT TRACK & PLAY", false)) {
            ITSong* song = it_player_get_song();
            if (song && it_demo_load_full_song(song, (ITGenreId)s_selected_genre)) {
                it_ui_set_status("Loaded full %s demo song!", sel_info->name);
                it_ui_set_page(PAGE_PATTERN);
                it_player_play_song();
            }
        }
        ry += 52;

        /* BUTTON 3: Insert Pattern into active song */
        if (it_widget_button(rx + 20, ry, 360, 30, "+ Insert Drum Pattern into Active Song", false)) {
            ITSong* song = it_player_get_song();
            if (song) {
                int cur_pat = it_player_get_pattern();
                it_demo_insert_pattern(song, cur_pat, (ITGenreId)s_selected_genre);
                it_ui_set_status("Inserted %s drum groove into Pattern %02d!", sel_info->name, cur_pat);
                it_ui_set_page(PAGE_PATTERN);
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
    int tab_w = 200;
    int tab_h = 28;

    if (it_widget_tab(tab_x, tab_y, tab_w, tab_h, "Local Disk Browser", s_tab == 0)) {
        s_tab = 0;
    }
    if (it_widget_tab(tab_x + tab_w + 4, tab_y, 400, tab_h, "Real IT Genre Tracks & Drum Grooves (20 Styles)", s_tab == 1)) {
        s_tab = 1;
    }

    /* Back to Tracker shortcut button */
    if (it_widget_button(screen_w - 180, tab_y, 170, tab_h, "Back to Tracker [Esc]", false)) {
        it_ui_set_page(PAGE_PATTERN);
    }

    if (s_tab == 0) {
        render_disk_tab(screen_w, screen_h, ui);
    } else {
        render_demos_tab(screen_w, screen_h, ui);
    }
}
