#include "audio.h"
#include "player.h"
#include "mixer.h"

static SDL_AudioStream* s_audio_stream = NULL;
static uint32_t s_sample_rate = 48000;
static SDL_Mutex* s_audio_mutex = NULL;

static void SDLCALL audio_stream_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
    if (additional_amount <= 0) return;

    size_t num_frames = (size_t)(additional_amount / (sizeof(float) * 2));
    if (num_frames == 0) return;

    /* Max temporary buffer chunk */
    #define CHUNK_FRAMES 1024
    float chunk_buffer[CHUNK_FRAMES * 2];

    size_t frames_left = num_frames;
    while (frames_left > 0) {
        size_t frames_to_render = IT_MIN(frames_left, CHUNK_FRAMES);

        it_audio_lock();
        ITSong* song = it_player_get_song();
        if (song) {
            it_player_advance((uint32_t)frames_to_render);
            it_mixer_render(song, chunk_buffer, frames_to_render);
        } else {
            memset(chunk_buffer, 0, frames_to_render * 2 * sizeof(float));
        }
        it_audio_unlock();

        SDL_PutAudioStreamData(stream, chunk_buffer, (int)(frames_to_render * 2 * sizeof(float)));
        frames_left -= frames_to_render;
    }
}

bool it_audio_init(uint32_t sample_rate) {
    s_sample_rate = sample_rate ? sample_rate : 48000;
    if (!s_audio_mutex) s_audio_mutex = SDL_CreateMutex();

    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;
    spec.freq = (int)s_sample_rate;

    s_audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_stream_callback, NULL);
    if (!s_audio_stream) {
        SDL_Log("SDL_OpenAudioDeviceStream error: %s", SDL_GetError());
        return false;
    }

    it_mixer_init(s_sample_rate);
    SDL_ResumeAudioStreamDevice(s_audio_stream);
    return true;
}

void it_audio_shutdown(void) {
    if (s_audio_stream) {
        SDL_DestroyAudioStream(s_audio_stream);
        s_audio_stream = NULL;
    }
    if (s_audio_mutex) {
        SDL_DestroyMutex(s_audio_mutex);
        s_audio_mutex = NULL;
    }
}

void it_audio_lock(void) {
    if (s_audio_mutex) SDL_LockMutex(s_audio_mutex);
}

void it_audio_unlock(void) {
    if (s_audio_mutex) SDL_UnlockMutex(s_audio_mutex);
}

uint32_t it_audio_get_sample_rate(void) {
    return s_sample_rate;
}

int it_audio_get_device_count(void) {
    int count = 0;
    SDL_AudioDeviceID* dev = SDL_GetAudioPlaybackDevices(&count);
    if (dev) SDL_free(dev);
    return count;
}

const char* it_audio_get_device_name(int index) {
    int count = 0;
    SDL_AudioDeviceID* devs = SDL_GetAudioPlaybackDevices(&count);
    if (!devs || index < 0 || index >= count) {
        if (devs) SDL_free(devs);
        return "Default Audio Device";
    }
    const char* name = SDL_GetAudioDeviceName(devs[index]);
    SDL_free(devs);
    return name ? name : "Audio Device";
}
