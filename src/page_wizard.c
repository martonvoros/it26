#include "page_wizard.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "player.h"
#include "mixer.h"
#include "config.h"
#include "demo_songs.h"
#include "fmt_it.h"
#include <stdio.h>
#include <string.h>

static int s_wizard_step = 0; /* 0..3 */
static int s_active_key_flash = -1;
static uint32_t s_key_flash_timer = 0;

void it_page_wizard_init(void) {
    s_wizard_step = 0;
    s_active_key_flash = -1;
    s_key_flash_timer = 0;
}

static void play_preview_note(int note, int inst) {
    ITSong* song = it_player_get_song();
    if (!song) return;
    if (inst < 1) inst = 1;
    if (inst > song->sample_count) inst = 1;
    ITSample* smp = &song->samples[inst - 1];
    if (!smp->data || smp->length == 0) return;

    /* Audition on dedicated preview channel (host channel 63) */
    it_player_trigger_note(63, note, inst, 64);
    s_active_key_flash = note;
    s_key_flash_timer = SDL_GetTicks() + 200;
}

static void render_now_playing_banner(int x, int y, int w) {
    ITSong* song = it_player_get_song();
    bool playing = it_player_is_playing();

    it_gfx_fill_rect(x, y, w, 32, (ITColor){ 18, 24, 34, 255 });
    it_gfx_draw_rect(x, y, w, 32, (ITColor){ 45, 55, 75, 255 });

    /* Play/Pause Button */
    int btn_w = 96;
    if (it_widget_button(x + 5, y + 4, btn_w, 24, playing ? "■ Pause" : "▶ Resume", playing)) {
        if (playing) it_player_stop();
        else it_player_play_song();
    }

    /* Track info */
    char info_buf[128];
    const char* sname = (song && strlen(song->name) > 0) ? song->name : "House Of Heaven";
    snprintf(info_buf, sizeof(info_buf), "NOW PLAYING: %-22.22s | %d BPM | Spd %02d | %s",
             sname, it_player_get_tempo(), it_player_get_speed(), playing ? "LIVE" : "PAUSED");
    it_gfx_draw_text_bold(x + btn_w + 14, y + 8, info_buf, playing ? IT_COL_WHITE : IT_COL_GRAY);

    /* Quick Song Switchers (All 100% verified real IT hit tracks) */
    int sw_x = x + w - 425;
    if (it_widget_button(sw_x, y + 4, 102, 24, "House Heaven", false)) {
        if (song && it_load_module_any(song, "assets/hard_house.it")) {
            it_player_play_song();
            it_ui_set_status("Loaded House Of Heaven (Xtended)!");
        }
    }
    if (it_widget_button(sw_x + 106, y + 4, 102, 24, "UT Foregone", false)) {
        if (song && it_load_module_any(song, "assets/demos/foregone.it")) {
            it_player_play_song();
            it_ui_set_status("Loaded Michiel van den Bos: Unreal Tournament - Foregone Destruction!");
        }
    }
    if (it_widget_button(sw_x + 212, y + 4, 102, 24, "Skaven Mix", false)) {
        if (song && it_load_module_any(song, "assets/demos/skaven_-_quality_time_-_big_money_mix_v2.it")) {
            it_player_play_song();
            it_ui_set_status("Loaded Skaven: Quality Time (Big Money Mix)!");
        }
    }
    if (it_widget_button(sw_x + 318, y + 4, 102, 24, "Seeker UT", false)) {
        if (song && it_load_module_any(song, "assets/demos/basehead_-_unreal_tournament_-_seeker.it")) {
            it_player_play_song();
            it_ui_set_status("Loaded Basehead: Seeker (Unreal Tournament Soundtrack)!");
        }
    }
}

static void render_step_0(int x, int y, int w, int h) {
    /* Step 0: Tracker Mental Model vs Modern Horizontal DAWs */
    it_gfx_draw_text_bold(x + 20, y + 16, "1. THE TRACKER CONCEPT: VERTICAL TIMELINE VS HORIZONTAL DAW", IT_COL_YELLOW);
    it_gfx_draw_line(x + 20, y + 36, x + w - 20, y + 36, IT_COL_PANEL_BORDER);

    int cy = y + 48;
    it_gfx_draw_text_bold(x + 20, cy, "Welcome to IT26 - The Ultimate 64-Bit Tracker Workstation!", IT_COL_CYAN);
    cy += 24;

    it_gfx_draw_text(x + 20, cy, "If you're coming from FL Studio, Ableton Live, Logic Pro, or Reaper, tracker sequencers look different,", IT_COL_WHITE); cy += 20;
    it_gfx_draw_text(x + 20, cy, "but they are renowned by electronic legends (Aphex Twin, Deadmau5, Venetian Snares) for unmatched speed.", IT_COL_WHITE); cy += 28;

    /* Comparison Box */
    int box_w = (w - 60) / 2;
    int box_h = 170;

    /* Left Box: Horizontal DAWs */
    it_widget_panel(x + 20, cy, box_w, box_h, "Horizontal DAWs (Ableton, FL, Logic)");
    it_gfx_draw_text(x + 32, cy + 30, "* Time flows horizontally from Left to Right.", IT_COL_GRAY);
    it_gfx_draw_text(x + 32, cy + 54, "* Piano Roll: You click MIDI notes with a mouse.", IT_COL_GRAY);
    it_gfx_draw_text(x + 32, cy + 78, "* Automation: Drawn as curves underneath tracks.", IT_COL_GRAY);
    it_gfx_draw_text(x + 32, cy + 102, "* High mouse fatigue; zoom & scroll dependent.", IT_COL_GRAY);
    it_gfx_draw_text(x + 32, cy + 126, "* Swing & groove quantized to float milliseconds.", IT_COL_GRAY);

    /* Right Box: IT26 */
    it_widget_panel(x + 30 + box_w, cy, box_w, box_h, "IT26 (Vertical Speed Workstation)");
    it_gfx_draw_text(x + 42 + box_w, cy + 30, "* Time flows VERTICALLY from Top to Bottom.", IT_COL_CYAN);
    it_gfx_draw_text(x + 42 + box_w, cy + 54, "* Every line is a Row (16th note / rhythmic tick).", IT_COL_WHITE);
    it_gfx_draw_text(x + 42 + box_w, cy + 78, "* Everything on keyboard: type notes like words at 120 WPM!", IT_COL_GREEN);
    it_gfx_draw_text(x + 42 + box_w, cy + 102, "* Resonant 24dB SVF Lowpass Filter commands ('Z' effect).", IT_COL_YELLOW);
    it_gfx_draw_text(x + 42 + box_w, cy + 126, "* Micro-timing accurate to 1 tracker audio tick (zero jitter).", IT_COL_WHITE);

    cy += box_h + 20;

    it_widget_panel(x + 20, cy, w - 40, 76, "Pro Producer Insight");
    it_gfx_draw_text_bold(x + 32, cy + 28, "Tip: Tracks in trackers are monophonic or polyphonic voice streams.", IT_COL_YELLOW);
    it_gfx_draw_text(x + 32, cy + 50, "Channel 1 is usually Kick, Ch 2 Snare, Ch 3 Hi-Hats, Ch 4 Bassline, Ch 5-8 Chords & Leads.", IT_COL_WHITE);
}

static void render_step_1(int x, int y, int w, int h) {
    /* Step 1: Anatomy of a Pattern Cell */
    it_gfx_draw_text_bold(x + 20, y + 16, "2. ANATOMY OF A PATTERN ROW & CELL", IT_COL_YELLOW);
    it_gfx_draw_line(x + 20, y + 36, x + w - 20, y + 36, IT_COL_PANEL_BORDER);

    int cy = y + 48;
    it_gfx_draw_text(x + 20, cy, "Every tracker cell contains up to 4 values packed compactly in standard IT26 notation:", IT_COL_WHITE);
    cy += 28;

    /* Big Visual Cell Breakdown Box */
    int cell_box_w = IT_MIN(700, w - 40);
    int cell_box_x = x + (w - cell_box_w) / 2;
    it_gfx_fill_rect(cell_box_x, cy, cell_box_w, 70, (ITColor){ 10, 18, 30, 255 });
    it_gfx_draw_rect(cell_box_x, cy, cell_box_w, 70, IT_COL_CYAN);

    /* Individual columns */
    int col_w = cell_box_w / 4;
    it_gfx_draw_text_bold(cell_box_x + 20, cy + 12, "[ C-5 ]", IT_COL_WHITE);
    it_gfx_draw_text(cell_box_x + 20, cy + 38, "1. NOTE (Pitch)", IT_COL_CYAN);

    it_gfx_draw_text_bold(cell_box_x + col_w + 20, cy + 12, "[ 01 ]", IT_COL_YELLOW);
    it_gfx_draw_text(cell_box_x + col_w + 20, cy + 38, "2. INSTRUMENT", IT_COL_YELLOW);

    it_gfx_draw_text_bold(cell_box_x + col_w * 2 + 20, cy + 12, "[ v64 ]", IT_COL_GREEN);
    it_gfx_draw_text(cell_box_x + col_w * 2 + 20, cy + 38, "3. VOL / PAN", IT_COL_GREEN);

    it_gfx_draw_text_bold(cell_box_x + col_w * 3 + 20, cy + 12, "[ Z80 ]", (ITColor){ 255, 128, 64, 255 });
    it_gfx_draw_text(cell_box_x + col_w * 3 + 20, cy + 38, "4. EFFECT / CMD", (ITColor){ 255, 128, 64, 255 });

    cy += 88;

    /* Explanations Grid */
    int pw = (w - 60) / 2;
    it_widget_panel(x + 20, cy, pw, 140, "Column 1 & 2: Pitch & Sample");
    it_gfx_draw_text(x + 32, cy + 28, "* Note: Musical pitch (C-0 to B-9).", IT_COL_WHITE);
    it_gfx_draw_text(x + 32, cy + 50, "* Note Cut (^^^): Instantly silences the voice.", IT_COL_CYAN);
    it_gfx_draw_text(x + 32, cy + 72, "* Note Off (===): Releases note with envelope decay.", IT_COL_CYAN);
    it_gfx_draw_text(x + 32, cy + 94, "* Inst (01-99): Selects sample or synth patch.", IT_COL_YELLOW);
    it_gfx_draw_text(x + 32, cy + 116, "* Dots (...): Replays previous instrument at new pitch.", IT_COL_GRAY);

    it_widget_panel(x + 30 + pw, cy, pw, 140, "Column 3 & 4: Dynamics & Commands");
    it_gfx_draw_text(x + 42 + pw, cy + 28, "* Volume: v00 to v64 (silent to maximum).", IT_COL_GREEN);
    it_gfx_draw_text(x + 42 + pw, cy + 50, "* Panning: p00 (hard left) to p64 (hard right).", IT_COL_GREEN);
    it_gfx_draw_text(x + 42 + pw, cy + 72, "* Zxx: Resonant Filter Cutoff (Z00=0Hz, Z7F=open).", (ITColor){ 255, 140, 80, 255 });
    it_gfx_draw_text(x + 42 + pw, cy + 94, "* Gxx: Tone Portamento / Note Glide (acid slides).", IT_COL_WHITE);
    it_gfx_draw_text(x + 42 + pw, cy + 116, "* Dxx: Volume Slide (smooth fades up and down).", IT_COL_WHITE);
}

static void render_step_2(int x, int y, int w, int h) {
    /* Step 2: Interactive Sound & Keyboard Lab */
    it_gfx_draw_text_bold(x + 20, y + 16, "3. INTERACTIVE SOUND & KEYBOARD AUDITION LAB", IT_COL_YELLOW);
    it_gfx_draw_line(x + 20, y + 36, x + w - 20, y + 36, IT_COL_PANEL_BORDER);

    int cy = y + 46;
    it_gfx_draw_text(x + 20, cy, "Click on any piano key below, or press keys on your PC keyboard or MIDI controller to hear live sounds!", IT_COL_WHITE);
    cy += 24;

    ITUIState* ui = it_ui_get_state();
    ITSong* song = it_player_get_song();

    /* Instrument Selector Header */
    it_gfx_draw_text_bold(x + 20, cy + 4, "Audition Instrument:", IT_COL_GRAY);
    char inst_name_buf[64];
    int cur_ins = ui->current_instrument;
    const char* iname = (song && cur_ins <= song->sample_count) ? song->samples[cur_ins - 1].name : "Sample";
    snprintf(inst_name_buf, sizeof(inst_name_buf), "%02d: %s", cur_ins, iname);
    it_gfx_draw_text_bold(x + 190, cy + 4, inst_name_buf, IT_COL_YELLOW);

    if (it_widget_button(x + 450, cy, 26, 22, "-", false)) {
        if (ui->current_instrument > 1) ui->current_instrument--;
    }
    if (it_widget_button(x + 482, cy, 26, 22, "+", false)) {
        if (ui->current_instrument < (song ? song->sample_count : 99)) ui->current_instrument++;
    }

    it_gfx_draw_text(x + 524, cy + 4, "Octave:", IT_COL_GRAY);
    char oct_buf[8];
    snprintf(oct_buf, sizeof(oct_buf), "%d", ui->current_octave);
    it_gfx_draw_text_bold(x + 584, cy + 4, oct_buf, IT_COL_WHITE);
    if (it_widget_button(x + 602, cy, 26, 22, "-", false)) {
        if (ui->current_octave > 0) ui->current_octave--;
    }
    if (it_widget_button(x + 634, cy, 26, 22, "+", false)) {
        if (ui->current_octave < 9) ui->current_octave++;
    }

    cy += 36;

    /* Interactive Virtual 2-Octave Keyboard (17 White Keys + 12 Black Keys) */
    int kb_x = x + 20;
    int kb_y = cy;
    int wk_w = 42;
    int wk_h = 130;
    int bk_w = 26;
    int bk_h = 80;

    /* White keys mapping */
    struct WhiteKey {
        int note_offset;
        const char* note_label;
        const char* pc_key;
    };
    struct WhiteKey white_keys[14] = {
        { 0,  "C", "Z" }, { 2,  "D", "X" }, { 4,  "E", "C" }, { 5,  "F", "V" },
        { 7,  "G", "B" }, { 9,  "A", "N" }, { 11, "B", "M" },
        { 12, "C", "Q" }, { 14, "D", "W" }, { 16, "E", "E" }, { 17, "F", "R" },
        { 19, "G", "T" }, { 21, "A", "Y" }, { 23, "B", "U" }
    };

    uint32_t now = SDL_GetTicks();
    if (now > s_key_flash_timer) s_active_key_flash = -1;

    /* 1. Draw White Keys */
    for (int k = 0; k < 14; k++) {
        int kx = kb_x + k * wk_w;
        int note = 1 + ui->current_octave * 12 + white_keys[k].note_offset;
        bool is_active = (s_active_key_flash == note);

        bool hovered = (ui->mouse_x >= kx && ui->mouse_x < kx + wk_w &&
                        ui->mouse_y >= kb_y && ui->mouse_y < kb_y + wk_h);

        if (hovered && ui->mouse_clicked && !ui->click_consumed) {
            play_preview_note(note, ui->current_instrument);
            ui->click_consumed = true;
        }

        ITColor bg = is_active ? IT_COL_CYAN : (hovered ? (ITColor){ 220, 230, 245, 255 } : IT_COL_WHITE);
        it_gfx_fill_rect(kx, kb_y, wk_w - 2, wk_h, bg);
        it_gfx_draw_rect(kx, kb_y, wk_w - 2, wk_h, (ITColor){ 30, 40, 60, 255 });

        /* Note label & PC key */
        char n_str[16];
        snprintf(n_str, sizeof(n_str), "%s-%d", white_keys[k].note_label, ui->current_octave + (white_keys[k].note_offset >= 12 ? 1 : 0));
        it_gfx_draw_text(kx + 4, kb_y + wk_h - 38, n_str, (ITColor){ 40, 50, 70, 255 });
        it_gfx_draw_text_bold(kx + 12, kb_y + wk_h - 20, white_keys[k].pc_key, (ITColor){ 10, 20, 40, 255 });
    }

    /* 2. Draw Black Keys */
    struct BlackKey {
        int note_offset;
        const char* note_label;
        const char* pc_key;
        int white_key_idx;
    };
    struct BlackKey black_keys[10] = {
        { 1,  "C#", "S", 0 }, { 3,  "D#", "D", 1 },
        { 6,  "F#", "G", 3 }, { 8,  "G#", "H", 4 }, { 10, "A#", "J", 5 },
        { 13, "C#", "2", 7 }, { 15, "D#", "3", 8 },
        { 18, "F#", "5", 10}, { 20, "G#", "6", 11}, { 22, "A#", "7", 12}
    };

    for (int k = 0; k < 10; k++) {
        int kx = kb_x + black_keys[k].white_key_idx * wk_w + (wk_w - bk_w / 2);
        int note = 1 + ui->current_octave * 12 + black_keys[k].note_offset;
        bool is_active = (s_active_key_flash == note);

        bool hovered = (ui->mouse_x >= kx && ui->mouse_x < kx + bk_w &&
                        ui->mouse_y >= kb_y && ui->mouse_y < kb_y + bk_h);

        if (hovered && ui->mouse_clicked && !ui->click_consumed) {
            play_preview_note(note, ui->current_instrument);
            ui->click_consumed = true;
        }

        ITColor bg = is_active ? IT_COL_CYAN : (hovered ? (ITColor){ 50, 70, 100, 255 } : (ITColor){ 16, 22, 34, 255 });
        it_gfx_fill_rect(kx, kb_y, bk_w, bk_h, bg);
        it_gfx_draw_rect(kx, kb_y, bk_w, bk_h, (ITColor){ 60, 80, 110, 255 });

        it_gfx_draw_text(kx + 2, kb_y + bk_h - 28, black_keys[k].note_label, IT_COL_CYAN);
        it_gfx_draw_text_bold(kx + 7, kb_y + bk_h - 14, black_keys[k].pc_key, IT_COL_WHITE);
    }

    cy += wk_h + 20;
    it_widget_panel(x + 20, cy, w - 40, 56, "Hardware MIDI Controller Support");
    it_gfx_draw_text_bold(x + 32, cy + 24, "Plug-and-play:", IT_COL_GREEN);
    it_gfx_draw_text(x + 150, cy + 24, "655 modern USB/MIDI controllers are auto-detected (Novation, Akai, Arturia, Korg).", IT_COL_WHITE);
}

static void render_step_3(int x, int y, int w, int h) {
    /* Step 3: 1-Click Launchpad */
    it_gfx_draw_text_bold(x + 20, y + 16, "4. CHOOSE YOUR LAUNCHPAD: WHAT WOULD YOU LIKE TO DO?", IT_COL_YELLOW);
    it_gfx_draw_line(x + 20, y + 36, x + w - 20, y + 36, IT_COL_PANEL_BORDER);

    int cy = y + 48;
    int card_w = (w - 60) / 2;
    int card_h = 130;

    /* Card 1: Explore Loaded Hit Track in Pattern Editor */
    it_widget_panel(x + 20, cy, card_w, card_h, "1. DIVE INTO THE PATTERN EDITOR [F2]");
    it_gfx_draw_text(x + 32, cy + 28, "Inspect how the active house track is arranged in 64 channels.", IT_COL_WHITE);
    it_gfx_draw_text(x + 32, cy + 48, "Press SPACE to toggle Edit Mode, use arrow keys to navigate.", IT_COL_GRAY);
    if (it_widget_button(x + 32, cy + 80, card_w - 64, 34, "Open Pattern Editor Now [F2]", true)) {
        it_ui_set_page(PAGE_PATTERN);
        it_player_play_song();
    }

    /* Card 2: 20 Real Genre Kits & Demos */
    it_widget_panel(x + 30 + card_w, cy, card_w, card_h, "2. 20 REAL GENRE TRACKS & BEATS [F10]");
    it_gfx_draw_text(x + 42 + card_w, cy + 28, "Load legendary IT tracks: Unreal Tournament DnB, Trance, Techno...", IT_COL_WHITE);
    it_gfx_draw_text(x + 42 + card_w, cy + 48, "Drums on Ch 1-4, Ch 5-64 completely open for your melodies!", IT_COL_GRAY);
    if (it_widget_button(x + 42 + card_w, cy + 80, card_w - 64, 34, "Browse Real Genre Tracks [F10]", false)) {
        it_ui_set_page(PAGE_FILES);
    }

    cy += card_h + 16;

    /* Card 3: MIDI Controller Studio */
    it_widget_panel(x + 20, cy, card_w, card_h, "3. CONNECT MIDI CONTROLLER [F11]");
    it_gfx_draw_text(x + 32, cy + 28, "Check incoming MIDI notes, pitch bend, and live CC signals.", IT_COL_WHITE);
    it_gfx_draw_text(x + 32, cy + 48, "1-click MIDI Learn: bind hardware knobs directly to Cutoff (Zxx)!", IT_COL_GRAY);
    if (it_widget_button(x + 32, cy + 80, card_w - 64, 34, "Open MIDI Controller Studio [F11]", false)) {
        it_ui_set_page(PAGE_SETTINGS);
    }

    /* Card 4: Clean Blank Project */
    it_widget_panel(x + 30 + card_w, cy, card_w, card_h, "4. START BLANK PROJECT (BLANK CANVAS)");
    it_gfx_draw_text(x + 42 + card_w, cy + 28, "Wipe the slate clean and start composing your own track from row 00.", IT_COL_WHITE);
    it_gfx_draw_text(x + 42 + card_w, cy + 48, "Synthesize drums or import your own WAV samples.", IT_COL_GRAY);
    if (it_widget_button(x + 42 + card_w, cy + 80, card_w - 64, 34, "Create New Blank Song", false)) {
        ITSong* song = it_player_get_song();
        if (song) it_song_init_default(song);
        it_ui_set_page(PAGE_PATTERN);
        it_ui_set_status("Started clean new module!");
    }
}

void it_page_wizard_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();

    int wx = 12;
    int wy = 72;
    int ww = screen_w - 24;
    int wh = screen_h - wy - 30;

    /* Main background frame */
    it_gfx_fill_rect(wx, wy, ww, wh, (ITColor){ 24, 30, 42, 255 });
    it_gfx_draw_rect(wx, wy, ww, wh, (ITColor){ 45, 55, 75, 255 });

    /* Top: Now Playing Live Music Bar */
    render_now_playing_banner(wx + 8, wy + 8, ww - 16);

    /* Step Navigation Tabs Bar */
    int tab_y = wy + 46;
    int tab_h = 28;
    int tw = (ww - 24) / 4;
    const char* tab_names[4] = {
        "1. Concepts & Mental Model",
        "2. Pattern Cell Anatomy",
        "3. Live Keyboard Sound Lab",
        "4. 1-Click Launchpad"
    };

    for (int i = 0; i < 4; i++) {
        int tx = wx + 8 + i * (tw + 2);
        if (it_widget_tab(tx, tab_y, tw, tab_h, tab_names[i], s_wizard_step == i)) {
            s_wizard_step = i;
        }
    }

    /* Content Area */
    int content_y = tab_y + tab_h + 8;
    int foot_y = wy + wh - 38;
    int content_h = foot_y - content_y - 6;

    it_gfx_fill_rect(wx + 8, content_y, ww - 16, content_h, (ITColor){ 16, 20, 28, 255 });
    it_gfx_draw_rect(wx + 8, content_y, ww - 16, content_h, (ITColor){ 40, 50, 68, 255 });

    switch (s_wizard_step) {
        case 0: render_step_0(wx + 12, content_y, ww - 24, content_h); break;
        case 1: render_step_1(wx + 12, content_y, ww - 24, content_h); break;
        case 2: render_step_2(wx + 12, content_y, ww - 24, content_h); break;
        case 3: render_step_3(wx + 12, content_y, ww - 24, content_h); break;
    }

    /* Bottom Navigation Controls */
    it_gfx_draw_line(wx, foot_y - 2, wx + ww, foot_y - 2, (ITColor){ 40, 50, 68, 255 });

    /* Previous button */
    if (s_wizard_step > 0) {
        if (it_widget_button(wx + 12, foot_y + 4, 130, 26, "< Previous Step", false)) {
            s_wizard_step--;
        }
    }

    /* Step indicator dots */
    char step_str[32];
    snprintf(step_str, sizeof(step_str), "Step %d of 4", s_wizard_step + 1);
    it_gfx_draw_text_bold(wx + 155, foot_y + 8, step_str, IT_COL_YELLOW);

    /* Show on startup checkbox */
    ITConfig* cfg = it_config_get();
    bool show_start = cfg->show_help_on_startup;
    if (it_widget_checkbox(wx + 260, foot_y + 5, "Show this Quickstart Wizard on startup", &show_start)) {
        cfg->show_help_on_startup = show_start;
        it_config_save(cfg);
    }

    /* Next button */
    if (s_wizard_step < 3) {
        if (it_widget_button(wx + ww - 280, foot_y + 4, 125, 26, "Next Step >", true)) {
            s_wizard_step++;
        }
    }

    /* Launch Tracker button */
    if (it_widget_button(wx + ww - 148, foot_y + 4, 136, 26, "Launch Tracker [F2]", true)) {
        it_ui_set_page(PAGE_PATTERN);
        it_player_play_song();
    }
}

void it_page_wizard_handle_key(SDL_Keycode key, uint16_t mod) {
    if (key == SDLK_LEFT && s_wizard_step > 0) s_wizard_step--;
    else if (key == SDLK_RIGHT && s_wizard_step < 3) s_wizard_step++;
    else if (key == SDLK_RETURN) {
        it_ui_set_page(PAGE_PATTERN);
        it_player_play_song();
    }
}
