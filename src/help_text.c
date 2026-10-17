#include "help_text.h"
#include <stddef.h>

static const ITHelpSection s_sections[] = {
    {
        "Quickstart",
        "WELCOME TO IMPULSE TRACKER MODERN",
        "Impulse Tracker is a legendary digital audio workstation and music tracker\n"
        "originally created by Jeffrey Lim in 1995 for MS-DOS, now completely ported\n"
        "to modern 64-bit systems with high-resolution vector fonts, 32-bit float audio,\n"
        "and plug-and-play USB MIDI controller support.\n\n"
        "TRACKER CONCEPTS IN 60 SECONDS:\n"
        "-------------------------------------------------------------------------\n"
        "* MUSIC FLOWS VERTICALLY from top to bottom through rows.\n"
        "* A SONG consists of an ORDER LIST (sequence of patterns to play).\n"
        "* Each PATTERN has 64 rows (customizable up to 200) across up to 64 CHANNELS.\n"
        "* Each CELL on a channel contains 5 fields:\n"
        "    [Note] [Instrument] [Volume] [Effect] [Parameter]\n"
        "    Example:  C-5  01  v64  D02  (Play C-5 on Inst 1, Max Vol, Slide down)\n\n"
        "QUICK COMMANDS TO GET STARTED:\n"
        "-------------------------------------------------------------------------\n"
        "  F5 : Play entire song from beginning\n"
        "  F6 : Play current pattern from cursor row\n"
        "  F8 : Stop playback immediately\n"
        "  SPACE : Toggle EDIT MODE (allows typing notes directly into rows)\n"
        "  F1..F12 : Switch between Pattern, Samples, Instruments, Orders & Settings\n"
        "  MOUSE : Click any cell, slider, or button; use wheel to scroll rows!"
    },
    {
        "Keyboard",
        "PIANO KEYBOARD & EDITING SHORTCUTS",
        "THE TRACKER PIANO LAYOUT:\n"
        "-------------------------------------------------------------------------\n"
        "Lower Octave:   [S] [D]     [G] [H] [J]\n"
        "               [Z] [X] [C] [V] [B] [N] [M]\n"
        "               C-  D-  E-  F-  G-  A-  B-  (C#, D#, F#, G#, A#)\n\n"
        "Upper Octave:   [2] [3]     [5] [6] [7]\n"
        "               [Q] [W] [E] [R] [T] [Y] [U]\n"
        "               C-  D-  E-  F-  G-  A-  B-  (C#, D#, F#, G#, A#)\n\n"
        "NAVIGATION & EDITING KEYS:\n"
        "-------------------------------------------------------------------------\n"
        "  Arrow Keys        : Move cursor between rows and channel columns\n"
        "  Page Up / Down    : Jump by 16 rows\n"
        "  Home / End        : Jump to row 0 / last row of pattern\n"
        "  Tab / Shift+Tab   : Move to next / previous channel\n"
        "  < and > (or / and *): Shift active octave down / up (0 to 9)\n"
        "  Delete / Backspace: Clear current cell or value\n"
        "  ` (Tilde) / 1     : Insert Note Cut (^^^) or Note Off (===)\n"
        "  Alt + Enter / F11 : Toggle Fullscreen mode"
    },
    {
        "Mouse",
        "MODERN MOUSE NAVIGATION & GESTURES",
        "MODERN MOUSE CONTROLS:\n"
        "-------------------------------------------------------------------------\n"
        "* CLICK ANYWHERE: Click directly into any pattern cell to position the cursor.\n"
        "* SCROLL WHEEL: Smoothly scrolls pattern rows up and down.\n"
        "* SHIFT + WHEEL: Scrolls channels horizontally (across all 64 channels).\n"
        "* TOP TOOLBAR: Click any button ([F1 Help], [F2 Pattern], [F5 Play], etc.).\n"
        "* SLIDERS & KNOBS: Click and drag to smoothly adjust Tempo, Speed, Volume,\n"
        "  Panning, and Resonant Filter Cutoff in real-time!\n"
        "* CHANNEL HEADERS: Click any channel header to Mute / Unmute, or right-click\n"
        "  to Solo that channel instantly.\n"
        "* SAMPLE EDITOR: Click & drag on the waveform display to set Loop Start\n"
        "  and Loop End positions interactively.\n"
        "* ENVELOPE EDITOR: Click and drag envelope nodes to shape custom volume curves!"
    },
    {
        "Effects",
        "COMPLETE IMPULSE TRACKER EFFECTS REFERENCE",
        "COMMAND  EFFECT DESCRIPTION & EXAMPLES\n"
        "-------------------------------------------------------------------------\n"
        "  Axx    Set Speed (ticks per row, e.g. A06 = 6 ticks)\n"
        "  Bxx    Jump to Order xx (e.g. B03 jumps to order 3)\n"
        "  Cxx    Break to Row xx of next pattern\n"
        "  Dxx    Volume Slide: Dx0=Slide Up x, D0x=Slide Down x, DxF=Fine Up\n"
        "  Exx    Pitch Slide Down (Amiga or Linear slides)\n"
        "  Fxx    Pitch Slide Up\n"
        "  Gxx    Portamento to Note (glissando / smooth transition to target note)\n"
        "  Hxy    Vibrato: Speed x, Depth y (updated every tick for lush warmth)\n"
        "  Ixy    Tremor: On-time x ticks, Off-time y ticks\n"
        "  Jxy    Arpeggio: Cycles base note, note+x semitones, note+y semitones\n"
        "  Kxx    Dual Command: Vibrato (H00) + Volume Slide (Dxx)\n"
        "  Lxx    Dual Command: Portamento (G00) + Volume Slide (Dxx)\n"
        "  Mxx    Set Channel Volume (0..64)\n"
        "  Nxx    Channel Volume Slide (Nx0 up, N0x down)\n"
        "  Oxx    Sample Offset: Start playback at sample frame xx * 256\n"
        "  Pxx    Panning Slide (Px0 slide right, P0x slide left)\n"
        "  Qxy    Retrigger Note: Retriggers note every y ticks with volume effect x\n"
        "  Rxy    Tremolo: Volume Vibrato (Speed x, Depth y)\n"
        "  Sxx    Extended Effects:\n"
        "           S3x : Set Vibrato Waveform (0=Sine, 1=Ramp, 2=Square, 3=Random)\n"
        "           S4x : Set Tremolo Waveform\n"
        "           S7x : Past Note Cut / NNA Action\n"
        "           S8x : Set Panning (0=Left .. 15=Right)\n"
        "           SCx : Cut Note at tick x\n"
        "           SDx : Delay Note by tick x\n"
        "           SEx : Pattern Delay by x rows\n"
        "  Txx    Set Tempo (BPM from 32 to 255, e.g. T80 = 128 BPM)\n"
        "  Uxy    Fine Vibrato (fine depth)\n"
        "  Vxx    Set Global Volume (0 to 128)\n"
        "  Wxx    Global Volume Slide\n"
        "  Xxx    Set 8-bit Panning (0=Left .. 128=Right)\n"
        "  Yxy    Panbrello: Panning Vibrato (Speed x, Depth y)\n"
        "  Zxx    Resonant Low-Pass Filter: Cutoff (00..7F) and Resonance (80..FF)!"
    },
    {
        "MIDI Tutorial",
        "MODERN MIDI CONTROLLERS & INTERACTIVE REMAPPING TUTORIAL",
        "WELCOME TO THE IMPULSE TRACKER MIDI WORKSTATION:\n"
        "-------------------------------------------------------------------------\n"
        "Impulse Tracker features native plug-and-play USB and DIN MIDI controller\n"
        "support with over 650 pre-configured hardware profiles (2018 through October 2026).\n\n"
        "SUPPORTED CONTROLLERS (650+ POPULAR MODELS):\n"
        "-------------------------------------------------------------------------\n"
        "* Akai: MPK Mini MK3/Plus, MPK225/249/261, APC64, APC Mini MK2, MPC Key 37/61\n"
        "* Arturia: KeyStep Pro/37, MiniLab 3, KeyLab Essential mk3, KeyLab mk3\n"
        "* Novation: Launchkey MK4 (25/37/49/61), Launchpad Pro MK3, FLkey, SL MK3\n"
        "* Native Instruments: Kontrol S49/S61/S88 MK3, Kontrol A/M32, Maschine MK3\n"
        "* Korg: Keystage (Poly Aftertouch), microKEY Air, nanoKEY2, nanoKONTROL Studio\n"
        "* Roland: A-PRO series, A-88MKII, JD-Xi, Boutique controllers, System-8\n"
        "* M-Audio: Oxygen Pro 25/49/61, Keystation MK3, Hammer 88 Pro\n"
        "* Plus Alesis, Behringer, Yamaha, Casio, Expressive E Osmose, Hydrasynth & more!\n\n"
        "TUTORIAL: HOW TO MAP ANY UNSUPPORTED OR CUSTOM CONTROLLER:\n"
        "-------------------------------------------------------------------------\n"
        "If you have a custom DIY Arduino controller, a boutique synthesizer, an older\n"
        "MIDI master keyboard, or just want your own custom CC button layout:\n\n"
        "  Step 1: Plug your controller into any USB port or MIDI interface.\n"
        "  Step 2: Press [F11 Settings] and navigate to 'MIDI Learn & Remap Studio'.\n"
        "  Step 3: Look at the 'Live Hardware MIDI Monitor' to verify signals arrive.\n"
        "  Step 4: Click [Learn] next to any command (Play, Stop, Filter Cutoff, Volume).\n"
        "  Step 5: Turn the physical knob or press the physical key/pad on your device.\n"
        "  Step 6: The CC or Note is bound instantly and persists in your configuration!\n\n"
        "Click the button below to test or configure your controller right now:"
    }
};

int it_help_get_section_count(void) {
    return (int)(sizeof(s_sections) / sizeof(s_sections[0]));
}

const ITHelpSection* it_help_get_section(int index) {
    if (index < 0 || index >= it_help_get_section_count()) return NULL;
    return &s_sections[index];
}
