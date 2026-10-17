#ifndef IT_MIDI_H
#define IT_MIDI_H

#include "common.h"

typedef enum {
    IT_MIDI_NOTE_OFF = 0x80,
    IT_MIDI_NOTE_ON = 0x90,
    IT_MIDI_POLY_PRESSURE = 0xA0,
    IT_MIDI_CONTROL_CHANGE = 0xB0,
    IT_MIDI_PROGRAM_CHANGE = 0xC0,
    IT_MIDI_CHANNEL_PRESSURE = 0xD0,
    IT_MIDI_PITCH_BEND = 0xE0
} ITMidiType;

typedef struct {
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
    uint32_t timestamp;
} ITMidiMessage;

bool it_midi_init(void);
void it_midi_shutdown(void);

int it_midi_get_device_count(void);
const char* it_midi_get_device_name(int index);
int it_midi_get_current_device(void);
bool it_midi_open_device(int index);
void it_midi_close_device(void);

/* Poll for incoming MIDI messages from USB/Hardware devices */
bool it_midi_poll(ITMidiMessage* out_msg);

/* Process MIDI messages (trigger voices or record into pattern) */
void it_midi_process(void);

/* Get the most recently received MIDI message and elapsed time since arrival */
bool it_midi_get_last_message(ITMidiMessage* out_msg, uint32_t* out_age_ms);

#endif /* IT_MIDI_H */
