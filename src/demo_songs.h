#ifndef IT_DEMO_SONGS_H
#define IT_DEMO_SONGS_H

#include "song.h"

typedef enum {
    IT_GENRE_HOUSE = 0,
    IT_GENRE_DNB,
    IT_GENRE_BREAKS,
    IT_GENRE_HIPHOP,
    IT_GENRE_TECHNO,
    IT_GENRE_TRANCE,
    IT_GENRE_RAVE,
    IT_GENRE_DUBTECHNO,
    IT_GENRE_POP,
    IT_GENRE_RNB,
    IT_GENRE_AMBIENT,
    IT_GENRE_ELECTRO,
    IT_GENRE_GARAGE,
    IT_GENRE_PSYTRANCE,
    IT_GENRE_INDUSTRIAL,
    IT_GENRE_SYNTHWAVE,
    IT_GENRE_CYBERPUNK,
    IT_GENRE_LOFI,
    IT_GENRE_HARDSTYLE,
    IT_GENRE_MINIMAL,
    IT_GENRE_COUNT
} ITGenreId;

typedef struct {
    ITGenreId id;
    const char* name;
    int bpm;
    int speed;
    const char* description;
    const char* drum_kit_info;
} ITGenreInfo;

int it_demo_get_genre_count(void);
const ITGenreInfo* it_demo_get_genre_info(int index);

/* Load drum groove on channels 1..4 (or 1..6) leaving channels 5..64 empty for user compositions */
bool it_demo_load_drum_groove(ITSong* song, ITGenreId genre);

/* Load full musical demo track (drums + bassline + lead melodies + chord stabs) */
bool it_demo_load_full_song(ITSong* song, ITGenreId genre);

/* Insert drum groove pattern directly into a specific pattern number */
bool it_demo_insert_pattern(ITSong* song, int pattern_idx, ITGenreId genre);

#endif /* IT_DEMO_SONGS_H */
