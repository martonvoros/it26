#include "demo_songs.h"
#include "util.h"
#include "fmt_it.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const ITGenreInfo s_genres[IT_GENRE_COUNT] = {
    { IT_GENRE_HOUSE,      "House of Heaven (Xtended)", 124, 6, "Alexander Brandon / Funky Grooves - Classic French Touch & Chicago House.", "86 Instruments, 65 Real SB16/CD Samples, 72 Patterns, 45 Orders" },
    { IT_GENRE_DNB,        "Foregone Destruction (Unreal Tournament)", 140, 5, "Michiel van den Bos - The iconic Facing Worlds CTF theme from Unreal Tournament.", "31 Real SB16 WAV Samples, 32 Patterns, 45 Orders (C-Minor Strings & Break)" },
    { IT_GENRE_BREAKS,     "Breaking The Barriers", 132, 6, "Aeuk - Heavy nu-skool big beat funk breaks with acoustic snares and fat riffs.", "49 Real Breakbeat Samples, 28 Patterns, 37 Orders" },
    { IT_GENRE_HIPHOP,     "Groovetasticola Funk", 90,  6, "Golden-era boom-bap crunch with live swinging funk bass and dusty vinyl chops.", "17 Real Drum & Funk Samples, 10 Patterns, 256 Orders" },
    { IT_GENRE_TECHNO,     "Beyond Techno Peak-Time", 135, 6, "Berlin peak-time industrial driving rumble kick with relentless hypnotic energy.", "13 Real Industrial Samples, 6 Patterns, 8 Orders" },
    { IT_GENRE_TRANCE,     "Trance After Tranquility", 138, 6, "Euphoric uplifting 138 BPM melodic trance odyssey with soaring supersaw leads.", "37 Real Instruments, 35 Samples, 34 Patterns, 58 Orders" },
    { IT_GENRE_RAVE,       "Rave-Type Thing", 152, 5, "Fast 90s oldskool rave breakbeat hardcore with sped amen rolls and ragga bass.", "18 Real Instruments, 18 Samples, 22 Patterns, 35 Orders" },
    { IT_GENRE_DUBTECHNO,  "Ambient Blank Dub", 122, 6, "Echospace deep analog tape-delayed chords surrounded by washed sub percussion.", "12 Real Samples, 8 Patterns, 9 Orders" },
    { IT_GENRE_POP,        "Dream State Synthpop", 120, 6, "Bright 80s nostalgic LinnDrum groove, gated snares, and singing analog melody.", "18 Real Samples, 28 Patterns, 29 Orders" },
    { IT_GENRE_RNB,        "Eek! (Surasshu)", 112, 6, "Surasshu - Contemporary funky R&B, gliding 808 sub bass, and crisp neo-soul chops.", "19 Real Instruments, 19 Samples, 12 Patterns, 16 Orders" },
    { IT_GENRE_AMBIENT,    "The Promise", 82,  8, "Basehead - Deep cinematic evolving atmospheric soundscapes and warm acoustic brushes.", "53 Real Instruments, 53 Samples, 53 Patterns, 62 Orders" },
    { IT_GENRE_ELECTRO,    "Electro Beat 2", 128, 6, "Drexciya & Kraftwerk style syncopated robot funk with TR-808 cowbells and vocoder bass.", "7 Real Instruments, 6 Samples, 17 Patterns, 23 Orders" },
    { IT_GENRE_GARAGE,     "Quality Time (Big Money Mix)", 134, 6, "Skaven - Authentic UK Garage 2-step swinging ghost snares and pitched organ chords.", "40 Real Instruments, 33 Samples, 82 Patterns, 101 Orders" },
    { IT_GENRE_PSYTRANCE,  "Indigo Acid", 144, 5, "Basehead - Relentless rolling Goa psytrance bassline groove with acid 303 lasers.", "26 Real Instruments, 62 Samples, 42 Patterns, 57 Orders" },
    { IT_GENRE_INDUSTRIAL, "Seeker (Unreal Tournament)", 130, 6, "Basehead - The heavy dark electronic industrial combat theme from Unreal Tournament.", "22 Real Industrial Samples, 11 Patterns, 37 Orders" },
    { IT_GENRE_SYNTHWAVE,  "1997 Hornet Upgrade", 114, 6, "Necros - Legendary FM & analog 80s Outrun synthwave showcase with gated drums.", "26 Real Samples, 19 Patterns, 25 Orders" },
    { IT_GENRE_CYBERPUNK,  "Clint Cyber DNB", 170, 6, "Heavy cyberpunk midtempo/neuro bass stomps with aggressive sub distortion.", "19 Real Instruments, 19 Samples, 63 Patterns, 100 Orders" },
    { IT_GENRE_LOFI,       "Winds of Fjords", 84,  6, "Skaven - World-renowned melodic chillhop masterpiece with warm jazz flute melodies.", "15 Real Instruments, 15 Samples, 20 Patterns, 26 Orders" },
    { IT_GENRE_HARDSTYLE,  "Techno Dancer", 150, 5, "Ash - Relentless hard dance reverse bass rumble with anthemic lead screeches.", "18 Real Instruments, 11 Samples, 14 Patterns, 35 Orders" },
    { IT_GENRE_MINIMAL,    "Skyward Fire DNB", 168, 6, "Skyward Progression - Intricate liquid minimal drum & bass with deep warm subs.", "11 Real Samples, 28 Patterns, 66 Orders" }
};

static const char* s_genre_it_files[IT_GENRE_COUNT] = {
    "assets/hard_house.it",                                     /* IT_GENRE_HOUSE (House Of Heaven) */
    "assets/demos/foregone.it",                                 /* IT_GENRE_DNB (Foregone Destruction - UT Facing Worlds) */
    "assets/demos/aeuk_-_breaking_the_barriers.it",             /* IT_GENRE_BREAKS (Breaking The Barriers) */
    "assets/groovetasticola.it",                                /* IT_GENRE_HIPHOP (Groovetasticola Funk) */
    "assets/demos/beyond_-_techno.it",                          /* IT_GENRE_TECHNO (Beyond Techno Peak-Time) */
    "assets/demos/a-trance.it",                                 /* IT_GENRE_TRANCE (Euphoric Trance Odyssey) */
    "assets/demos/82190-rave.it",                               /* IT_GENRE_RAVE (90s Oldskool Rave Storm) */
    "assets/demos/ambient_blank.it",                            /* IT_GENRE_DUBTECHNO (Ambient Blank Dub Space) */
    "assets/demos/a-9n3.it",                                    /* IT_GENRE_POP (Dream State Synthpop) */
    "assets/demos/surasshu_-_eek.it",                           /* IT_GENRE_RNB (Surasshu - Eek!) */
    "assets/demos/basehead_-_the_promise.it",                   /* IT_GENRE_AMBIENT (Basehead: The Promise) */
    "assets/demos/acsid_electrobeat2.it",                       /* IT_GENRE_ELECTRO (Electro Beat 2) */
    "assets/demos/skaven_-_quality_time_-_big_money_mix_v2.it", /* IT_GENRE_GARAGE (Skaven: Quality Time Big Money Mix) */
    "assets/demos/basehead_-_indigo.it",                        /* IT_GENRE_PSYTRANCE (Basehead: Indigo Acid) */
    "assets/demos/basehead_-_unreal_tournament_-_seeker.it",    /* IT_GENRE_INDUSTRIAL (Basehead: Seeker Unreal Tournament) */
    "assets/demos/necros_-_1997_hornet_multimedia_upgrade.it",  /* IT_GENRE_SYNTHWAVE (Necros: Hornet Multimedia Upgrade) */
    "assets/demos/dnb_clint.it",                                /* IT_GENRE_CYBERPUNK (Clint Cyber Bass) */
    "assets/demos/a-windf.it",                                  /* IT_GENRE_LOFI (Skaven: Winds of Fjords) */
    "assets/demos/ash_technodancer.it",                         /* IT_GENRE_HARDSTYLE (Ash: Techno Dancer) */
    "assets/demos/dnbskyward.it"                                /* IT_GENRE_MINIMAL (Skyward Minimal Progression) */
};

int it_demo_get_genre_count(void) {
    return IT_GENRE_COUNT;
}

const ITGenreInfo* it_demo_get_genre_info(int index) {
    if (index < 0 || index >= IT_GENRE_COUNT) return NULL;
    return &s_genres[index];
}

/* 100% Authentic Real Track Loader: Loads real multi-channel module with SB16+ samples */
bool it_demo_load_full_song(ITSong* song, ITGenreId genre) {
    if (!song) return false;
    if (genre < 0 || genre >= IT_GENRE_COUNT) genre = IT_GENRE_HOUSE;

    const char* filepath = s_genre_it_files[genre];
    if (!filepath) return false;

    it_song_clear(song);

    if (it_load_module_any(song, filepath)) {
        return true;
    }
    return false;
}

bool it_demo_load_drum_groove(ITSong* song, ITGenreId genre) {
    return it_demo_load_full_song(song, genre);
}

/* Insert a pattern from the real track into the active song */
bool it_demo_insert_pattern(ITSong* song, int pattern_idx, ITGenreId genre) {
    if (!song || pattern_idx < 0 || pattern_idx >= IT_MAX_PATTERNS) return false;
    if (genre < 0 || genre >= IT_GENRE_COUNT) genre = IT_GENRE_HOUSE;

    const char* path = s_genre_it_files[genre];
    if (!path) return false;

    ITSong temp_song;
    memset(&temp_song, 0, sizeof(temp_song));
    if (!it_load_file(&temp_song, path)) return false;

    int src_pat = (temp_song.order_count > 0 && temp_song.orders[0] < temp_song.pattern_count) ? temp_song.orders[0] : 0;
    ITPattern* src = &temp_song.patterns[src_pat];
    ITPattern* dst = &song->patterns[pattern_idx];
    it_pattern_alloc(dst, src->rows > 0 ? src->rows : 64);

    int rows = dst->rows;
    for (int r = 0; r < rows; r++) {
        for (int ch = 0; ch < IT_MAX_CHANNELS; ch++) {
            const ITCell* sc = it_pattern_get_cell(src, r, ch);
            ITCell* dc = it_pattern_get_cell(dst, r, ch);
            if (sc && dc) *dc = *sc;
        }
    }

    /* Copy real Sound Blaster 16+ samples if empty in active song */
    for (int s = 1; s <= temp_song.sample_count && s < IT_MAX_SAMPLES; s++) {
        if (!song->samples[s].data && temp_song.samples[s].data) {
            song->samples[s] = temp_song.samples[s];
            song->samples[s].data = (float*)malloc(temp_song.samples[s].length * sizeof(float));
            if (song->samples[s].data) {
                memcpy(song->samples[s].data, temp_song.samples[s].data, temp_song.samples[s].length * sizeof(float));
            }
            if (s > song->sample_count) song->sample_count = s;
        }
    }

    it_song_clear(&temp_song);
    return true;
}
