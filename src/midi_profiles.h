#ifndef IT_MIDI_PROFILES_H
#define IT_MIDI_PROFILES_H

#include "common.h"

typedef struct {
    uint8_t cc_play;
    uint8_t cc_stop;
    uint8_t cc_record;
    uint8_t cc_loop;
    uint8_t cc_prev_order;
    uint8_t cc_next_order;

    /* Knobs / Encoders CC */
    uint8_t cc_knob_cutoff;    /* Filter Cutoff */
    uint8_t cc_knob_resonance; /* Filter Resonance */
    uint8_t cc_knob_volume;    /* Active Channel Volume */
    uint8_t cc_knob_pan;       /* Active Channel Pan */
    uint8_t cc_knob_tempo;     /* Song Tempo (BPM) */
    uint8_t cc_knob_global_vol;/* Song Global Volume */

    /* Pad octave / base note offset */
    int pad_octave_offset;
} ITMidiMapping;

typedef struct {
    const char* brand;
    const char* model;
    const char* match_pattern; /* Substring in USB/MIDI device name */
    ITMidiMapping mapping;
} ITMidiControllerProfile;

void it_midi_profiles_init(void);
int it_midi_profiles_get_count(void);
const ITMidiControllerProfile* it_midi_profiles_get(int index);
const ITMidiControllerProfile* it_midi_profiles_find_for_device(const char* device_name);
const ITMidiMapping* it_midi_profiles_get_active_mapping(void);
void it_midi_profiles_set_active(int index);

#endif /* IT_MIDI_PROFILES_H */
