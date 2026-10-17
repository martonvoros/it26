#ifndef IT_MIDI_LEARN_H
#define IT_MIDI_LEARN_H

#include "common.h"

typedef enum {
    IT_ACTION_PLAY = 0,
    IT_ACTION_STOP,
    IT_ACTION_RECORD,
    IT_ACTION_NEXT_ORDER,
    IT_ACTION_PREV_ORDER,
    IT_ACTION_OCTAVE_UP,
    IT_ACTION_OCTAVE_DOWN,
    IT_ACTION_KNOB_CUTOFF,
    IT_ACTION_KNOB_RESONANCE,
    IT_ACTION_KNOB_VOLUME,
    IT_ACTION_KNOB_PAN,
    IT_ACTION_KNOB_TEMPO,
    IT_ACTION_KNOB_GLOBAL_VOL,
    IT_ACTION_MUTE_CH1,
    IT_ACTION_MUTE_CH2,
    IT_ACTION_MUTE_CH3,
    IT_ACTION_MUTE_CH4,
    IT_ACTION_COUNT
} ITMidiAction;

typedef struct {
    bool is_bound;
    bool is_note;   /* true = note, false = CC */
    uint8_t cc_or_note;
    uint8_t channel;/* 0 = any, 1..16 */
} ITMidiBinding;

void it_midi_learn_init(void);
void it_midi_learn_start(ITMidiAction action);
void it_midi_learn_cancel(void);
bool it_midi_learn_is_learning(void);
ITMidiAction it_midi_learn_get_learning_action(void);

const char* it_midi_learn_get_action_name(ITMidiAction action);
const ITMidiBinding* it_midi_learn_get_binding(ITMidiAction action);
void it_midi_learn_clear_binding(ITMidiAction action);

/* Handle incoming MIDI message for MIDI Learn & custom binding triggers */
bool it_midi_learn_handle_message(uint8_t status, uint8_t data1, uint8_t data2);

#endif /* IT_MIDI_LEARN_H */
