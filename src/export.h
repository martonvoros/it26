#ifndef IT_EXPORT_H
#define IT_EXPORT_H

#include <stdbool.h>
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Export active pattern (loop) to 16-bit 44.1kHz Stereo PCM WAV */
bool it_export_wav_loop(const ITSong* song, int pattern_idx, const char* filepath);

/* Export entire song to 16-bit 44.1kHz Stereo PCM WAV */
bool it_export_wav_song(const ITSong* song, const char* filepath);

/* Export active pattern (loop) to Standard MIDI File (.mid) */
bool it_export_midi_loop(const ITSong* song, int pattern_idx, const char* filepath);

/* Export entire song to Standard MIDI File (.mid) */
bool it_export_midi_song(const ITSong* song, const char* filepath);

#ifdef __cplusplus
}
#endif

#endif /* IT_EXPORT_H */
