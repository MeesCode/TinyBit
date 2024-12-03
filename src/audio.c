
#include "SDL_mixer.h"

#ifdef _WIN32
    #include "SDL_mixer.h"
#else
    #include <SDL2/SDL_mixer.h>
#endif

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "audio.h"

#define M_PI 3.14159265358979323846
#define GAIN 500
#define SAMPLERATE 44100

SDL_AudioDeviceID audio_device;
SDL_AudioSpec audio_spec;
int bpm = 100;
int channel = 0;
int volume = 10;

int16_t* sound_buffers[5];
size_t sound_buffer_len[5];

void parse_and_play(const char* input);

const float frequencies[12] = {
    16.35f,
    17.32f,
    18.35f,
    19.45f,
    20.60f,
    21.83f,
    23.12f,
    24.50f,
    25.96f,
    27.50f,
    29.14f,
    30.87f,
};

void audio_init(){
    // set up audio
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(SAMPLERATE, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_AllocateChannels(5);

    for (int i = 0; i < 5; i++) {
        sound_buffers[i] = (int16_t*)calloc(35000000, sizeof(int16_t));
        sound_buffer_len[i] = 0;
    }

    parse_and_play(
        "C1 SINE E5 1/8\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE C5 1/8\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE E5 1/8\n"
        "C1 SINE E5 1/8\n"
        "C1 SINE E5 1/4\n"
        "\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE D5 1/4\n"
        "C1 SINE E5 1/8\n"
        "C1 SINE G5 1/8\n"
        "C1 SINE G5 1/4\n"
        "\n"
        "C1 SINE E5 1/8\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE C5 1/8\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE E5 1/8\n"
        "C1 SINE E5 1/8\n"
        "C1 SINE E5 1/8\n"
        "C1 SINE E5 1/8\n"
        "\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE E5 1/8\n"
        "C1 SINE D5 1/8\n"
        "C1 SINE C5 1/2\n"
        "\n"
        "C2 SQUARE C4 1/8\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE E4 1/8\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE C4 1/8\n"
        "C2 SQUARE C4 1/8\n"
        "C2 SQUARE C4 1/4\n"
        "\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE D4 1/4\n"
        "C2 SQUARE E4 1/8\n"
        "C2 SQUARE G4 1/8\n"
        "C2 SQUARE G4 1/4\n"
        "\n"
        "C2 SQUARE C4 1/8\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE E4 1/8\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE C4 1/8\n"
        "C2 SQUARE C4 1/8\n"
        "C2 SQUARE C4 1/8\n"
        "C2 SQUARE C4 1/8\n"
        "\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE E4 1/8\n"
        "C2 SQUARE D4 1/8\n"
        "C2 SQUARE C4 1/2\n"
        "\n"
        "C3 REST A 1/8\n"
        "C3 NOISE C3 1/8\n"
        "C3 REST A 1/8\n"
        "C3 NOISE C3 1/8\n"
        "C3 REST A 1/8\n"
        "C3 NOISE C3 1/8\n"
        "C3 REST A 1/8\n"
        "C3 NOISE C3 1/8\n"
        "\n"
        "C4 SAW G2 1/4\n"
        "C4 SAW E3 1/4\n"
        "C4 SAW G2 1/4\n"
        "C4 SAW E3 1/4\n"
        "\n"
        "C4 SAW F2 1/4\n"
        "C4 SAW D3 1/4\n"
        "C4 SAW F2 1/4\n"
        "C4 SAW D3 1/4\n");

}

void queue_freq_sin(int channel, float freq, int samples, int vol) {
    if(vol < 0 || vol > 10) {
        return;
    }
    float x = 0;
    for (int i = sound_buffer_len[channel]; i < sound_buffer_len[channel] + samples; i++) {
        x += 2 * M_PI * freq/SAMPLERATE;
        sound_buffers[channel][i] = sin(x) * GAIN * vol;
    }
    sound_buffer_len[channel] += samples;
}

void queue_freq_saw(int channel, float freq, int samples, int vol) {
    if(vol < 0 || vol > 10) {
        return;
    }
    float x = 0;
    for (int i = sound_buffer_len[channel]; i < sound_buffer_len[channel] + samples; i++) {
        x += freq/SAMPLERATE;
        if (x >= 1.0f) x -= 1.0f;
        sound_buffers[channel][i] = (x * 2 - 1) * GAIN * vol;
    }
    sound_buffer_len[channel] += samples;
}

void queue_freq_square(int channel, float freq, int samples, int vol) {
    if(vol < 0 || vol > 10) {
        return;
    }
    float x = 0;
    for (int i = sound_buffer_len[channel]; i < sound_buffer_len[channel] + samples; i++) {
        x += freq/SAMPLERATE;
        if (x >= 1.0f) x -= 1.0f;
        sound_buffers[channel][i] = (x < 0.5f ? -1 : 1) * GAIN * vol;
    }
    sound_buffer_len[channel] += samples;
}

void queue_noise(int channel, int samples, int vol) {
    if(vol < 0 || vol > 10) {
        return;
    }
    for (int i = sound_buffer_len[channel]; i < sound_buffer_len[channel] + samples; i++) {
        sound_buffers[channel][i] = (rand() % ((GAIN * vol) * 2)) - (GAIN * vol);
    }
    sound_buffer_len[channel] += samples;
}

void queue_rest(int channel, int samples) {
    for (int i = sound_buffer_len[channel]; i < sound_buffer_len[channel] + samples; i++) {
        sound_buffers[channel][i] = 0;
    }
    sound_buffer_len[channel] += samples;
}

void lua_setup_audio() {
    // set lua tone variables
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

    // set lua waveforms
    lua_pushinteger(L, SINE);
    lua_setglobal(L, "SINE");
    lua_pushinteger(L, SAW);
    lua_setglobal(L, "SAW");
    lua_pushinteger(L, SQUARE);
    lua_setglobal(L, "SQUARE");
}

void play_noise(int eights, int vol, int chan) {

    if (eights < 0 || vol < 0 || vol > 10 || chan < 0 || chan > 3) {
        return;
    }

    int ms = (60000 / bpm) * eights;
    int samples = (SAMPLERATE * ms) / 1000;
    int16_t* buffer = (int16_t*)malloc(samples * sizeof(int16_t));

    sound_buffer_len[0] = 0;
    queue_noise(0, samples, vol);
    Mix_Chunk* chunk = Mix_QuickLoad_RAW((Uint8*)sound_buffers[0], samples * sizeof(int16_t));
    Mix_PlayChannel(chan, chunk, 0);
}

void play_tone(TONE tone, int octave, int eights, WAVEFORM w, int vol, int chan) {

    if (octave < 0 || octave > 6 || tone < 0 || tone > 11 || eights < 0 || vol < 0 || vol > 10 || chan < 0 || chan > 3) {
        return;
    }

    int ms = (60000 / bpm) * eights;
    int samples = (SAMPLERATE * ms) / 1000;
    float freq = frequencies[tone] * pow(2, octave);

    sound_buffer_len[0] = 0;

    switch (w) {
    case SINE:
        queue_freq_sin(0, freq, samples, vol);
        break;
    case SAW:
        queue_freq_saw(0, freq, samples, vol);
        break;
    case SQUARE:
        queue_freq_square(0, freq, samples, vol);
        break;
    }
    Mix_Chunk* chunk = Mix_QuickLoad_RAW((Uint8*)sound_buffers[0], samples * sizeof(int16_t));
    Mix_PlayChannel(chan, chunk, 0);
}

void parse_and_play(const char* input) {
    char line[256];
    char* input_copy = strdup(input); // Create a modifiable copy of the input
    char* line_ptr = strtok(input_copy, "\n");

    // clear sound channels
    for (int i = 0; i < 5; i++) {
        sound_buffer_len[i] = 0;
    }

    while (line_ptr != NULL) {
        strncpy(line, line_ptr, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0'; // Ensure null-termination

        // Skip empty lines and lines starting with "BPM"
        if (strlen(line) == 0) {
            line_ptr = strtok(NULL, "\n");
            printf("empty line\n");
            continue;
        }

        // Parse the line
        int chan;
        char waveform_str[16], note_str[8], duration_str[8];
        if (sscanf(line, "C%d %15s %7s %7s", &chan, waveform_str, note_str, duration_str) != 4) {
            printf("invalid line: %s\n", line);
            line_ptr = strtok(NULL, "\n");
            continue; // Skip invalid lines
        }

        // Parse waveform
        WAVEFORM waveform;
        if (strcmp(waveform_str, "SINE") == 0) {
            waveform = SINE;
        }
        else if (strcmp(waveform_str, "SQUARE") == 0) {
            waveform = SQUARE;
        }
        else if (strcmp(waveform_str, "SAW") == 0) {
            waveform = SAW;
        }
        else if (strcmp(waveform_str, "NOISE") == 0) {
            waveform = NOISE;
        }
        else if (strcmp(waveform_str, "REST") == 0) {
            waveform = REST;
        }
        else {
            printf("invalid waveform: %s\n", line);
            line_ptr = strtok(NULL, "\n");
            continue; // Skip invalid waveforms
        }

        // Parse note and octave
        char note_char = note_str[0];
        int octave = atoi(&note_str[strlen(note_str) - 1]);
        TONE tone;
        switch (note_char) {
        case 'A': tone = (note_str[1] == '#' || note_str[1] == 's') ? As : (note_str[1] == 'b' || note_str[1] == 'f') ? Ab : A; break;
        case 'B': tone = (note_str[1] == '#' || note_str[1] == 's') ? Bs : (note_str[1] == 'b' || note_str[1] == 'f') ? Bb : B; break;
        case 'C': tone = (note_str[1] == '#' || note_str[1] == 's') ? Cs : (note_str[1] == 'b' || note_str[1] == 'f') ? Cb : C; break;
        case 'D': tone = (note_str[1] == '#' || note_str[1] == 's') ? Ds : (note_str[1] == 'b' || note_str[1] == 'f') ? Db : D; break;
        case 'E': tone = (note_str[1] == '#' || note_str[1] == 's') ? Es : (note_str[1] == 'b' || note_str[1] == 'f') ? Eb : E; break;
        case 'F': tone = (note_str[1] == '#' || note_str[1] == 's') ? Fs : (note_str[1] == 'b' || note_str[1] == 'f') ? Fb : F; break;
        case 'G': tone = (note_str[1] == '#' || note_str[1] == 's') ? Gs : (note_str[1] == 'b' || note_str[1] == 'f') ? Gb : G; break;
        default: 
            printf("invalid tone: %s\n", line); 
            line_ptr = strtok(NULL, "\n"); 
            continue; // Skip invalid notes
        }

        // Parse duration as eights
        int eights = atoi(duration_str);
        if (strstr(duration_str, "/")) {
            int numerator, denominator;
            sscanf(duration_str, "%d/%d", &numerator, &denominator);
            eights = 8 * numerator / denominator;
        }

        int ms = (60000 / bpm) * eights;
        int samples = (SAMPLERATE * ms) / 1000;
        float freq = frequencies[tone] * pow(2, octave);

        // printf("line: %s, channel %d, tone: %d, octave: %d, waveform: %d, eights: %d, ms: %d\n", line, chan, tone, octave, waveform, eights, ms);

        // Call play_tone
        switch (waveform) {
        case SINE:
            queue_freq_sin(chan, freq, samples, 2);
            break;
        case SQUARE:
            queue_freq_square(chan, freq, samples, 2);
            break;
        case SAW:
            queue_freq_saw(chan, freq, samples, 2);
            break;
        case NOISE:
            queue_noise(chan, samples, 2);
            break;
        case REST:
            queue_rest(chan, samples);
            break;
        default:
            printf("invalid waveform: %s\n", line);
            continue;
        }

        line_ptr = strtok(NULL, "\n");
    }

    for (int i = 1; i <= 4; i++) {
        // printf("channel %d length: %d\n", i, sound_buffer_len[i]);
        Mix_Chunk* chunk = Mix_QuickLoad_RAW((Uint8*)sound_buffers[i], sound_buffer_len[i] * sizeof(int16_t));
        Mix_PlayChannel(i, chunk, -1);
    }

    free(input_copy);
}

void set_bpm(int new_bpm) {
    bpm = new_bpm;
}

void set_channel(int new_chan) {
    if(new_chan < 0 || new_chan > 3) {
        return;
    }
    channel = new_chan;
}

void set_volume(int new_vol) {
    if(new_vol < 0 || new_vol > 10) {
        return;
    }
    volume = new_vol;
}