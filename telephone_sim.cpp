#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <sys/time.h>

constexpr float PI2 = 2.0 * M_PI;
constexpr float SAMPLE_FREQ = 8000.0;
constexpr int SAMPLES_PER_MS = SAMPLE_FREQ / 1000;

typedef struct {
    float freq;
    float mod;
} DTFM_KEY;

const DTFM_KEY DTFM_KEY_0 = { 941, 1336 };
const DTFM_KEY DTFM_KEY_1 = { 697, 1209 };
const DTFM_KEY DTFM_KEY_2 = { 697, 1336 };
const DTFM_KEY DTFM_KEY_3 = { 697, 1477 };
const DTFM_KEY DTFM_KEY_4 = { 770, 1209 };
const DTFM_KEY DTFM_KEY_5 = { 770, 1336 };
const DTFM_KEY DTFM_KEY_6 = { 770, 1477 };
const DTFM_KEY DTFM_KEY_7 = { 852, 1209 };
const DTFM_KEY DTFM_KEY_8 = { 852, 1336 };
const DTFM_KEY DTFM_KEY_9 = { 852, 1477 };

// Super dagnerous, no bounds checking, don't try this at home
inline void silence(float* stream, int* idx, int ms)
{

    int len_silence = SAMPLES_PER_MS * ms;

    for (int i = *idx; i < *idx + len_silence; i++) {
        stream[i] = 0;
    }
    *idx += len_silence;
}

inline void ring(float* stream, int* idx, int ms, int carrier_freq, int mod_freq)
{
    static const float volume = 0.2;
    int len_ring = SAMPLES_PER_MS * ms;

    for (int i = *idx; i < *idx + len_ring; i++) {
        double time = i / SAMPLE_FREQ;
        stream[i] = volume * (sin(carrier_freq * PI2 * time) + sin(mod_freq * PI2 * time));
    }
    *idx += len_ring;
}

int ring_tone(float* stream)
{
    float* fstream = (float*)(stream);
    int len_ring = 0;
    int idx = 0;
    int carrier_freq = 440;
    int modulation_freq = 480;

    ring(fstream, &idx, 400, carrier_freq, modulation_freq);
    silence(fstream, &idx, 200);
    ring(fstream, &idx, 400, carrier_freq, modulation_freq);
    silence(fstream, &idx, 2000);
    std::cout << idx << std::endl;
    return idx;
}

int call_number_tone(float* stream)
{
    float* fstream = (float*)(stream);
    int len_ring = 0;
    int idx = 0;
    int sleeptime = 70;
    int presstime = 150;

    ring(fstream, &idx, presstime, DTFM_KEY_8.freq, DTFM_KEY_8.mod);
    silence(fstream, &idx, sleeptime);
    ring(fstream, &idx, presstime, DTFM_KEY_1.freq, DTFM_KEY_1.mod);
    silence(fstream, &idx, sleeptime);
    ring(fstream, &idx, presstime, DTFM_KEY_5.freq, DTFM_KEY_5.mod);
    silence(fstream, &idx, sleeptime);
    ring(fstream, &idx, presstime, DTFM_KEY_4.freq, DTFM_KEY_4.mod);
    silence(fstream, &idx, sleeptime);
    ring(fstream, &idx, presstime, DTFM_KEY_9.freq, DTFM_KEY_9.mod);
    silence(fstream, &idx, sleeptime);
    ring(fstream, &idx, presstime, DTFM_KEY_3.freq, DTFM_KEY_3.mod);
    silence(fstream, &idx, sleeptime);
    ring(fstream, &idx, presstime, DTFM_KEY_0.freq, DTFM_KEY_0.mod);
    silence(fstream, &idx, sleeptime);
    ring(fstream, &idx, presstime, DTFM_KEY_0.freq, DTFM_KEY_0.mod);
    silence(fstream, &idx, sleeptime);
    std::cout << idx << std::endl;
    return idx;
}

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Error initializing SDL. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_AudioSpec ring_tone_spec_want, ring_tone_spec;
    SDL_memset(&ring_tone_spec_want, 0, sizeof(ring_tone_spec_want));

    ring_tone_spec_want.freq = 8000;
    ring_tone_spec_want.format = AUDIO_F32;
    ring_tone_spec_want.channels = 1;
    ring_tone_spec_want.samples = 32 * 1024;
    ring_tone_spec_want.callback = NULL;
    ring_tone_spec_want.userdata = NULL;

    float stream[32 * 1024];

    SDL_AudioDeviceID audio_device_id = SDL_OpenAudioDevice(
        NULL, 0,
        &ring_tone_spec_want, &ring_tone_spec,
        SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (!audio_device_id) {
        fprintf(stderr, "Error creating SDL audio device. SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    bool pause = false;
    uint32_t t1, t2;
    uint32_t elapsedTime = 0;
    bool running = true;
    t1 = SDL_GetTicks();
    std::cout << "Generate samples" << std::endl;
    float number_stream[24 * 1024];
    int samples_played = call_number_tone(number_stream);
    int sample_size = sizeof(float) * samples_played;
    SDL_QueueAudio(audio_device_id, number_stream, sample_size);
    samples_played = ring_tone(stream);
    sample_size = sizeof(float) * samples_played;
    float stream2[24 * 1024];
    SDL_QueueAudio(audio_device_id, stream, sample_size);
    samples_played = ring_tone(stream2);
    SDL_QueueAudio(audio_device_id, stream2, sample_size);
    SDL_PauseAudioDevice(audio_device_id, 0);
    SDL_Delay(8000);

    SDL_CloseAudioDevice(audio_device_id);
    SDL_Quit();

    return 0;
}
