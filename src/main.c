#include "common.h"
#include <SDL3/SDL_main.h>
#include "config.h"
#include "gfx.h"
#include "song.h"
#include "player.h"
#include "mixer.h"
#include "audio.h"
#include "midi.h"
#include "midi_profiles.h"
#include "midi_learn.h"
#include "ui.h"
#include "fmt_it.h"

int main(int argc, char* argv[]) {
    /* Initialize SDL3 subsystems */
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        SDL_Log("Failed to initialize SDL3: %s", SDL_GetError());
        return 1;
    }

    /* Load user preferences */
    ITConfig* cfg = it_config_get();
    it_config_load(cfg);

    /* Create high-DPI capable window */
    int win_w = (int)(IT_LOGICAL_WIDTH * cfg->ui_scale);
    int win_h = (int)(IT_LOGICAL_HEIGHT * cfg->ui_scale);

    SDL_Window* window = SDL_CreateWindow(
        "Impulse Tracker Modern - [64-bit Workstation]",
        win_w, win_h,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (!window) {
        SDL_Log("Failed to create SDL window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetWindowMinimumSize(window, 800, 500);

    /* Create hardware-accelerated renderer */
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Failed to create SDL renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    /* Initialize Graphics System */
    if (!it_gfx_init(window, renderer)) {
        SDL_Log("Failed to initialize vector font graphics system");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    it_gfx_set_scale(cfg->ui_scale);

    /* Load IT26 Bootscreen Image */
    it_gfx_load_bootscreen("head.jpg");

    /* Initialize Starter Song */
    ITSong* song = it_song_create();
    it_song_init_default(song);

    /* Load real, complex classic house track on startup (House Of Heaven - 86 inst, 65 smp, 72 pat) */
    bool startup_loaded = false;
    if (it_load_module_any(song, "assets/hard_house.it") ||
        it_load_module_any(song, "assets/demos/foregone.it") ||
        it_load_module_any(song, "assets/groovetasticola.it")) {
        startup_loaded = true;
    }

    /* Initialize Mixer & Player */
    it_mixer_init(cfg->sample_rate);
    it_player_init(song, cfg->sample_rate);

    /* Initialize Audio Driver */
    if (!it_audio_init(cfg->sample_rate)) {
        SDL_Log("Warning: Audio driver could not be opened; running in silent mode.");
    }

    /* Autoplay iconic hit track on startup */
    if (startup_loaded) {
        it_player_play_song();
    }

    /* Initialize MIDI Input Subsystems & Profiles */
    it_midi_init();
    it_midi_profiles_init();
    it_midi_learn_init();

    /* Initialize UI Coordinator */
    it_ui_init();

    /* Check if a module was passed on command line */
    if (argc > 1 && argv[1]) {
        if (it_load_module_any(song, argv[1])) {
            it_ui_set_status("Loaded module from command line: %s", argv[1]);
            it_ui_set_page(PAGE_PATTERN);
            it_player_play_song();
        }
    }

    /* Main Application Event Loop */
    bool running = true;
    uint64_t last_frame_time = SDL_GetTicks();

    while (running) {
        /* Process all incoming system and user events */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }
            if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED || e.type == SDL_EVENT_WINDOW_RESIZED) {
                it_gfx_handle_resize();
            }
            it_ui_handle_event(&e);
        }

        ITUIState* ui = it_ui_get_state();
        if (ui->request_quit) {
            running = false;
        }

        /* Process hardware MIDI controller events (Notes, CCs, Pitch Bend) */
        it_midi_process();

        /* Render entire UI */
        it_gfx_begin_frame();
        it_ui_render();
        it_gfx_end_frame();

        /* Update frame state at the end of the frame */
        it_ui_update();

        /* Frame Rate Limiter (~60 FPS) */
        uint64_t now = SDL_GetTicks();
        uint64_t elapsed = now - last_frame_time;
        if (elapsed < 16) {
            SDL_Delay((uint32_t)(16 - elapsed));
        }
        last_frame_time = SDL_GetTicks();
    }

    /* Clean Shutdown */
    it_audio_shutdown();
    it_midi_shutdown();
    it_ui_shutdown();
    it_song_destroy(song);
    it_gfx_shutdown();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
