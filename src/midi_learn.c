#include "midi_learn.h"
#include "player.h"

static ITMidiBinding s_bindings[IT_ACTION_COUNT];
static bool s_is_learning = false;
static ITMidiAction s_learning_action = IT_ACTION_PLAY;

static const char* s_action_names[IT_ACTION_COUNT] = {
    "Play Song / Pattern",
    "Stop Playback",
    "Toggle Record / Edit",
    "Next Order",
    "Previous Order",
    "Octave Up",
    "Octave Down",
    "Filter Cutoff (Knob)",
    "Filter Resonance (Knob)",
    "Channel Volume (Knob)",
    "Channel Pan (Knob)",
    "Song Tempo (Knob)",
    "Global Volume (Knob)",
    "Mute / Unmute Channel 1",
    "Mute / Unmute Channel 2",
    "Mute / Unmute Channel 3",
    "Mute / Unmute Channel 4"
};

void it_midi_learn_init(void) {
    memset(s_bindings, 0, sizeof(s_bindings));
    s_is_learning = false;

    /* Set default factory bindings for standard CCs */
    s_bindings[IT_ACTION_PLAY].is_bound = true;
    s_bindings[IT_ACTION_PLAY].is_note = false;
    s_bindings[IT_ACTION_PLAY].cc_or_note = 115; /* MMC Play */

    s_bindings[IT_ACTION_STOP].is_bound = true;
    s_bindings[IT_ACTION_STOP].is_note = false;
    s_bindings[IT_ACTION_STOP].cc_or_note = 114; /* MMC Stop */

    s_bindings[IT_ACTION_KNOB_CUTOFF].is_bound = true;
    s_bindings[IT_ACTION_KNOB_CUTOFF].is_note = false;
    s_bindings[IT_ACTION_KNOB_CUTOFF].cc_or_note = 74; /* Standard Sound Brightness / Cutoff */

    s_bindings[IT_ACTION_KNOB_RESONANCE].is_bound = true;
    s_bindings[IT_ACTION_KNOB_RESONANCE].is_note = false;
    s_bindings[IT_ACTION_KNOB_RESONANCE].cc_or_note = 71; /* Standard Harmonic Resonance */

    s_bindings[IT_ACTION_KNOB_VOLUME].is_bound = true;
    s_bindings[IT_ACTION_KNOB_VOLUME].is_note = false;
    s_bindings[IT_ACTION_KNOB_VOLUME].cc_or_note = 7; /* Channel Volume */

    s_bindings[IT_ACTION_KNOB_PAN].is_bound = true;
    s_bindings[IT_ACTION_KNOB_PAN].is_note = false;
    s_bindings[IT_ACTION_KNOB_PAN].cc_or_note = 10; /* Pan */
}

void it_midi_learn_start(ITMidiAction action) {
    if (action >= 0 && action < IT_ACTION_COUNT) {
        s_is_learning = true;
        s_learning_action = action;
    }
}

void it_midi_learn_cancel(void) {
    s_is_learning = false;
}

bool it_midi_learn_is_learning(void) {
    return s_is_learning;
}

ITMidiAction it_midi_learn_get_learning_action(void) {
    return s_learning_action;
}

const char* it_midi_learn_get_action_name(ITMidiAction action) {
    if (action >= 0 && action < IT_ACTION_COUNT) {
        return s_action_names[action];
    }
    return "Unknown Action";
}

const ITMidiBinding* it_midi_learn_get_binding(ITMidiAction action) {
    if (action >= 0 && action < IT_ACTION_COUNT) {
        return &s_bindings[action];
    }
    return NULL;
}

void it_midi_learn_clear_binding(ITMidiAction action) {
    if (action >= 0 && action < IT_ACTION_COUNT) {
        memset(&s_bindings[action], 0, sizeof(ITMidiBinding));
    }
}

bool it_midi_learn_handle_message(uint8_t status, uint8_t data1, uint8_t data2) {
    uint8_t type = status & 0xF0;

    /* If user clicked Learn on a command, capture this incoming CC or Note */
    if (s_is_learning) {
        if (type == 0xB0) { /* CC */
            s_bindings[s_learning_action].is_bound = true;
            s_bindings[s_learning_action].is_note = false;
            s_bindings[s_learning_action].cc_or_note = data1;
            s_bindings[s_learning_action].channel = (status & 0x0F) + 1;
            s_is_learning = false;
            return true;
        } else if (type == 0x90 && data2 > 0) { /* Note On */
            s_bindings[s_learning_action].is_bound = true;
            s_bindings[s_learning_action].is_note = true;
            s_bindings[s_learning_action].cc_or_note = data1;
            s_bindings[s_learning_action].channel = (status & 0x0F) + 1;
            s_is_learning = false;
            return true;
        }
    }

    /* Check triggers */
    ITSong* song = it_player_get_song();

    for (int i = 0; i < IT_ACTION_COUNT; i++) {
        if (!s_bindings[i].is_bound) continue;

        bool match = false;
        if (!s_bindings[i].is_note && type == 0xB0 && s_bindings[i].cc_or_note == data1) {
            match = true;
        } else if (s_bindings[i].is_note && type == 0x90 && data2 > 0 && s_bindings[i].cc_or_note == data1) {
            match = true;
        }

        if (match) {
            switch ((ITMidiAction)i) {
                case IT_ACTION_PLAY:
                    if (!it_player_is_playing()) it_player_play_song();
                    else it_player_pause();
                    return true;
                case IT_ACTION_STOP:
                    it_player_stop();
                    return true;
                case IT_ACTION_KNOB_CUTOFF:
                    /* Adjust filter cutoff on active voices */
                    for (int v = 0; v < IT_MAX_VIRTUAL_CHANNELS; v++) {
                        ITVoice* voice = it_mixer_get_voice(v);
                        if (voice && voice->active) {
                            voice->filter_enabled = true;
                            voice->filter_cutoff = (float)data2;
                        }
                    }
                    return true;
                case IT_ACTION_KNOB_RESONANCE:
                    for (int v = 0; v < IT_MAX_VIRTUAL_CHANNELS; v++) {
                        ITVoice* voice = it_mixer_get_voice(v);
                        if (voice && voice->active) {
                            voice->filter_enabled = true;
                            voice->filter_resonance = (float)data2;
                        }
                    }
                    return true;
                case IT_ACTION_KNOB_VOLUME:
                    if (song) song->channel_vol[0] = data2 / 2; /* 0..127 -> 0..64 */
                    return true;
                case IT_ACTION_KNOB_PAN:
                    if (song) song->channel_pan[0] = data2 / 2; /* 0..127 -> 0..64 */
                    return true;
                case IT_ACTION_KNOB_TEMPO: {
                    int tempo = 32 + (int)((data2 * (255 - 32)) / 127);
                    if (song) song->initial_tempo = tempo;
                    return true;
                }
                case IT_ACTION_KNOB_GLOBAL_VOL:
                    if (song) song->global_vol = data2; /* 0..127 */
                    return true;
                case IT_ACTION_MUTE_CH1:
                case IT_ACTION_MUTE_CH2:
                case IT_ACTION_MUTE_CH3:
                case IT_ACTION_MUTE_CH4: {
                    int ch = i - IT_ACTION_MUTE_CH1;
                    it_player_set_channel_mute(ch, !it_player_get_channel_mute(ch));
                    return true;
                }
                default:
                    break;
            }
        }
    }
    return false;
}
