#include "mixer.h"
#include "util.h"
#include <math.h>

static ITVoice s_voices[IT_MAX_VIRTUAL_CHANNELS];
static ITChannelVis s_chan_vis[IT_MAX_CHANNELS];
static uint32_t s_sample_rate = 48000;
static float s_master_peak_l = 0.0f;
static float s_master_peak_r = 0.0f;

void it_mixer_init(uint32_t sample_rate) {
    s_sample_rate = sample_rate ? sample_rate : 48000;
    it_mixer_reset_voices();
    memset(s_chan_vis, 0, sizeof(s_chan_vis));
}

void it_mixer_set_sample_rate(uint32_t sample_rate) {
    if (sample_rate > 0) s_sample_rate = sample_rate;
}

uint32_t it_mixer_get_sample_rate(void) {
    return s_sample_rate;
}

void it_mixer_reset_voices(void) {
    memset(s_voices, 0, sizeof(s_voices));
}

ITVoice* it_mixer_alloc_voice(int host_channel, int nna) {
    /* 1. Try to find an inactive voice */
    for (int i = 0; i < IT_MAX_VIRTUAL_CHANNELS; i++) {
        if (!s_voices[i].active) {
            s_voices[i].active = true;
            s_voices[i].host_channel = (uint8_t)host_channel;
            s_voices[i].background = false;
            return &s_voices[i];
        }
    }

    /* 2. Steal background voice with lowest volume */
    int best_idx = -1;
    float min_vol = 999.0f;
    for (int i = 0; i < IT_MAX_VIRTUAL_CHANNELS; i++) {
        if (s_voices[i].background && s_voices[i].vol_final < min_vol) {
            min_vol = s_voices[i].vol_final;
            best_idx = i;
        }
    }

    if (best_idx >= 0) {
        memset(&s_voices[best_idx], 0, sizeof(ITVoice));
        s_voices[best_idx].active = true;
        s_voices[best_idx].host_channel = (uint8_t)host_channel;
        return &s_voices[best_idx];
    }

    /* 3. Fallback to any voice */
    int idx = host_channel % IT_MAX_VIRTUAL_CHANNELS;
    memset(&s_voices[idx], 0, sizeof(ITVoice));
    s_voices[idx].active = true;
    s_voices[idx].host_channel = (uint8_t)host_channel;
    return &s_voices[idx];
}

void it_mixer_stop_voice(ITVoice* v) {
    if (v) v->active = false;
}

void it_mixer_stop_channel(int host_channel) {
    for (int i = 0; i < IT_MAX_VIRTUAL_CHANNELS; i++) {
        if (s_voices[i].active && s_voices[i].host_channel == host_channel) {
            s_voices[i].active = false;
        }
    }
}

ITVoice* it_mixer_get_voice(int index) {
    if (index < 0 || index >= IT_MAX_VIRTUAL_CHANNELS) return NULL;
    return &s_voices[index];
}

const ITChannelVis* it_mixer_get_channel_vis(int host_channel) {
    if (host_channel < 0 || host_channel >= IT_MAX_CHANNELS) return NULL;
    return &s_chan_vis[host_channel];
}

static float evaluate_envelope(const ITEnvelope* env, uint16_t tick, bool key_off) {
    if (!env || !(env->flags & ENV_FLAG_ON) || env->num_nodes == 0) return 1.0f;
    if (env->num_nodes == 1) return (float)env->nodes[0].y;

    /* Loop handling */
    uint16_t max_tick = env->nodes[env->num_nodes - 1].tick;
    if (!key_off && (env->flags & ENV_FLAG_SUSTAIN) && env->susloop_end < env->num_nodes) {
        uint16_t s_start = env->nodes[env->susloop_start].tick;
        uint16_t s_end = env->nodes[env->susloop_end].tick;
        if (tick >= s_end && s_end > s_start) {
            tick = s_start + ((tick - s_start) % (s_end - s_start));
        }
    } else if ((env->flags & ENV_FLAG_LOOP) && env->loop_end < env->num_nodes) {
        uint16_t l_start = env->nodes[env->loop_start].tick;
        uint16_t l_end = env->nodes[env->loop_end].tick;
        if (tick >= l_end && l_end > l_start) {
            tick = l_start + ((tick - l_start) % (l_end - l_start));
        }
    }

    if (tick >= max_tick) {
        return (float)env->nodes[env->num_nodes - 1].y;
    }

    for (int i = 0; i < env->num_nodes - 1; i++) {
        if (tick >= env->nodes[i].tick && tick <= env->nodes[i + 1].tick) {
            uint16_t t0 = env->nodes[i].tick;
            uint16_t t1 = env->nodes[i + 1].tick;
            float frac = (t1 == t0) ? 0.0f : (float)(tick - t0) / (float)(t1 - t0);
            return (float)env->nodes[i].y + frac * (float)(env->nodes[i + 1].y - env->nodes[i].y);
        }
    }

    return (float)env->nodes[env->num_nodes - 1].y;
}

void it_mixer_render(ITSong* song, float* out_interleaved, size_t num_frames) {
    if (!song || !out_interleaved || num_frames == 0) return;

    memset(out_interleaved, 0, num_frames * 2 * sizeof(float));

    /* Decay VU levels smoothly */
    for (int c = 0; c < IT_MAX_CHANNELS; c++) {
        s_chan_vis[c].vu_level *= 0.85f;
    }

    float master_gain = (float)song->global_vol / 128.0f * (float)song->mix_vol / 64.0f * 0.5f;

    for (int vi = 0; vi < IT_MAX_VIRTUAL_CHANNELS; vi++) {
        ITVoice* v = &s_voices[vi];
        if (!v->active) continue;

        int smp_idx = v->sample_num;
        if (smp_idx < 1 || smp_idx >= IT_MAX_SAMPLES) {
            v->active = false;
            continue;
        }

        const ITSample* smp = &song->samples[smp_idx];
        if (!smp->data || smp->length == 0) {
            v->active = false;
            continue;
        }

        int ch = v->host_channel;
        bool is_muted = song->channel_mute[ch];

        /* Panning calculation */
        float pan = v->pan_final; /* -1.0 .. +1.0 */
        float left_gain = sqrtf(0.5f * (1.0f - pan));
        float right_gain = sqrtf(0.5f * (1.0f + pan));

        /* Fadeout component */
        float fade = (v->fadeout_count > 0) ? (float)v->fadeout_count / 1024.0f : 0.0f;
        float total_vol = v->vol_final * fade * master_gain;

        bool has_loop = (smp->flags & SMP_FLAG_LOOP) && (smp->loop_end > smp->loop_start);
        uint32_t loop_st = smp->loop_start;
        uint32_t loop_en = smp->loop_end;

        float voice_peak = 0.0f;

        for (size_t f = 0; f < num_frames; f++) {
            uint32_t pos_int = (uint32_t)v->sample_pos;
            if (pos_int >= smp->length) {
                if (has_loop) {
                    uint32_t l_len = loop_en - loop_st;
                    v->sample_pos = loop_st + fmod(v->sample_pos - loop_st, (double)l_len);
                    pos_int = (uint32_t)v->sample_pos;
                } else {
                    v->active = false;
                    break;
                }
            }

            float frac = (float)(v->sample_pos - (double)pos_int);
            float s0 = smp->data[pos_int];
            float s1 = (pos_int + 1 < smp->length) ? smp->data[pos_int + 1] : (has_loop ? smp->data[loop_st] : s0);
            float sample_val = s0 + frac * (s1 - s0);

            /* Resonant low-pass filter (State Variable Filter) */
            if (v->filter_enabled && v->filter_cutoff < 127.0f) {
                float fc = (float)v->filter_cutoff / 128.0f * 0.45f;
                float q = 1.0f - ((float)v->filter_resonance / 128.0f) * 0.95f;
                float f_val = 2.0f * sinf((float)M_PI * fc);
                v->flt_low[0] += f_val * v->flt_band[0];
                float high = sample_val - v->flt_low[0] - q * v->flt_band[0];
                v->flt_band[0] += f_val * high;
                sample_val = v->flt_low[0];
            }

            sample_val *= total_vol;

            float abs_val = fabsf(sample_val);
            if (abs_val > voice_peak) voice_peak = abs_val;

            if (!is_muted) {
                out_interleaved[f * 2 + 0] += sample_val * left_gain;
                out_interleaved[f * 2 + 1] += sample_val * right_gain;
            }

            v->sample_pos += v->sample_step;
        }

        /* Update channel visualization meters */
        if (voice_peak > s_chan_vis[ch].vu_level) {
            s_chan_vis[ch].vu_level = voice_peak;
        }
        int wr = s_chan_vis[ch].scope_write_pos;
        s_chan_vis[ch].scope[wr] = voice_peak;
        s_chan_vis[ch].scope_write_pos = (wr + 1) % IT_SCOPE_BUFFER_SIZE;
    }

    /* Soft limiter / clipper to avoid harsh digital overflow & compute master VU peak */
    float peak_l = 0.0f, peak_r = 0.0f;
    for (size_t i = 0; i < num_frames; i++) {
        float l = out_interleaved[i * 2 + 0];
        float r = out_interleaved[i * 2 + 1];

        if (l > 1.0f) l = 1.0f - expf(-l);
        else if (l < -1.0f) l = -1.0f + expf(l);

        if (r > 1.0f) r = 1.0f - expf(-r);
        else if (r < -1.0f) r = -1.0f + expf(r);

        out_interleaved[i * 2 + 0] = l;
        out_interleaved[i * 2 + 1] = r;

        float al = fabsf(l);
        float ar = fabsf(r);
        if (al > peak_l) peak_l = al;
        if (ar > peak_r) peak_r = ar;
    }

    if (peak_l > s_master_peak_l) s_master_peak_l = peak_l;
    if (peak_r > s_master_peak_r) s_master_peak_r = peak_r;
}

void it_mixer_get_vu_levels(float* out_l, float* out_r) {
    if (out_l) *out_l = s_master_peak_l;
    if (out_r) *out_r = s_master_peak_r;
    /* Smooth exponential decay */
    s_master_peak_l *= 0.88f;
    s_master_peak_r *= 0.88f;
}

