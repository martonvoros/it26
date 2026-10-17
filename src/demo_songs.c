#include "demo_songs.h"
#include "util.h"
#include "fmt_it.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const ITGenreInfo s_genres[IT_GENRE_COUNT] = {
    { IT_GENRE_HOUSE,      "House (Classic / Deep)",    124, 6, "Four-on-the-floor groove with crisp 909 hats and pumping energy.", "TR-909 Kick, Snare, 909 Open Hat, Closed Hat, Clap" },
    { IT_GENRE_DNB,        "Drum & Bass (Liquid / Fast)",174, 4, "High-octane 174 BPM syncopated 2-step break with heavy sub punch.", "Punchy DnB Kick, Tight Rim Snare, Ghost Hats, Shaker" },
    { IT_GENRE_BREAKS,     "Breakbeat (Nu-Skool)",       132, 6, "Funky syncopated break with acoustic snares and shuffling hi-hats.", "Subby Kick, Funky Fat Snare, Shuffling Hats, Tambourine" },
    { IT_GENRE_HIPHOP,     "Hip-Hop (Boom-Bap / 90s)",   90,  6, "Laid-back 90 BPM golden-era boom-bap crunch with heavy swing.", "Booming 808 Kick, Hard Snap Snare, Closed Hat, Vinyl Chack" },
    { IT_GENRE_TECHNO,     "Techno (Berlin Peak-Time)",  135, 6, "Driving industrial rumble kick, offbeat claps, and relentless hypnotic drive.", "Rumble 909 Kick, Industrial Clap, Metallic Hat, Perc Stab" },
    { IT_GENRE_TRANCE,     "Trance (Euphoric Uplifting)",138, 6, "Driving 138 BPM kick with 16th rolling sub and bright open hi-hats.", "Punchy Trance Kick, Crisp Snare, Sizzling Open Hat, Shaker" },
    { IT_GENRE_RAVE,       "Rave (90s Oldskool / Break)",152, 5, "Fast 90s rave break with energetic rolls and classic breakbeat accents.", "Hardcore Kick, Sped Snare, High Speed Hats, Tambourine" },
    { IT_GENRE_DUBTECHNO,  "Dub Techno (Echospace)",     122, 6, "Deep warm sub kick surrounded by tape-delayed washed percussion.", "Deep Soft Kick, Rimshot Echo, Filtered Hat, Ambient White" },
    { IT_GENRE_POP,        "Synthpop / Retropop",        120, 6, "Bright 80s LinnDrum style punchy kick and gated snare.", "Linn Kick, Gated Snare, Bright Hats, Electro Perc" },
    { IT_GENRE_RNB,        "Contemporary R&B / Trap",   112, 6, "Gliding sub-bass, snappy finger snaps, and rolling trap hi-hats.", "Deep Sub 808 Kick, Snap/Clap, Trap 16th Hats, Rimshot" },
    { IT_GENRE_AMBIENT,    "Ambient / Downtempo",        82,  8, "Relaxed low-tempo groove with gentle rimshots and warm brushes.", "Soft Warm Kick, Wood Rimshot, Brushed Hat, Sub Pulse" },
    { IT_GENRE_ELECTRO,    "Electro (Drexciya Style)",   128, 6, "Syncopated robot funk with snappy 808 kicks and electronic cowbells.", "TR-808 Kick, Snappy 808 Snare, 808 Hat, Electro Cowbell" },
    { IT_GENRE_GARAGE,     "UK Garage (2-Step Swing)",   134, 6, "Syncopated skipping 2-step rhythm with swinging ghost snares.", "Skipping Kick, 2-Step Snare, Swung Ghost Hat, Woodblock" },
    { IT_GENRE_PSYTRANCE,  "Psytrance / Goa",            144, 5, "Relentless rolling bassline groove with punchy transient kick.", "Laser-sharp Psy Kick, Snappy Snare, Rolling Offbeat Hat, Zap" },
    { IT_GENRE_INDUSTRIAL, "Industrial / EBM",           130, 6, "Distorted punchy kick with metallic percussions and dark energy.", "Overdriven Kick, Metallic Snare, Industrial Hat, Metal Clank" },
    { IT_GENRE_SYNTHWAVE,  "Synthwave / Outrun",         114, 6, "Nostalgic 80s drum machine pulse with gated reverb snap.", "707 Gated Kick, Gated Snare, Tight Hats, Tom Perc" },
    { IT_GENRE_CYBERPUNK,  "Cyberpunk / Midtempo",       105, 6, "Heavy distorted half-time stomp with industrial aggressive punch.", "Distorted Sub Kick, Sledge Snare, Glitch Hat, Noise Click" },
    { IT_GENRE_LOFI,       "Lo-Fi Hip-Hop (Chillhop)",   84,  6, "Dusty swinging vinyl kick with soft laid-back snare and gentle clicks.", "Dusty Soft Kick, Vinyl Snare, Dusty Hat, Vinyl Dust" },
    { IT_GENRE_HARDSTYLE,  "Hardstyle (Raw & Euphoric)", 150, 5, "Distorted pitched 909 kick punch with reverse bass rumble.", "Hardstyle Tok Kick, Punch Snare, Offbeat Hat, Reverse Bass" },
    { IT_GENRE_MINIMAL,    "Minimal / Microhouse",       126, 6, "Intricate micro-percussions, tight clicky rimshots, and subby low-end.", "Minimal Sub Kick, Tight Click Snare, Micro Hat, Wood Click" }
};

int it_demo_get_genre_count(void) {
    return IT_GENRE_COUNT;
}

const ITGenreInfo* it_demo_get_genre_info(int index) {
    if (index < 0 || index >= IT_GENRE_COUNT) return NULL;
    return &s_genres[index];
}

/* Procedural Sample Synthesizers */
static void synth_drum_kick(ITSample* smp, const char* name, float start_f, float end_f, float decay, float dist) {
    int sr = 44100;
    int len = (int)(sr * 0.45f);
    smp->flags = SMP_FLAG_EXISTS;
    smp->length = len;
    smp->loop_start = 0;
    smp->loop_end = 0;
    smp->c5_speed = sr;
    smp->default_vol = 64;
    smp->global_vol = 64;
    strncpy(smp->name, name, sizeof(smp->name) - 1);

    if (smp->data) free(smp->data);
    smp->data = (float*)malloc(len * sizeof(float));

    float phase = 0.0f;
    for (int i = 0; i < len; i++) {
        float t = (float)i / (float)sr;
        float freq = (start_f - end_f) * expf(-t * decay) + end_f;
        phase += 2.0f * (float)M_PI * freq / (float)sr;
        float amp = expf(-t * 8.0f);
        float s = sinf(phase) * amp;

        if (dist > 0.0f) {
            s = tanhf(s * (1.0f + dist * 3.0f));
        }
        smp->data[i] = s * 0.95f;
    }
}

static void synth_drum_snare(ITSample* smp, const char* name, float tone_f, float decay, float noise_mix) {
    int sr = 44100;
    int len = (int)(sr * 0.35f);
    smp->flags = SMP_FLAG_EXISTS;
    smp->length = len;
    smp->loop_start = 0;
    smp->loop_end = 0;
    smp->c5_speed = sr;
    smp->default_vol = 60;
    smp->global_vol = 64;
    strncpy(smp->name, name, sizeof(smp->name) - 1);

    if (smp->data) free(smp->data);
    smp->data = (float*)malloc(len * sizeof(float));

    float phase = 0.0f;
    for (int i = 0; i < len; i++) {
        float t = (float)i / (float)sr;
        float freq = tone_f * expf(-t * 25.0f) + 120.0f;
        phase += 2.0f * (float)M_PI * freq / (float)sr;
        float tone = sinf(phase) * expf(-t * 22.0f);

        float n = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * expf(-t * decay);
        float s = tone * (1.0f - noise_mix) + n * noise_mix;
        smp->data[i] = s * 0.90f;
    }
}

static void synth_drum_hihat(ITSample* smp, const char* name, float decay, bool open) {
    int sr = 44100;
    float dur = open ? 0.40f : 0.08f;
    int len = (int)(sr * dur);
    smp->flags = SMP_FLAG_EXISTS;
    smp->length = len;
    smp->loop_start = 0;
    smp->loop_end = 0;
    smp->c5_speed = sr;
    smp->default_vol = open ? 56 : 48;
    smp->global_vol = 64;
    strncpy(smp->name, name, sizeof(smp->name) - 1);

    if (smp->data) free(smp->data);
    smp->data = (float*)malloc(len * sizeof(float));

    float last_n = 0.0f;
    for (int i = 0; i < len; i++) {
        float t = (float)i / (float)sr;
        float raw_n = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f);
        /* High-pass differentiator for metallic snap */
        float hp_n = raw_n - last_n * 0.75f;
        last_n = raw_n;

        float amp = expf(-t * decay);
        smp->data[i] = hp_n * amp * 0.75f;
    }
}

static void synth_drum_perc(ITSample* smp, const char* name, float freq, float decay) {
    int sr = 44100;
    int len = (int)(sr * 0.15f);
    smp->flags = SMP_FLAG_EXISTS;
    smp->length = len;
    smp->loop_start = 0;
    smp->loop_end = 0;
    smp->c5_speed = sr;
    smp->default_vol = 52;
    smp->global_vol = 64;
    strncpy(smp->name, name, sizeof(smp->name) - 1);

    if (smp->data) free(smp->data);
    smp->data = (float*)malloc(len * sizeof(float));

    float phase = 0.0f;
    for (int i = 0; i < len; i++) {
        float t = (float)i / (float)sr;
        phase += 2.0f * (float)M_PI * freq / (float)sr;
        float tone = sinf(phase) * expf(-t * decay);
        float noise = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * expf(-t * (decay * 2.0f));
        smp->data[i] = (tone * 0.8f + noise * 0.2f) * 0.85f;
    }
}

static void synth_looped_wave(ITSample* smp, const char* name, int type) {
    int len = 1024;
    smp->flags = SMP_FLAG_EXISTS | SMP_FLAG_LOOP;
    smp->length = len;
    smp->loop_start = 0;
    smp->loop_end = len;
    smp->c5_speed = 8363;
    smp->default_vol = 60;
    smp->global_vol = 64;
    strncpy(smp->name, name, sizeof(smp->name) - 1);

    if (smp->data) free(smp->data);
    smp->data = (float*)malloc(len * sizeof(float));

    for (int i = 0; i < len; i++) {
        float phase = (float)i / (float)len;
        if (type == 0) { /* Saw */
            smp->data[i] = 1.0f - 2.0f * phase;
        } else if (type == 1) { /* Pulse 35% */
            smp->data[i] = (phase < 0.35f) ? 0.85f : -0.85f;
        } else if (type == 2) { /* Warm Sine + 2nd harmonic */
            smp->data[i] = sinf(2.0f * (float)M_PI * phase) * 0.75f + sinf(4.0f * (float)M_PI * phase) * 0.25f;
        } else { /* Supersaw dual */
            smp->data[i] = (1.0f - 2.0f * phase) * 0.6f + (1.0f - 2.0f * fmodf(phase * 2.01f, 1.0f)) * 0.4f;
        }
    }
}

static void init_genre_drumkit(ITSong* song, ITGenreId genre) {
    song->sample_count = 8;
    switch (genre) {
        case IT_GENRE_HOUSE:
            synth_drum_kick(&song->samples[1], "909 Punch Kick", 150.0f, 48.0f, 14.0f, 0.2f);
            synth_drum_snare(&song->samples[2], "909 Snare/Clap", 210.0f, 18.0f, 0.75f);
            synth_drum_hihat(&song->samples[3], "909 Closed Hat", 55.0f, false);
            synth_drum_hihat(&song->samples[4], "909 Open Hat", 12.0f, true);
            synth_drum_perc(&song->samples[5], "House Rimshot", 440.0f, 35.0f);
            synth_drum_perc(&song->samples[6], "Perc Shaker", 900.0f, 45.0f);
            break;
        case IT_GENRE_DNB:
            synth_drum_kick(&song->samples[1], "DnB Fast Kick", 170.0f, 55.0f, 22.0f, 0.35f);
            synth_drum_snare(&song->samples[2], "Tight DnB Snare", 260.0f, 26.0f, 0.85f);
            synth_drum_hihat(&song->samples[3], "Ghost Closed Hat", 70.0f, false);
            synth_drum_hihat(&song->samples[4], "DnB Open Hat", 16.0f, true);
            synth_drum_perc(&song->samples[5], "Jungle Rim", 520.0f, 40.0f);
            synth_drum_perc(&song->samples[6], "Roll Shaker", 1100.0f, 50.0f);
            break;
        case IT_GENRE_BREAKS:
            synth_drum_kick(&song->samples[1], "Fat Breaks Kick", 140.0f, 50.0f, 16.0f, 0.3f);
            synth_drum_snare(&song->samples[2], "Funky Snare", 220.0f, 20.0f, 0.70f);
            synth_drum_hihat(&song->samples[3], "Breaks Hat", 60.0f, false);
            synth_drum_hihat(&song->samples[4], "Sizzle Hat", 14.0f, true);
            synth_drum_perc(&song->samples[5], "Tambourine", 800.0f, 30.0f);
            synth_drum_perc(&song->samples[6], "Woodblock", 600.0f, 45.0f);
            break;
        case IT_GENRE_HIPHOP:
            synth_drum_kick(&song->samples[1], "Boom-Bap Kick", 130.0f, 42.0f, 10.0f, 0.45f);
            synth_drum_snare(&song->samples[2], "Vinyl Snare", 190.0f, 15.0f, 0.80f);
            synth_drum_hihat(&song->samples[3], "Dusty Hat", 50.0f, false);
            synth_drum_hihat(&song->samples[4], "Open Sizzle", 10.0f, true);
            synth_drum_perc(&song->samples[5], "Finger Snap", 750.0f, 35.0f);
            synth_drum_perc(&song->samples[6], "Vinyl Chack", 320.0f, 25.0f);
            break;
        case IT_GENRE_TECHNO:
            synth_drum_kick(&song->samples[1], "Berlin Rumble Kick", 160.0f, 40.0f, 12.0f, 0.65f);
            synth_drum_snare(&song->samples[2], "Industrial Clap", 180.0f, 22.0f, 0.85f);
            synth_drum_hihat(&song->samples[3], "Metallic Hat", 75.0f, false);
            synth_drum_hihat(&song->samples[4], "Offbeat Open Hat", 15.0f, true);
            synth_drum_perc(&song->samples[5], "Metal Perc", 950.0f, 30.0f);
            synth_drum_perc(&song->samples[6], "Dark Click", 350.0f, 50.0f);
            break;
        case IT_GENRE_TRANCE:
            synth_drum_kick(&song->samples[1], "Trance Punch Kick", 175.0f, 50.0f, 18.0f, 0.25f);
            synth_drum_snare(&song->samples[2], "Bright Snare", 240.0f, 20.0f, 0.70f);
            synth_drum_hihat(&song->samples[3], "Crisp Hat", 65.0f, false);
            synth_drum_hihat(&song->samples[4], "Uplifting Open Hat", 11.0f, true);
            synth_drum_perc(&song->samples[5], "Trance Shaker", 1000.0f, 45.0f);
            synth_drum_perc(&song->samples[6], "Crash Perc", 700.0f, 20.0f);
            break;
        default:
            synth_drum_kick(&song->samples[1], "Punch Kick", 150.0f, 48.0f, 15.0f, 0.3f);
            synth_drum_snare(&song->samples[2], "Snappy Snare", 210.0f, 18.0f, 0.75f);
            synth_drum_hihat(&song->samples[3], "Closed Hat", 60.0f, false);
            synth_drum_hihat(&song->samples[4], "Open Hat", 14.0f, true);
            synth_drum_perc(&song->samples[5], "Wood Rimshot", 500.0f, 35.0f);
            synth_drum_perc(&song->samples[6], "Electro Shaker", 850.0f, 40.0f);
            break;
    }

    /* Melodic Instruments for full demo tracks */
    synth_looped_wave(&song->samples[7], "Synth Bass", 1);
    synth_looped_wave(&song->samples[8], "Lead Stabs", (genre == IT_GENRE_TRANCE || genre == IT_GENRE_RAVE) ? 3 : 0);
}

/* Helper to program cell */
static void set_cell(ITPattern* pat, int row, int ch, uint8_t note, uint8_t inst, uint8_t vol, uint8_t fx, uint8_t param) {
    ITCell* c = it_pattern_get_cell(pat, row, ch);
    if (!c) return;
    c->note = note;
    c->instrument = inst;
    c->volume = vol;
    c->effect = fx;
    c->param = param;
}

bool it_demo_insert_pattern(ITSong* song, int pattern_idx, ITGenreId genre) {
    if (!song || pattern_idx < 0 || pattern_idx >= IT_MAX_PATTERNS) return false;
    ITPattern* pat = &song->patterns[pattern_idx];
    it_pattern_alloc(pat, 64);

    /* Program genre-specific drums */
    for (int r = 0; r < 64; r++) {
        switch (genre) {
            case IT_GENRE_HOUSE:
            case IT_GENRE_TECHNO:
            case IT_GENRE_TRANCE:
            case IT_GENRE_MINIMAL:
                /* 4 on the floor: Kick on 0, 4, 8, 12... */
                if (r % 4 == 0) {
                    set_cell(pat, r, 0, 61, 1, 64, 0, 0); /* C-5 Kick */
                }
                /* Snare/Clap on 4, 12, 20, 28... (beats 2 & 4) */
                if (r % 8 == 4) {
                    set_cell(pat, r, 1, 61, 2, 60, 0, 0); /* Snare */
                }
                /* Offbeat Open Hihat on 2, 6, 10, 14... */
                if (r % 4 == 2) {
                    set_cell(pat, r, 2, 61, 4, 54, 0, 0); /* Open Hat */
                } else if (r % 2 == 0) {
                    set_cell(pat, r, 2, 61, 3, 40, 0, 0); /* Closed Hat */
                }
                /* Shaker on every 16th row with velocity accents */
                set_cell(pat, r, 3, 61, 6, (r % 2 == 0) ? 44 : 32, 0, 0);
                break;

            case IT_GENRE_DNB:
                /* Fast 2-step break: Kick on 0, 10; Snare on 4, 12 */
                if (r % 16 == 0 || r % 16 == 10) {
                    set_cell(pat, r, 0, 61, 1, 64, 0, 0);
                }
                if (r % 16 == 4 || r % 16 == 12) {
                    set_cell(pat, r, 1, 61, 2, 64, 0, 0);
                }
                /* Fast 16th hats with ghost rolls */
                set_cell(pat, r, 2, 61, (r % 8 == 2 || r % 8 == 6) ? 4 : 3, (r % 2 == 0) ? 50 : 38, 0, 0);
                if (r % 16 == 7 || r % 16 == 15) {
                    set_cell(pat, r, 3, 61, 5, 42, 0, 0); /* Ghost rim */
                }
                break;

            case IT_GENRE_BREAKS:
            case IT_GENRE_GARAGE:
                /* Syncopated breaks: Kick on 0, 6, 10; Snare on 4, 12 */
                if (r % 16 == 0 || r % 16 == 6 || r % 16 == 10) {
                    set_cell(pat, r, 0, 61, 1, 64, 0, 0);
                }
                if (r % 16 == 4 || r % 16 == 12) {
                    set_cell(pat, r, 1, 61, 2, 62, 0, 0);
                }
                if (r % 4 == 2) {
                    set_cell(pat, r, 2, 61, 4, 52, 0, 0);
                } else {
                    set_cell(pat, r, 2, 61, 3, (r % 2 == 0) ? 46 : 34, 0, 0);
                }
                break;

            case IT_GENRE_HIPHOP:
            case IT_GENRE_LOFI:
                /* Boom-Bap 90 BPM: Kick on 0, 10, 22; Snare on 8, 24 */
                if (r % 32 == 0 || r % 32 == 10 || r % 32 == 22) {
                    set_cell(pat, r, 0, 61, 1, 64, 0, 0);
                }
                if (r % 32 == 8 || r % 32 == 24) {
                    set_cell(pat, r, 1, 61, 2, 62, 0, 0);
                }
                if (r % 2 == 0) {
                    set_cell(pat, r, 2, 61, (r % 8 == 6) ? 4 : 3, 48, 0, 0);
                }
                break;

            case IT_GENRE_RAVE:
            case IT_GENRE_HARDSTYLE:
                /* Relentless kick & offbeat drive */
                if (r % 4 == 0) {
                    set_cell(pat, r, 0, 61, 1, 64, 0, 0);
                }
                if (r % 8 == 4) {
                    set_cell(pat, r, 1, 61, 2, 64, 0, 0);
                }
                if (r % 4 == 2) {
                    set_cell(pat, r, 2, 61, 4, 60, 0, 0);
                }
                set_cell(pat, r, 3, 61, 6, 40, 0, 0);
                break;

            default:
                if (r % 4 == 0) set_cell(pat, r, 0, 61, 1, 64, 0, 0);
                if (r % 8 == 4) set_cell(pat, r, 1, 61, 2, 60, 0, 0);
                if (r % 2 == 0) set_cell(pat, r, 2, 61, (r % 4 == 2) ? 4 : 3, 46, 0, 0);
                break;
        }
    }
    return true;
}

bool it_demo_load_drum_groove(ITSong* song, ITGenreId genre) {
    if (!song) return false;
    if (genre < 0 || genre >= IT_GENRE_COUNT) genre = IT_GENRE_HOUSE;

    const ITGenreInfo* info = &s_genres[genre];
    it_song_clear(song);

    snprintf(song->name, sizeof(song->name), "IT %s (Drum Beat)", info->name);
    song->initial_tempo = info->bpm;
    song->initial_speed = info->speed;
    song->global_vol = 128;
    song->mix_vol = 48;
    song->order_count = 1;
    song->orders[0] = 0;
    song->pattern_count = 1;

    /* Initialize samples */
    init_genre_drumkit(song, genre);

    /* Generate drum groove into pattern 0 */
    it_demo_insert_pattern(song, 0, genre);

    /* Channels 1..4: Active drums; Channels 5..64: UNMUTED & READY FOR USER WRITING */
    for (int c = 0; c < IT_MAX_CHANNELS; c++) {
        song->channel_vol[c] = 64;
        song->channel_mute[c] = (c >= 16); /* First 16 channels active */
        if (c == 0) song->channel_pan[c] = 32; /* Kick center */
        else if (c == 1) song->channel_pan[c] = 32; /* Snare center */
        else if (c == 2) song->channel_pan[c] = 20; /* Hats slightly left */
        else if (c == 3) song->channel_pan[c] = 44; /* Percs right */
        else song->channel_pan[c] = (c % 2 == 0) ? 24 : 40;
    }

    /* Attach helpful message */
    char msg[1024];
    snprintf(msg, sizeof(msg),
        "=========================================================================\n"
        "  GENRE: %s (%d BPM) - DRUM GROOVE READY TO WRITE ON\n"
        "=========================================================================\n\n"
        "CHANNELS 1-4 : Looping %s drum groove.\n"
        "CHANNELS 5-64: COMPLETELY FREE FOR YOUR COMPOSITION!\n\n"
        "HOW TO WRITE OVER THIS BEAT:\n"
        "  1. Press SPACE to toggle Edit Mode.\n"
        "  2. Move cursor to Channel 5 (Tab or Arrow Keys).\n"
        "  3. Play your MIDI Controller or type notes on your PC keyboard (Z..M, Q..U).\n"
        "  4. Press F5 to play with your new melodies!\n",
        info->name, info->bpm, info->name);
    song->message = (char*)malloc(strlen(msg) + 1);
    if (song->message) strcpy(song->message, msg);

    return true;
}

static const char* s_genre_it_files[IT_GENRE_COUNT] = {
    "assets/house_of_funk.it",                        /* IT_GENRE_HOUSE (DJ-Aldav: House of Fame) */
    "assets/demos/foregone.it",                        /* IT_GENRE_DNB (Michiel van den Bos - Unreal Tournament Foregone Destruction) */
    "assets/demos/a-000003.it",                        /* IT_GENRE_BREAKS */
    "assets/demos/adfchips.it",                        /* IT_GENRE_HIPHOP (Funky Chiptune Boom-Bap) */
    "assets/demos/ash_technodancer.it",                /* IT_GENRE_TECHNO (Techno Dancer) */
    "assets/demos/a-lin_trancen.it",                   /* IT_GENRE_TRANCE (Euphoric Trance) */
    "assets/demos/82190-rave.it",                      /* IT_GENRE_RAVE (90s Oldskool Rave) */
    "assets/demos/dubbel_part_1.it",                   /* IT_GENRE_DUBTECHNO (Dub Techno) */
    "assets/demos/a-9n3.it",                           /* IT_GENRE_POP (Synthpop) */
    "assets/demos/surasshu_-_eek.it",                  /* IT_GENRE_RNB (Surasshu - Eek) */
    "assets/demos/ambiant_.it",                        /* IT_GENRE_AMBIENT (Ambient Works) */
    "assets/demos/acsid_electrobeat.it",               /* IT_GENRE_ELECTRO (Electro Beat) */
    "assets/demos/81590-future.it",                    /* IT_GENRE_GARAGE (Future Garage) */
    "assets/demos/aacid.it",                           /* IT_GENRE_PSYTRANCE (Acid Goa) */
    "assets/demos/aeuk_-_cyber_heavey_punk_slab.it",   /* IT_GENRE_INDUSTRIAL (Cyber Heavy Punk Slab) */
    "assets/demos/a-000005.it",                        /* IT_GENRE_SYNTHWAVE (Synth Kings) */
    "assets/demos/aeuk_-_cyber_heavey_punk_slab.it",   /* IT_GENRE_CYBERPUNK (Cyberpunk Midtempo) */
    "assets/demos/a-windf.it",                         /* IT_GENRE_LOFI (Skaven: Windfall) */
    "assets/dead_lock.it",                             /* IT_GENRE_HARDSTYLE (Elwood: Dead Lock) */
    "assets/demos/01_space_light.it"                   /* IT_GENRE_MINIMAL (Space Light Minimal) */
};

bool it_demo_load_full_song(ITSong* song, ITGenreId genre) {
    if (!song) return false;
    if (genre < 0 || genre >= IT_GENRE_COUNT) genre = IT_GENRE_HOUSE;

    const ITGenreInfo* info = &s_genres[genre];

    /* First attempt: Load authentic real IT track if available */
    if (s_genre_it_files[genre] && it_load_module_any(song, s_genre_it_files[genre])) {
        return true;
    }

    /* Fallback: Load drum groove first */
    it_demo_load_drum_groove(song, genre);
    snprintf(song->name, sizeof(song->name), "IT %s (Full Demo)", info->name);

    ITPattern* pat = &song->patterns[0];

    /* Now compose musical Bassline on Channel 5 and Leads on Channel 6 */
    /* Bassline notes (C, Eb, F, G minor groove) */
    uint8_t bass_notes[16] = { 37, 0, 37, 40,  0, 42, 0, 43,  37, 0, 37, 40,  45, 0, 43, 0 }; /* C-3 based */
    for (int r = 0; r < 64; r++) {
        uint8_t bn = bass_notes[r % 16];
        if (bn > 0) {
            /* Bass on channel 4 (0-indexed = Channel 5) */
            /* Add resonant filter cutoff effect Zxx (Z30..Z70) */
            uint8_t cutoff = 0x20 + (uint8_t)((r * 2) % 0x50);
            set_cell(pat, r, 4, bn, 7, 60, 'Z', cutoff);
        }
    }

    /* Lead Melody / Chord Stabs on Channel 6 (0-indexed = Channel 5) */
    uint8_t lead_notes[16] = { 61, 0, 64, 0,  66, 0, 67, 0,  71, 0, 67, 0,  66, 64, 61, 0 }; /* C-5 based */
    for (int r = 0; r < 64; r++) {
        if (r % 4 == 0) {
            uint8_t ln = lead_notes[(r / 4) % 16];
            if (ln > 0) {
                /* Channel 5 (0-indexed = Ch 6): Lead note + vibrato H42 */
                set_cell(pat, r, 5, ln, 8, 54, 'H', 0x42);
            }
        }
    }

    /* Arpeggio Accent on Channel 7 (0-indexed = Channel 6) */
    for (int r = 0; r < 64; r++) {
        if (r % 8 == 2 || r % 8 == 6) {
            /* Note C-6 with Volume Slide D02 */
            set_cell(pat, r, 6, 73, 8, 48, 'D', 0x02);
        }
    }

    /* Update Message */
    char msg[1024];
    snprintf(msg, sizeof(msg),
        "=========================================================================\n"
        "  FULL MUSICAL DEMONSTRATION: %s (%d BPM)\n"
        "=========================================================================\n\n"
        "CHANNELS 1-4 : %s\n"
        "CHANNEL 5    : Resonant Filter Swept Bassline (Sample 7, 'Z' effects)\n"
        "CHANNEL 6    : Melodic Lead with Real-time Vibrato (Sample 8, 'H' effects)\n"
        "CHANNEL 7    : Arpeggiated Chord Accents with Volume Slides ('D' effects)\n"
        "CHANNELS 8-64: OPEN FOR YOU TO ADD EXTRA TRACKS!\n\n"
        "Press F5 to enjoy the playback, or turn on your MIDI controller to play along!\n",
        info->name, info->bpm, info->drum_kit_info);
    if (song->message) free(song->message);
    song->message = (char*)malloc(strlen(msg) + 1);
    if (song->message) strcpy(song->message, msg);

    return true;
}
