#ifndef IT_COMMON_H
#define IT_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL3/SDL.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define IT_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define IT_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define IT_CLAMP(x, lo, hi) (((x) < (lo)) ? (lo) : (((x) > (hi)) ? (hi) : (x)))
#define IT_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define IT_MAX_CHANNELS          64
#define IT_MAX_VIRTUAL_CHANNELS  128
#define IT_MAX_ORDERS            256
#define IT_MAX_PATTERNS          256
#define IT_MAX_ROWS              200
#define IT_DEFAULT_ROWS          64
#define IT_MAX_SAMPLES           100
#define IT_MAX_INSTRUMENTS       100

#define IT_LOGICAL_WIDTH         1280
#define IT_LOGICAL_HEIGHT        800

#define IT_ORDER_SKIP            254
#define IT_ORDER_END             255

#define IT_NOTE_NONE             0
#define IT_NOTE_MIN              1
#define IT_NOTE_MAX              120
#define IT_NOTE_FADE             253
#define IT_NOTE_CUT              254
#define IT_NOTE_OFF              255

#define NNA_CUT                  0
#define NNA_CONTINUE             1
#define NNA_NOTE_OFF             2
#define NNA_NOTE_FADE            3

#define DCT_OFF                  0
#define DCT_NOTE                 1
#define DCT_SAMPLE               2
#define DCT_INSTRUMENT           3

#define DCA_CUT                  0
#define DCA_NOTE_OFF             1
#define DCA_NOTE_FADE            2

#define ENV_FLAG_ON              (1 << 0)
#define ENV_FLAG_LOOP            (1 << 1)
#define ENV_FLAG_SUSTAIN         (1 << 2)
#define ENV_FLAG_FILTER          (1 << 7)

#define SMP_FLAG_EXISTS          (1 << 0)
#define SMP_FLAG_16BIT           (1 << 1)
#define SMP_FLAG_STEREO          (1 << 2)
#define SMP_FLAG_COMPRESSED      (1 << 3)
#define SMP_FLAG_LOOP            (1 << 4)
#define SMP_FLAG_SUSTAIN         (1 << 5)
#define SMP_FLAG_PINGPONG        (1 << 6)
#define SMP_FLAG_PINGPONG_SUS    (1 << 7)

#define SONG_FLAG_STEREO         (1 << 0)
#define SONG_FLAG_VOL0MIX        (1 << 1)
#define SONG_FLAG_INSTRUMENTS    (1 << 2)
#define SONG_FLAG_LINEAR_SLIDES  (1 << 3)
#define SONG_FLAG_OLD_EFFECTS    (1 << 4)
#define SONG_FLAG_LINK_G         (1 << 5)
#define SONG_FLAG_USE_MIDI_PITCH (1 << 6)
#define SONG_FLAG_EMBED_MIDI     (1 << 7)

#endif /* IT_COMMON_H */
