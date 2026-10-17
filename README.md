# Impulse Tracker Modern (IT26)

[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)]()
[![Standard](https://img.shields.io/badge/Language-C99-blue.svg)]()
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-6f42c1.svg)]()
[![Framework](https://img.shields.io/badge/Audio%2FGFX-SDL3-red.svg)]()
[![MIDI](https://img.shields.io/badge/MIDI%20Controllers-655%2B%20Supported-orange.svg)]()
[![License](https://img.shields.io/badge/License-GPLv2%2BMIT-lightgrey.svg)]()

> **ImpulseTracker was a standout MS-DOS sequencer with real influence on modern electronic music.**  
>  
> I’ve made a clean Windows/Mac/Linux port that scales on modern monitors and sounds correct. *(and even supports your modern MIDI controller)*

---

![Impulse Tracker Modern](head.jpg)

---

## ⚡ What is Impulse Tracker Modern?

In 1995, **Jeffrey Lim** released **Impulse Tracker 2.14** for MS-DOS. It became the weapon of choice for a generation of underground electronic musicians, game composers, and demoscene legends. The lightning-fast keyboard workflow, laser-precise tick timing, and Jeffrey Lim's signature **resonant 24dB state-variable lowpass filter (`Zxx`)** defined the sound of 90s Drum & Bass, Jungle, Acid Techno, and Game Soundtracks (like *Unreal Tournament* and *Deus Ex*).

**Impulse Tracker Modern (IT26)** is a clean, native **C99** and **SDL3** recreation engineered from the ground up for modern 64-bit platforms. It preserves the authentic factory Impulse Tracker 2.14 layout, typography, and mathematical audio behavior while delivering high-DPI scaling, seamless tactile mouse interaction, plug-and-play support for **655 modern USB/MIDI controllers**, and an interactive **Quickstart Wizard** for modern electronic music producers.

---

## ✨ Key Features

### 🎛️ 1. Factory-Authentic IT 2.14 Interface (Modern & Scalable)
* **Classic Proportions**: Ultra-compact 60px header block displaying Song, Order, Pattern, Row, Speed, Tempo, Volumes, Voices, Instrument, Octave, and Master Stereo Peak VU Meter.
* **Maximum Pattern Workspace**: Starts immediately below the header with 25 to 50+ visible rows.
* **Vector Monospace Font**: Sub-pixel antialiased JetBrains Mono rendering that scales cleanly on 1080p, 1440p, 4K, and Retina displays with zero pixel blur.
* **Zero Clipping & Clean Glyphs**: Custom vector glyph renderer for play triangles (`▶`), stop squares (`■`), and signal dots (`●`) without ASCII corruption or text overlap.

### 🖱️ 2. Modern Tactile Mouse & Keyboard Engine
* **Full Mouse Interaction**:
  - Click-and-drag cell scrubbing (`Note`, `Instrument`, `Volume`, `Effect`).
  - Tactile channel header **`[M]` (Mute)** and **`[S]` (Solo)** buttons with instant visual feedback.
  - Double-click any channel header to solo.
  - Mouse wheel row scrolling & `Shift + Wheel` channel horizontal scrolling.
  - Interactive sliders and scrollbars with global drag-capture.
* **Authentic Tracker Keyboard Flow**:
  - `Space`: Toggle Edit Mode.
  - `Z`..`M`: Lower Octave piano notes.
  - `Q`..`U`: Upper Octave piano notes.
  - `Delete`: Clear note/cell.
  - `F1`..`F12`: Instant navigation across Help, Pattern Editor, Samples, Instruments, Play/Stop, Orders, Info, Disk, and MIDI Studio.

### 🧙 3. Modern Quickstart Wizard (`PAGE_WIZARD`)
Designed specifically for modern musicians transitioning from horizontal DAWs (Ableton Live, FL Studio, Logic Pro, Reaper):
1. **Vertical Timeline vs Horizontal DAW**: Clear conceptual guide explaining tick-based tracking, 16th-note rows, and lightning-fast keyboard muscle memory.
2. **Cell Anatomy**: Interactive diagram breaking down `[ Note | Instrument | Volume | Effect ]` and hexadecimal notation (`00`..`FF`).
3. **Interactive Piano Sound Lab**: A 2-octave virtual keyboard on screen allowing mouse clicking, computer keyboard typing, or MIDI playing to audition sounds in real-time.
4. **1-Click Launchpad**: Jump straight into the Pattern Editor, browse 20 genre demo tracks, configure MIDI hardware, or start a clean blank project.

### 🎹 4. Plug-and-Play Support for 655 Hardware MIDI Controllers
* **Automated Profile Database**: Native recognition of 655 controllers (2018–2026) across 40+ brands:
  - *Akai Professional* (MPK Mini Plus/MK3, APC64, Force, MPC One, Midimix)
  - *Novation* (Launchkey MK4/MK3, Launchpad Pro MK3, FLkey 37/49/61, SL MKIII)
  - *Arturia* (KeyLab Essential mk3, MiniLab 3, MicroFreak, BeatStep Pro)
  - *Native Instruments* (Komplete Kontrol S49/S61/S88 MK3, Maschine MK3)
  - *Korg* (nanoKEY2, microKEY Air, opsix, wavestate, Minilogue XD)
  - *Roland, Yamaha, Teenage Engineering, Elektron, ASM, Behringer, M-Audio, Nektar, Keith McMillen, Joué, ROLI, and more.*
* **Interactive MIDI Learn & Remap Studio (`[F11]`)**:
  - Click any tracker parameter (Filter Cutoff `Zxx`, Master Volume, Tempo, Channel Volume/Pan) and turn a physical knob or push a fader to bind instantly.
  - Real-time diagnostic inspector showing status bytes, CC numbers, note velocity, and signal age.

### 🎵 5. Iconic Real Impulse Tracker Hit Tracks & 20 Genre Demos
* **Autoplay on Startup**: Immediately loads and plays the legendary complex 90s classic house anthem **"House Of Heaven (Xtended)"** (`assets/hard_house.it`) with 86 instruments, 65 samples, and 72 patterns!
* **Famous Tracker Classics Included**:
  - `assets/hard_house.it`: "House Of Heaven (Xtended)" — Legendary complex 90s house masterpiece (86 instruments, 65 samples, 72 patterns)
  - `assets/demos/foregone.it`: Michiel van den Bos — "Foregone Destruction" (*Unreal Tournament* iconic Facing Worlds theme)
  - `assets/demos/basehead_-_unreal_tournament_-_seeker.it`: Basehead — "Seeker" (*Unreal Tournament* official soundtrack)
  - `assets/demos/skaven_-_quality_time_-_big_money_mix_v2.it`: Skaven — "Quality Time (Big Money Mix)"
  - `assets/demos/necros_-_1997_hornet_multimedia_upgrade.it`: Necros — "Hornet Multimedia Upgrade"
  - `assets/demos/basehead_-_indigo.it`: Basehead — "Indigo"
  - `assets/groovetasticola.it`: "Groovetasticola" — Classic funk groove
  - `assets/demos/a-windf.it`: Skaven — "Winds of Fjords"
  - `assets/demos/surasshu_-_eek.it`: Surasshu — "Eek!"
* **20 Real Electronic Genre Kits & Demos (`[F10]`)**:
  - House, Drum & Bass, Techno, Trance, Rave, Dub Techno, Synthwave, Breakbeat, Electro, Future Garage, Ambient, Acid Goa, Cyberpunk, and more.
  - Each genre provides both **"Load Drum Beat & Write Music"** (Drums on Ch 1-4, Ch 5-64 completely free for you to compose) and **"Load Full Real IT Track & Play"**.

---

## 🛠️ Architecture & Tech Stack

```
impulse-tracker/
├── src/
│   ├── main.c              # Application entry, event loop & high-DPI initialization
│   ├── song.c / song.h     # High-capacity module data model (64 channels, 256 patterns)
│   ├── player.c / player.h # Tick-accurate sequencer, effect dispatch & slide memory
│   ├── mixer.c / mixer.h   # 32-bit float mixer with 24dB resonant state-variable filters
│   ├── audio.c / audio.h   # SDL3 low-latency audio driver
│   ├── gfx.c / gfx.h       # Monospace vector typography & vector symbol renderer
│   ├── ui.c / ui.h         # Authentic compact header & page routing coordinator
│   ├── widgets.c           # Beveled buttons, panels, sliders, and scrollbars
│   ├── page_wizard.c       # Interactive modern Quickstart Wizard with Live Sound Lab
│   ├── page_pattern.c      # 64-channel Pattern Editor with tactile mouse controls
│   ├── page_orders.c       # Order sequence list & playback navigator
│   ├── page_samples.c      # Sample manager & waveform visualizer
│   ├── page_instruments.c  # Instrument envelopes (Volume, Pan, Pitch/Filter)
│   ├── page_files.c        # Local disk module browser & 20 genre kits
│   ├── page_settings.c     # MIDI setup & 655 controller profiles database
│   ├── midi.c / midi.h     # Cross-platform raw MIDI message processing
│   ├── midi_profiles.c     # 655 Hardware MIDI controller profile database
│   ├── midi_learn.c        # Dynamic MIDI CC & note learn matrix
│   ├── fmt_it.c            # Complete Jeffrey Lim Impulse Tracker binary loader/saver
│   ├── fmt_xm.c / fmt_s3m.c/ fmt_mod.c # Universal multi-format tracker loaders
│   └── demo_songs.c        # Genre kit synthesizer & real IT module loaders
├── res/                    # Windows manifest, version info & icons
└── assets/                 # Embedded fonts, bootscreen image & real IT modules
```

---

## 🚀 Building from Source

### Windows (MinGW-w64 + Ninja / CMake)
```powershell
# In PowerShell:
$env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Launch the tracker:
.\impulse-tracker.exe
```

### Linux (Ubuntu / Debian / Fedora / Arch)
```bash
sudo apt install build-essential cmake ninja-build libasound2-dev libpulse-dev libx11-dev libwayland-dev
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/impulse-tracker
```

### macOS (Apple Silicon / Intel)
```bash
brew install cmake ninja sdl3
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/impulse-tracker
```

---

## ⌨️ Global Keyboard Reference

| Key | Action |
|:---|:---|
| **`F1`** | Help & Tracker Manual |
| **`F2`** | Pattern Editor |
| **`F3`** | Sample Manager |
| **`F4`** | Instrument Envelopes & NNA Setup |
| **`F5`** | Play Song from Current Order |
| **`F6`** | Play Pattern from Current Row |
| **`F7`** | Order Sequence List |
| **`F8`** | Stop Playback |
| **`F9`** | 64-Channel Real-time Oscilloscope & Module Info |
| **`Shift + F9`** | Song Message / Author Notes Viewer |
| **`F10`** | Disk Browser & 20 Real Genre Kits |
| **`F11`** | MIDI Studio (655 Hardware Profiles & MIDI Learn) |
| **`F12`** | Song Variables & Channel Settings |
| **`Space`** | Toggle Edit Mode / View Mode |
| **`Alt + Enter`**| Toggle Fullscreen |
| **`Esc`** | Return to Pattern Editor from any page |
| **`Ctrl + Z`** | Undo |

---

## 📜 Credits & Heritage
* **Original Impulse Tracker**: Designed and programmed by **Jeffrey Lim** (1995–2014).
* **Music in Demos**:
  - Alexander David (DJ-Aldav) — *"House Of Fame"*
  - Bacter vs Saga Musix — *"Funky Junkie"*
  - Silicon / Chris Jarvis — *"Twilight Tears"*
  - Elwood (Jussi-Matti Salmela) — *"Dead Lock"*
  - Michiel van den Bos — *"Foregone Destruction"*
  - Skaven (Peter Hajba) — *"Windfall"*
  - Surasshu (Steven Velema) — *"Eek"*
* **Fonts**: JetBrains Mono (SIL Open Font License).
* **Libraries**: SDL3 (zlib license), stb_truetype & stb_image (public domain).

---

## 🤝 Contributing & License
Contributions, feedback, and bug reports are welcome!  
Distributed under the GNU General Public License v2 (GPL-2.0) and MIT License for modular components.
