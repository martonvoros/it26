#include "midi.h"
#include "midi_learn.h"
#include "midi_profiles.h"
#include "player.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

#define MIDI_QUEUE_SIZE 256

static ITMidiMessage s_midi_queue[MIDI_QUEUE_SIZE];
static volatile int s_queue_head = 0;
static volatile int s_queue_tail = 0;
static int s_current_device = -1;

#ifdef _WIN32
static HMIDIIN s_hMidiIn = NULL;

static void CALLBACK midiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    (void)hMidiIn; (void)dwInstance;
    if (wMsg == MIM_DATA) {
        uint8_t status = (uint8_t)(dwParam1 & 0xFF);
        uint8_t data1  = (uint8_t)((dwParam1 >> 8) & 0xFF);
        uint8_t data2  = (uint8_t)((dwParam1 >> 16) & 0xFF);
        uint32_t ts    = (uint32_t)dwParam2;

        int next_head = (s_queue_head + 1) % MIDI_QUEUE_SIZE;
        if (next_head != s_queue_tail) {
            s_midi_queue[s_queue_head].status = status;
            s_midi_queue[s_queue_head].data1 = data1;
            s_midi_queue[s_queue_head].data2 = data2;
            s_midi_queue[s_queue_head].timestamp = ts;
            s_queue_head = next_head;
        }
    }
}
#endif

bool it_midi_init(void) {
    s_queue_head = 0;
    s_queue_tail = 0;
    s_current_device = -1;

#ifdef _WIN32
    int num_devs = (int)midiInGetNumDevs();
    if (num_devs > 0) {
        it_midi_open_device(0); /* Automatically open the first available MIDI controller */
    }
#endif
    return true;
}

void it_midi_shutdown(void) {
    it_midi_close_device();
}

int it_midi_get_device_count(void) {
#ifdef _WIN32
    return (int)midiInGetNumDevs();
#else
    return 0;
#endif
}

const char* it_midi_get_device_name(int index) {
#ifdef _WIN32
    static char dev_name[MAXPNAMELEN];
    MIDIINCAPSA caps;
    if (midiInGetDevCapsA((UINT)index, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
        strncpy(dev_name, caps.szPname, sizeof(dev_name) - 1);
        dev_name[sizeof(dev_name) - 1] = '\0';
        return dev_name;
    }
#endif
    return "No MIDI Device";
}

int it_midi_get_current_device(void) {
    return s_current_device;
}

bool it_midi_open_device(int index) {
    it_midi_close_device();
#ifdef _WIN32
    if (index < 0 || index >= it_midi_get_device_count()) return false;
    MMRESULT res = midiInOpen(&s_hMidiIn, (UINT)index, (DWORD_PTR)midiInProc, 0, CALLBACK_FUNCTION);
    if (res != MMSYSERR_NOERROR) {
        s_hMidiIn = NULL;
        return false;
    }
    midiInStart(s_hMidiIn);
    s_current_device = index;
    return true;
#else
    return false;
#endif
}

void it_midi_close_device(void) {
#ifdef _WIN32
    if (s_hMidiIn) {
        midiInStop(s_hMidiIn);
        midiInClose(s_hMidiIn);
        s_hMidiIn = NULL;
    }
#endif
    s_current_device = -1;
}

static ITMidiMessage s_last_msg = {0};
static uint64_t s_last_msg_ticks = 0;

bool it_midi_poll(ITMidiMessage* out_msg) {
    if (s_queue_head == s_queue_tail) return false;
    *out_msg = s_midi_queue[s_queue_tail];
    s_queue_tail = (s_queue_tail + 1) % MIDI_QUEUE_SIZE;

    s_last_msg = *out_msg;
    s_last_msg_ticks = SDL_GetTicks();
    return true;
}

bool it_midi_get_last_message(ITMidiMessage* out_msg, uint32_t* out_age_ms) {
    if (s_last_msg_ticks == 0) return false;
    if (out_msg) *out_msg = s_last_msg;
    if (out_age_ms) *out_age_ms = (uint32_t)(SDL_GetTicks() - s_last_msg_ticks);
    return true;
}

void it_midi_process(void) {
    ITMidiMessage msg;
    while (it_midi_poll(&msg)) {
        /* Let MIDI Learn or custom bindings intercept first */
        if (it_midi_learn_handle_message(msg.status, msg.data1, msg.data2)) {
            continue;
        }

        uint8_t type = msg.status & 0xF0;
        uint8_t note_num = msg.data1;
        uint8_t velocity = msg.data2;

        /* Note On with velocity 0 is Note Off */
        if (type == IT_MIDI_NOTE_ON && velocity > 0) {
            /* Middle C is MIDI note 60 -> IT note 61 (C-5) */
            int it_note = (int)note_num + 1;
            if (it_note >= 1 && it_note <= 120) {
                int vol = (int)((velocity * 64) / 127);
                /* Trigger on channel 0 or current channel */
                it_player_trigger_note(0, it_note, 1, vol);
            }
        } else if (type == IT_MIDI_NOTE_OFF || (type == IT_MIDI_NOTE_ON && velocity == 0)) {
            it_player_key_off(0);
        } else if (type == IT_MIDI_PITCH_BEND) {
            /* 14-bit pitch bend: data1 = LSB, data2 = MSB */
            int bend = ((int)msg.data2 << 7) | msg.data1; /* 0..16383, center 8192 */
            float bend_factor = powf(2.0f, (float)(bend - 8192) / (8192.0f * 6.0f));
            for (int i = 0; i < IT_MAX_VIRTUAL_CHANNELS; i++) {
                ITVoice* v = it_mixer_get_voice(i);
                if (v && v->active && v->host_channel == 0) {
                    v->sample_step = (double)(v->base_freq * bend_factor) / (double)it_mixer_get_sample_rate();
                }
            }
        }
    }
}

