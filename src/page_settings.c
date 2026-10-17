#include "page_settings.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "config.h"
#include "midi.h"
#include "midi_profiles.h"
#include "midi_learn.h"
#include "audio.h"
#include "player.h"

static int s_settings_tab = 1; /* Default to MIDI tab */
static int s_profile_scroll = 0;
static char s_brand_filter[32] = "All";
static int s_learn_scroll = 0;

void it_page_settings_init(void) {
    s_settings_tab = 1;
    s_profile_scroll = 0;
    s_learn_scroll = 0;
}

static void render_audio_tab(int px, int py, int pw, int ph) {
    it_widget_panel(px, py, pw, ph, "Audio Output & Display Configuration");

    int lx = px + 24;
    int ly = py + 36;

    /* Master Volume */
    it_gfx_draw_text(lx, ly, "Master Volume:", IT_COL_GRAY);
    ITConfig* cfg = it_config_get();
    int vol = (int)(cfg->master_volume * 100.0f);
    if (it_widget_slider(lx + 140, ly - 2, 240, 20, NULL, &vol, 0, 100)) {
        cfg->master_volume = (float)vol / 100.0f;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%d %%", vol);
    it_gfx_draw_text(lx + 395, ly, buf, IT_COL_WHITE);
    ly += 36;

    /* Sample Rate */
    it_gfx_draw_text(lx, ly, "Sample Rate:", IT_COL_GRAY);
    if (it_widget_button(lx + 140, ly - 4, 100, 26, "44.1 kHz", cfg->sample_rate == 44100)) {
        cfg->sample_rate = 44100;
    }
    if (it_widget_button(lx + 250, ly - 4, 100, 26, "48.0 kHz", cfg->sample_rate == 48000)) {
        cfg->sample_rate = 48000;
    }
    if (it_widget_button(lx + 360, ly - 4, 100, 26, "96.0 kHz", cfg->sample_rate == 96000)) {
        cfg->sample_rate = 96000;
    }
    ly += 42;

    /* UI Scale */
    it_gfx_draw_text(lx, ly, "UI Scaling:", IT_COL_GRAY);
    if (it_widget_button(lx + 140, ly - 4, 75, 26, "1.0x", cfg->ui_scale == 1.0f)) {
        it_gfx_set_ui_scale(1.0f);
    }
    if (it_widget_button(lx + 225, ly - 4, 75, 26, "1.25x", cfg->ui_scale == 1.25f)) {
        it_gfx_set_ui_scale(1.25f);
    }
    if (it_widget_button(lx + 310, ly - 4, 75, 26, "1.5x", cfg->ui_scale == 1.5f)) {
        it_gfx_set_ui_scale(1.5f);
    }
    if (it_widget_button(lx + 395, ly - 4, 75, 26, "2.0x", cfg->ui_scale == 2.0f)) {
        it_gfx_set_ui_scale(2.0f);
    }
    ly += 42;

    /* Vector Font & DPI Details */
    it_gfx_draw_line(lx, ly, lx + pw - 48, ly, IT_COL_PANEL_BORDER);
    ly += 16;

    it_gfx_draw_text_bold(lx, ly, "TrueType Font Vector Rendering Engine:", IT_COL_CYAN);
    ly += 24;
    it_gfx_draw_text(lx, ly, "* Embedded JetBrains Mono vector font rasterized with sub-pixel antialiasing.", IT_COL_WHITE);
    ly += 20;
    it_gfx_draw_text(lx, ly, "* UI coordinates automatically adapt to 1080p, 1440p, 4K, and Retina displays.", IT_COL_WHITE);
    ly += 20;
    it_gfx_draw_text(lx, ly, "* 32-bit floating-point mixing with full resonant low-pass state-variable filters.", IT_COL_WHITE);
    ly += 36;

    if (it_widget_button(lx, ly, 220, 30, "Save Configuration", true)) {
        it_config_save(cfg);
        it_ui_set_status("Settings saved to configuration file!");
    }
}

static void render_profiles_tab(int px, int py, int pw, int ph) {
    int dev_count = it_midi_get_device_count();
    int cur_dev = it_midi_get_current_device();
    const char* dev_name = (cur_dev >= 0) ? it_midi_get_device_name(cur_dev) : "None Connected";

    /* Active Device Banner */
    int top_panel_h = 100;
    it_widget_panel(px, py, pw, top_panel_h, "Connected MIDI Hardware Device");

    int lx = px + 20;
    int ly = py + 26;

    it_gfx_draw_text(lx, ly, "Active Input Port:", IT_COL_GRAY);
    it_gfx_draw_text_bold(lx + 150, ly, dev_name, (dev_count > 0) ? IT_COL_GREEN : IT_COL_RED);

    /* Prev / Next device selector */
    if (it_widget_button(lx + 480, ly - 4, 80, 24, "< Prev", cur_dev > 0)) {
        if (cur_dev > 0) it_midi_open_device(cur_dev - 1);
    }
    if (it_widget_button(lx + 570, ly - 4, 80, 24, "Next >", cur_dev < dev_count - 1)) {
        if (cur_dev < dev_count - 1) it_midi_open_device(cur_dev + 1);
    }

    ly += 28;
    /* Auto profile match detection */
    const ITMidiControllerProfile* matched_prof = NULL;
    if (dev_count > 0 && cur_dev >= 0) {
        matched_prof = it_midi_profiles_find_for_device(dev_name);
    }

    if (matched_prof) {
        char match_msg[128];
        snprintf(match_msg, sizeof(match_msg), "VERIFIED PROFILE MATCH: %s %s (Factory CC mappings active!)",
                 matched_prof->brand, matched_prof->model);
        it_gfx_draw_text_bold(lx, ly, match_msg, IT_COL_YELLOW);
    } else {
        it_gfx_draw_text_bold(lx, ly, "CONTROLLER NOT DIRECTLY RECOGNIZED? NO PROBLEM:", IT_COL_ORANGE);
        it_gfx_draw_text(lx + 430, ly, "Use the 'MIDI Learn & Remap' tab to bind any control!", IT_COL_WHITE);
    }

    /* Database of 655 MIDI Controllers */
    int db_y = py + top_panel_h + 8;
    int db_h = ph - top_panel_h - 8;
    it_widget_panel(px, db_y, pw, db_h, "Supported MIDI Controllers Database (650+ Models from 2018 to October 2026)");

    int flx = px + 20;
    int fly = db_y + 24;

    /* Brand Filters */
    const char* brands[] = { "All", "Akai", "Arturia", "Novation", "Roland", "Korg", "Native Instruments", "M-Audio", "Behringer", "Yamaha" };
    int num_brands = (int)(sizeof(brands) / sizeof(brands[0]));
    int btn_w = 68;

    for (int b = 0; b < num_brands; b++) {
        bool selected = (strcmp(s_brand_filter, brands[b]) == 0);
        if (it_widget_button(flx + b * (btn_w + 4), fly, btn_w, 22, brands[b], selected)) {
            strncpy(s_brand_filter, brands[b], sizeof(s_brand_filter) - 1);
            s_profile_scroll = 0;
        }
    }

    fly += 30;

    /* Table Header */
    int tbl_x = px + 16;
    int tbl_y = fly;
    int tbl_w = pw - 32;
    int row_h = 24;

    it_gfx_fill_rect(tbl_x, tbl_y, tbl_w, 24, IT_COL_PANEL_DARK);
    it_gfx_draw_text_bold(tbl_x + 10, tbl_y + 4, "Manufacturer", IT_COL_CYAN);
    it_gfx_draw_text_bold(tbl_x + 150, tbl_y + 4, "Controller Model", IT_COL_CYAN);
    it_gfx_draw_text_bold(tbl_x + 360, tbl_y + 4, "Device Match ID", IT_COL_CYAN);
    it_gfx_draw_text_bold(tbl_x + 550, tbl_y + 4, "Factory CCs (Cutoff/Res/Vol/Tempo)", IT_COL_CYAN);
    it_gfx_draw_text_bold(tbl_x + tbl_w - 110, tbl_y + 4, "Action", IT_COL_CYAN);

    int list_y = tbl_y + 26;
    int list_h = db_h - (list_y - db_y) - 16;
    it_gfx_set_clip(tbl_x, list_y, tbl_w, list_h);

    int total_profiles = it_midi_profiles_get_count();
    int visible_rows = list_h / row_h;
    int rendered = 0;
    int skipped = 0;

    for (int i = 0; i < total_profiles; i++) {
        const ITMidiControllerProfile* prof = it_midi_profiles_get(i);
        if (!prof) continue;

        /* Filter */
        if (strcmp(s_brand_filter, "All") != 0 && strcmp(s_brand_filter, prof->brand) != 0) {
            continue;
        }

        if (skipped < s_profile_scroll) {
            skipped++;
            continue;
        }

        if (rendered >= visible_rows) break;

        int ry = list_y + rendered * row_h;
        if (rendered % 2 == 1) {
            it_gfx_fill_rect(tbl_x, ry, tbl_w, row_h, IT_COL_PANEL_BG);
        }

        it_gfx_draw_text(tbl_x + 10, ry + 4, prof->brand, IT_COL_WHITE);
        it_gfx_draw_text_bold(tbl_x + 150, ry + 4, prof->model, IT_COL_YELLOW);
        it_gfx_draw_text(tbl_x + 360, ry + 4, prof->match_pattern, IT_COL_GRAY);

        char cc_str[64];
        snprintf(cc_str, sizeof(cc_str), "CC %d / %d / %d / %d",
                 prof->mapping.cc_knob_cutoff,
                 prof->mapping.cc_knob_resonance,
                 prof->mapping.cc_knob_volume,
                 prof->mapping.cc_knob_tempo);
        it_gfx_draw_text(tbl_x + 550, ry + 4, cc_str, IT_COL_PANEL_BORDER);

        /* Apply Preset Button */
        char btn_id[32];
        snprintf(btn_id, sizeof(btn_id), "Apply##%d", i);
        if (it_widget_button(tbl_x + tbl_w - 105, ry + 1, 95, 21, "Apply Map", false)) {
            it_midi_profiles_set_active(i);
            it_ui_set_status("Loaded factory CC mappings for %s %s!", prof->brand, prof->model);
        }

        rendered++;
    }

    it_gfx_reset_clip();

    /* Mouse scroll */
    ITUIState* ui = it_ui_get_state();
    if (ui->wheel_y != 0 && ui->mouse_y > db_y) {
        s_profile_scroll -= ui->wheel_y * 3;
        if (s_profile_scroll < 0) s_profile_scroll = 0;
        if (s_profile_scroll > 600) s_profile_scroll = 600;
    }
}

static void render_learn_tab(int px, int py, int pw, int ph) {
    int screen_w = it_gfx_get_logical_width();
    (void)screen_w;

    /* Step-by-Step Remapping Tutorial Banner */
    int tut_h = 105;
    it_widget_panel(px, py, pw, tut_h, "Interactive MIDI Remapping Tutorial & Instructions");

    int tx = px + 20;
    int ty = py + 22;
    it_gfx_draw_text_bold(tx, ty, "HOW TO MAP ANY UNSUPPORTED OR CUSTOM CONTROLLER IN 3 STEPS:", IT_COL_YELLOW);
    ty += 20;
    it_gfx_draw_text(tx, ty, "1. Locate the target function you want to control in the table below (e.g. Play, Filter Cutoff, Channel Volume).", IT_COL_WHITE);
    ty += 18;
    it_gfx_draw_text(tx, ty, "2. Click the [Learn] button next to that action. The button will flash 'LISTENING...'.", IT_COL_WHITE);
    ty += 18;
    it_gfx_draw_text(tx, ty, "3. Turn the knob, push the slider, or hit the drum pad on your controller. It will bind immediately!", IT_COL_CYAN);

    /* Live MIDI Diagnostic Monitor */
    int mon_y = py + tut_h + 8;
    int mon_h = 56;
    it_widget_panel(px, mon_y, pw, mon_h, "Live Hardware MIDI Monitor & Signal Inspector");

    ITMidiMessage last_msg;
    uint32_t age_ms = 999999;
    bool has_msg = it_midi_get_last_message(&last_msg, &age_ms);

    /* Activity indicator */
    bool active_now = has_msg && (age_ms < 250);
    it_gfx_draw_text_bold(px + 20, mon_y + 24, "SIGNAL:", IT_COL_GRAY);
    it_gfx_draw_text_bold(px + 80, mon_y + 24, "[● MIDI IN]", active_now ? IT_COL_GREEN : IT_COL_GRAY);

    if (has_msg && age_ms < 10000) {
        char sig_buf[128];
        uint8_t type = last_msg.status & 0xF0;
        int ch = (last_msg.status & 0x0F) + 1;
        if (type == 0xB0) {
            snprintf(sig_buf, sizeof(sig_buf), "Control Change (CC): CC #%d = Value %d (Channel %d) [Age: %u ms]",
                     last_msg.data1, last_msg.data2, ch, age_ms);
        } else if (type == 0x90) {
            snprintf(sig_buf, sizeof(sig_buf), "Note On: Note %d (%s) Velocity %d (Channel %d) [Age: %u ms]",
                     last_msg.data1, it_note_to_string(last_msg.data1 + 1), last_msg.data2, ch, age_ms);
        } else if (type == 0x80) {
            snprintf(sig_buf, sizeof(sig_buf), "Note Off: Note %d (%s) (Channel %d)",
                     last_msg.data1, it_note_to_string(last_msg.data1 + 1), ch);
        } else if (type == 0xE0) {
            snprintf(sig_buf, sizeof(sig_buf), "Pitch Bend: Value %d", ((int)last_msg.data2 << 7) | last_msg.data1);
        } else {
            snprintf(sig_buf, sizeof(sig_buf), "Raw Status 0x%02X: Data1=%d Data2=%d", last_msg.status, last_msg.data1, last_msg.data2);
        }
        it_gfx_draw_text_bold(px + 180, mon_y + 24, sig_buf, IT_COL_WHITE);
    } else {
        it_gfx_draw_text(px + 180, mon_y + 24, "Waiting for incoming MIDI messages... (turn a knob or press a key to test)", IT_COL_GRAY);
    }

    /* Learning Warning Banner if actively waiting for input */
    if (it_midi_learn_is_learning()) {
        ITMidiAction act = it_midi_learn_get_learning_action();
        char learn_banner[128];
        snprintf(learn_banner, sizeof(learn_banner), ">> LISTENING FOR INPUT TO BIND: '%s' << (Move controller now, or click Cancel)",
                 it_midi_learn_get_action_name(act));
        it_gfx_fill_rect(px, mon_y + mon_h + 4, pw, 28, IT_COL_YELLOW);
        it_gfx_draw_text_bold(px + 20, mon_y + mon_h + 10, learn_banner, IT_COL_PANEL_BG);

        if (it_widget_button(px + pw - 130, mon_y + mon_h + 6, 110, 22, "Cancel", true)) {
            it_midi_learn_cancel();
        }
    }

    /* Actions Mapping Table */
    int table_y = mon_y + mon_h + (it_midi_learn_is_learning() ? 36 : 8);
    int table_h = ph - (table_y - py) - 40;
    it_widget_panel(px, table_y, pw, table_h, "Custom MIDI Mapping Table (All Controller Functions)");

    int tbl_x = px + 16;
    int tbl_y = table_y + 24;
    int tbl_w = pw - 32;
    int row_h = 24;

    /* Headers */
    it_gfx_fill_rect(tbl_x, tbl_y, tbl_w, 24, IT_COL_PANEL_DARK);
    it_gfx_draw_text_bold(tbl_x + 10, tbl_y + 4, "Tracker Action", IT_COL_CYAN);
    it_gfx_draw_text_bold(tbl_x + 280, tbl_y + 4, "Category", IT_COL_CYAN);
    it_gfx_draw_text_bold(tbl_x + 440, tbl_y + 4, "Current Assignment", IT_COL_CYAN);
    it_gfx_draw_text_bold(tbl_x + tbl_w - 200, tbl_y + 4, "Remap / Clear", IT_COL_CYAN);

    int list_y = tbl_y + 26;
    int list_h = table_h - (list_y - table_y) - 8;
    it_gfx_set_clip(tbl_x, list_y, tbl_w, list_h);

    const char* categories[IT_ACTION_COUNT] = {
        "Transport", "Transport", "Transport", "Navigation", "Navigation",
        "Octave", "Octave",
        "Resonant Filter", "Resonant Filter",
        "Mixer Volume", "Mixer Panning", "Sequencer Tempo", "Global Volume",
        "Channel Mute", "Channel Mute", "Channel Mute", "Channel Mute"
    };

    for (int i = 0; i < IT_ACTION_COUNT; i++) {
        int ry = list_y + i * row_h - s_learn_scroll;
        if (ry < list_y - row_h || ry > list_y + list_h) continue;

        if (i % 2 == 1) {
            it_gfx_fill_rect(tbl_x, ry, tbl_w, row_h, IT_COL_PANEL_BG);
        }

        /* Action Name */
        it_gfx_draw_text_bold(tbl_x + 10, ry + 4, it_midi_learn_get_action_name((ITMidiAction)i), IT_COL_WHITE);
        it_gfx_draw_text(tbl_x + 280, ry + 4, categories[i], IT_COL_GRAY);

        /* Binding Status */
        const ITMidiBinding* b = it_midi_learn_get_binding((ITMidiAction)i);
        char bind_str[64];
        if (b && b->is_bound) {
            if (b->is_note) {
                snprintf(bind_str, sizeof(bind_str), "Note %d (%s) [Ch %d]",
                         b->cc_or_note, it_note_to_string(b->cc_or_note + 1), b->channel);
            } else {
                snprintf(bind_str, sizeof(bind_str), "CC #%d [Ch %d]", b->cc_or_note, b->channel);
            }
            it_gfx_draw_text_bold(tbl_x + 440, ry + 4, bind_str, IT_COL_GREEN);
        } else {
            it_gfx_draw_text(tbl_x + 440, ry + 4, "-- Unassigned --", IT_COL_PANEL_BORDER);
        }

        /* Learn Button */
        bool is_curr_learning = (it_midi_learn_is_learning() && it_midi_learn_get_learning_action() == (ITMidiAction)i);
        const char* btn_text = is_curr_learning ? "Listening..." : "Learn";
        if (it_widget_button(tbl_x + tbl_w - 190, ry + 1, 90, 21, btn_text, is_curr_learning)) {
            if (is_curr_learning) {
                it_midi_learn_cancel();
            } else {
                it_midi_learn_start((ITMidiAction)i);
            }
        }

        /* Clear Button */
        if (it_widget_button(tbl_x + tbl_w - 90, ry + 1, 80, 21, "Clear", false)) {
            it_midi_learn_clear_binding((ITMidiAction)i);
            it_ui_set_status("Cleared binding for %s", it_midi_learn_get_action_name((ITMidiAction)i));
        }
    }

    it_gfx_reset_clip();

    /* Bottom Buttons */
    int foot_y = py + ph - 34;
    if (it_widget_button(px + 16, foot_y, 220, 28, "Reset to Factory Defaults", false)) {
        it_midi_learn_init();
        it_ui_set_status("Reset all MIDI bindings to factory defaults!");
    }
    if (it_widget_button(px + 250, foot_y, 200, 28, "Clear All Custom Mappings", false)) {
        for (int i = 0; i < IT_ACTION_COUNT; i++) {
            it_midi_learn_clear_binding((ITMidiAction)i);
        }
        it_ui_set_status("All MIDI custom mappings cleared!");
    }
    if (it_widget_button(px + 465, foot_y, 220, 28, "Open Full MIDI Tutorial", true)) {
        it_ui_set_page(PAGE_HELP);
    }
}

void it_page_settings_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();

    /* Navigation Tabs */
    int tab_x = 10;
    int tab_y = 62;
    int tab_w = 230;
    int tab_h = 28;

    if (it_widget_tab(tab_x, tab_y, tab_w, tab_h, "Audio & UI Display", s_settings_tab == 0)) {
        s_settings_tab = 0;
    }
    if (it_widget_tab(tab_x + (tab_w + 4), tab_y, tab_w, tab_h, "MIDI Controllers (650+ Models)", s_settings_tab == 1)) {
        s_settings_tab = 1;
    }
    if (it_widget_tab(tab_x + (tab_w + 4) * 2, tab_y, tab_w, tab_h, "MIDI Learn & Remap Studio", s_settings_tab == 2)) {
        s_settings_tab = 2;
    }

    /* Back to Tracker shortcut button */
    if (it_widget_button(screen_w - 180, tab_y, 170, tab_h, "Back to Tracker [Esc]", false)) {
        it_ui_set_page(PAGE_PATTERN);
    }

    /* Content Area */
    int panel_x = 10;
    int panel_y = 94;
    int panel_w = screen_w - 20;
    int panel_h = screen_h - panel_y - 28;

    if (s_settings_tab == 0) {
        render_audio_tab(panel_x, panel_y, panel_w, panel_h);
    } else if (s_settings_tab == 1) {
        render_profiles_tab(panel_x, panel_y, panel_w, panel_h);
    } else if (s_settings_tab == 2) {
        render_learn_tab(panel_x, panel_y, panel_w, panel_h);
    }
}
