#ifndef IT_FMT_IT_H
#define IT_FMT_IT_H

#include "song.h"

bool it_load_file(ITSong* song, const char* filepath);
bool it_save_file(const ITSong* song, const char* filepath);

/* Universal loader that detects format (.it, .s3m, .xm, .mod) */
bool it_load_module_any(ITSong* song, const char* filepath);

#endif /* IT_FMT_IT_H */
