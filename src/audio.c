
#include <math.h>
#include <stdint.h>

#include "main.h"
#include "audio.h"

SDL_AudioDeviceID audio_device;
SDL_AudioSpec audio_spec;
int bpm = 100;

const float frequencies[12][8] = {
    { 25.96f, 51.91f, 103.83f, 207.65f, 415.30f, 830.61f, 1661.22f },
    { 27.50f, 55.00f, 110.00f, 220.00f, 440.00f, 880.00f, 1760.00f },
    { 29.14f, 58.27f, 116.54f, 233.08f, 466.16f, 932.33f, 1864.66f },
    { 30.87f, 61.74f, 123.47f, 246.94f, 493.88f, 987.77f, 1975.53f },
    { 32.70f, 65.41f, 130.81f, 261.63f, 523.25f, 1046.50f, 2093.00f },
    { 34.65f, 69.30f, 138.59f, 277.18f, 554.37f, 1108.73f, 2217.46f },
    { 36.71f, 73.42f, 146.83f, 293.66f, 587.33f, 1174.66f, 2349.32f },
    { 38.89f, 77.78f, 155.56f, 311.13f, 622.25f, 1244.51f, 2489.02f },
    { 41.20f, 82.41f, 164.81f, 329.63f, 659.26f, 1318.51f, 2637.02f },
    { 43.65f, 87.31f, 174.61f, 349.23f, 698.46f, 1396.91f, 2793.83f },
    { 46.25f, 92.50f, 185.00f, 369.99f, 739.99f, 1479.98f, 2959.96f },
    { 49.00f, 98.00f, 196.00f, 392.00f, 783.99f, 1567.98f, 3135.96f },
};

void audio_init(){
    // set up audio
    SDL_Init(SDL_INIT_AUDIO);
    SDL_zero(audio_spec);
    audio_spec.freq = 44100;
    audio_spec.format = AUDIO_S16SYS;
    audio_spec.channels = 1;
    audio_spec.samples = 1024;
    audio_spec.callback = NULL;
    audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
    SDL_PauseAudioDevice(audio_device, 0);
}

void queue_freq_sin(float freq, int ms) {
    float x = 0;
    for (int i = 0; i < (audio_spec.freq/1000) * ms; i++) {

        // TODO: fix tone
        x += freq / (audio_spec.freq / 10);

        // 5000 is the gain
        int16_t sample = sin(x) * 5000;

        const int sample_size = sizeof(int16_t) * 1;
        SDL_QueueAudio(audio_device, &sample, sample_size);
    }
}

void lua_setup_audio() {
    // set lua tone variables
    lua_pushinteger(L, C);
    lua_setglobal(L, "C");
    lua_pushinteger(L, Cs);
    lua_setglobal(L, "Cs");
    lua_pushinteger(L, Db);
    lua_setglobal(L, "Db");
    lua_pushinteger(L, D);
    lua_setglobal(L, "D");
    lua_pushinteger(L, Ds);
    lua_setglobal(L, "Ds");
    lua_pushinteger(L, Eb);
    lua_setglobal(L, "Eb");
    lua_pushinteger(L, E);
    lua_setglobal(L, "E");
    lua_pushinteger(L, F);
    lua_setglobal(L, "F");
    lua_pushinteger(L, Fs);
    lua_setglobal(L, "Fs");
    lua_pushinteger(L, Gb);
    lua_setglobal(L, "Gb");
    lua_pushinteger(L, G);
    lua_setglobal(L, "G");
    lua_pushinteger(L, Gs);
    lua_setglobal(L, "Gs");
    lua_pushinteger(L, Ab);
    lua_setglobal(L, "Ab");
    lua_pushinteger(L, A);
    lua_setglobal(L, "A");
    lua_pushinteger(L, As);
    lua_setglobal(L, "As");
    lua_pushinteger(L, Bb);
    lua_setglobal(L, "Bb");
    lua_pushinteger(L, B);
    lua_setglobal(L, "B");

    // set lua waveforms
    lua_pushinteger(L, SIN);
    lua_setglobal(L, "SIN");
}

void play_tone(TONE tone, int octave, int eights, WAVEFORM w ) {

    // tone 
    if (octave < 0 || octave > 6 || tone < 0 || tone > 11 || eights < 0) {
        return;
    }

    int ms = ((60000 / bpm) / 8) * eights;
    float freq = frequencies[tone][octave];

    switch (w) {
    case SIN:
        queue_freq_sin(freq, ms);
    }
}

void set_bpm(int new_bpm) {
    bpm = new_bpm;
}