#include "page_instruments.h"
#include "gfx.h"
#include "widgets.h"
#include "ui.h"
#include "player.h"

static int s_selected_inst = 1;
static int s_selected_env = 0; /* 0=Vol, 1=Pan, 2=Pitch/Filter */

void it_page_instruments_init(void) {
    s_selected_inst = 1;
    s_selected_env = 0;
}

void it_page_instruments_render(void) {
    int screen_w = it_gfx_get_logical_width();
    int screen_h = it_gfx_get_logical_height();
    ITSong* song = it_player_get_song();
    ITUIState* ui = it_ui_get_state();
    if (!song) return;

    int panel_y = 62;
    int panel_h = screen_h - panel_y - 28;

    /* Left Panel: Instrument List */
    int list_w = 260;
    it_widget_panel(10, panel_y, list_w, panel_h, "INSTRUMENTS (1..99) [F4]");

    static int s_inst_scroll = 0;
    int visible_items = (panel_h - 40) / 22;
    if (visible_items < 1) visible_items = 1;

    /* Mouse wheel on instrument list */
    if (ui->wheel_y != 0 && ui->mouse_x >= 10 && ui->mouse_x < 10 + list_w &&
        ui->mouse_y >= panel_y && ui->mouse_y < panel_y + panel_h) {
        s_inst_scroll = IT_CLAMP(s_inst_scroll - ui->wheel_y * 2, 0, 99 - visible_items);
    }

    int list_y = panel_y + 30;
    for (int idx = 0; idx < visible_items; idx++) {
        int i = 1 + s_inst_scroll + idx;
        if (i > 99) break;

        int item_y = list_y + idx * 22;
        bool is_sel = (i == s_selected_inst);
        ITInstrument* inst = &song->instruments[i];

        if (is_sel) {
            it_gfx_fill_rect(14, item_y, list_w - 24, 20, IT_COL_ROW_HILIGHT2);
        }

        char item_str[48];
        snprintf(item_str, sizeof(item_str), "%02d: %s", i, (strlen(inst->name) > 0) ? inst->name : "<empty>");
        it_gfx_draw_text(18, item_y + 2, item_str, is_sel ? IT_COL_CYAN : IT_COL_WHITE);

        if (ui->mouse_clicked && !ui->click_consumed &&
            ui->mouse_x >= 14 && ui->mouse_x < 14 + list_w - 24 &&
            ui->mouse_y >= item_y && ui->mouse_y < item_y + 20) {
            s_selected_inst = i;
            ui->current_instrument = i;
            ui->click_consumed = true;
        }

        if (ui->mouse_double_clicked && !ui->click_consumed &&
            ui->mouse_x >= 14 && ui->mouse_x < 14 + list_w - 24 &&
            ui->mouse_y >= item_y && ui->mouse_y < item_y + 20) {
            s_selected_inst = i;
            ui->current_instrument = i;
            it_player_trigger_note(0, 61, i, 64);
            ui->click_consumed = true;
        }
    }

    /* Scrollbar for instrument list */
    it_widget_scrollbar(10 + list_w - 12, list_y, 10, panel_h - 40, 99, visible_items, &s_inst_scroll);

    /* Right Panel: Instrument Parameters & Envelope Editor */
    int edit_x = 10 + list_w + 6;
    int edit_w = screen_w - edit_x - 10;
    it_widget_panel(edit_x, panel_y, edit_w, panel_h, "INSTRUMENT & GRAPHICAL ENVELOPE EDITOR");

    ITInstrument* inst = &song->instruments[s_selected_inst];

    /* Info header */
    char hdr[128];
    static const char* s_nna_names[4] = { "Cut", "Continue", "Note Off", "Note Fade" };
    snprintf(hdr, sizeof(hdr), "Instrument %02d: %s | NNA: %s | Fadeout: %u",
             s_selected_inst, inst->name, s_nna_names[inst->nna & 3], inst->fadeout);
    it_gfx_draw_text_bold(edit_x + 16, panel_y + 30, hdr, IT_COL_YELLOW);

    /* Envelope Tabs */
    int env_tab_x = edit_x + 16;
    int env_tab_y = panel_y + 56;
    if (it_widget_tab(env_tab_x, env_tab_y, 110, 26, "Volume Env", s_selected_env == 0)) s_selected_env = 0;
    if (it_widget_tab(env_tab_x + 116, env_tab_y, 110, 26, "Panning Env", s_selected_env == 1)) s_selected_env = 1;
    if (it_widget_tab(env_tab_x + 232, env_tab_y, 130, 26, "Pitch/Filter Env", s_selected_env == 2)) s_selected_env = 2;

    /* Envelope Canvas */
    int canvas_x = edit_x + 16;
    int canvas_y = panel_y + 88;
    int canvas_w = edit_w - 32;
    int canvas_h = 160;

    it_gfx_fill_rect(canvas_x, canvas_y, canvas_w, canvas_h, IT_COL_PANEL_DARK);
    it_gfx_draw_rect(canvas_x, canvas_y, canvas_w, canvas_h, IT_COL_PANEL_BORDER);

    ITEnvelope* env = (s_selected_env == 0) ? &inst->vol_env :
                      ((s_selected_env == 1) ? &inst->pan_env : &inst->pitch_env);

    /* Grid lines */
    for (int gy = 0; gy <= 4; gy++) {
        int ly = canvas_y + gy * (canvas_h / 4);
        it_gfx_draw_line(canvas_x, ly, canvas_x + canvas_w, ly, IT_COL_ROW_HILIGHT1);
    }

    /* Draw envelope curve and node points */
    if (env->num_nodes >= 2) {
        float max_t = (float)env->nodes[env->num_nodes - 1].tick;
        if (max_t <= 0.0f) max_t = 100.0f;

        for (int n = 0; n < env->num_nodes - 1; n++) {
            float x0 = (float)env->nodes[n].tick / max_t * (float)(canvas_w - 40) + 20.0f;
            float y0 = (float)(canvas_h - 20) - ((float)env->nodes[n].y / 64.0f * (float)(canvas_h - 40));
            float x1 = (float)env->nodes[n + 1].tick / max_t * (float)(canvas_w - 40) + 20.0f;
            float y1 = (float)(canvas_h - 20) - ((float)env->nodes[n + 1].y / 64.0f * (float)(canvas_h - 40));

            it_gfx_draw_line(canvas_x + (int)x0, canvas_y + (int)y0,
                             canvas_x + (int)x1, canvas_y + (int)y1, IT_COL_CYAN);
            it_gfx_fill_rect(canvas_x + (int)x0 - 4, canvas_y + (int)y0 - 4, 8, 8, IT_COL_YELLOW);
        }
        float last_x = (float)(canvas_w - 20);
        float last_y = (float)(canvas_h - 20) - ((float)env->nodes[env->num_nodes - 1].y / 64.0f * (float)(canvas_h - 40));
        it_gfx_fill_rect(canvas_x + (int)last_x - 4, canvas_y + (int)last_y - 4, 8, 8, IT_COL_YELLOW);
    } else {
        it_gfx_draw_text(canvas_x + canvas_w / 2 - 80, canvas_y + canvas_h / 2 - 8, "Envelope Empty / Disabled", IT_COL_DARK_GRAY);
    }

    /* Envelope controls */
    int ctrl_y = canvas_y + canvas_h + 16;
    bool env_on = (env->flags & ENV_FLAG_ON) != 0;
    if (it_widget_checkbox(canvas_x, ctrl_y, "Enable Envelope", &env_on)) {
        if (env_on) env->flags |= ENV_FLAG_ON;
        else env->flags &= ~ENV_FLAG_ON;
    }

    bool env_loop = (env->flags & ENV_FLAG_LOOP) != 0;
    if (it_widget_checkbox(canvas_x + 180, ctrl_y, "Loop Envelope", &env_loop)) {
        if (env_loop) env->flags |= ENV_FLAG_LOOP;
        else env->flags &= ~ENV_FLAG_LOOP;
    }

    /* NNA selector */
    int nna_y = ctrl_y + 36;
    it_gfx_draw_text(canvas_x, nna_y + 4, "New Note Action (NNA):", IT_COL_GRAY);
    if (it_widget_button(canvas_x + 200, nna_y, 80, 26, "Cut", inst->nna == NNA_CUT)) inst->nna = NNA_CUT;
    if (it_widget_button(canvas_x + 286, nna_y, 90, 26, "Continue", inst->nna == NNA_CONTINUE)) inst->nna = NNA_CONTINUE;
    if (it_widget_button(canvas_x + 382, nna_y, 90, 26, "Note Off", inst->nna == NNA_NOTE_OFF)) inst->nna = NNA_NOTE_OFF;
    if (it_widget_button(canvas_x + 478, nna_y, 90, 26, "Note Fade", inst->nna == NNA_NOTE_FADE)) inst->nna = NNA_NOTE_FADE;

    /* Filter Cutoff & Resonance sliders */
    int flt_y = nna_y + 36;
    int cutoff = inst->filter_cutoff;
    if (it_widget_slider(canvas_x, flt_y, 220, 24, "Filter Cutoff", &cutoff, 0, 127)) {
        inst->filter_cutoff = (uint8_t)cutoff;
    }

    int reso = inst->filter_resonance;
    if (it_widget_slider(canvas_x + 360, flt_y, 220, 24, "Resonance", &reso, 0, 127)) {
        inst->filter_resonance = (uint8_t)reso;
    }
}
