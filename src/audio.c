
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
#include <stdbool.h>

#include "main.h"
#include "audio.h"

#define M_PI 3.14159265358979323846
#define GAIN 500
#define SAMPLERATE 44100

SDL_AudioDeviceID audio_device;
SDL_AudioSpec audio_spec;
int bpm = 100;
int volume = 10;

int16_t* sound_buffers[5];
size_t sound_buffers_len[5];

bool parse_and_play(const char* input);

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
        sound_buffers_len[i] = 0;
    }

    parse_and_play(
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/8 C5 V3\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/4 E5 V3\n"
        "\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/4 D5 V3\n"
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/8 G5 V3\n"
        "CH1 SINE 1/4 G5 V3\n"
        "\n"
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/8 C5 V3\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/8 E5 V3\n"
        "\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/8 E5 V3\n"
        "CH1 SINE 1/8 D5 V3\n"
        "CH1 SINE 1/2 C5 V3\n"
        "\n"
        "CH2 SQUARE 1/8 C4 V3\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/8 E4 V3\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/8 C4 V3\n"
        "CH2 SQUARE 1/8 C4 V3\n"
        "CH2 SQUARE 1/4 C4 V3\n"
        "\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/4 D4 V3\n"
        "CH2 SQUARE 1/8 E4 V3\n"
        "CH2 SQUARE 1/8 G4 V3\n"
        "CH2 SQUARE 1/4 G4 V3\n"
        "\n"
        "CH2 SQUARE 1/8 C4 V3\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/8 E4 V3\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/8 C4 V3\n"
        "CH2 SQUARE 1/8 C4 V3\n"
        "CH2 SQUARE 1/8 C4 V3\n"
        "CH2 SQUARE 1/8 C4 V3\n"
        "\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/8 E4 V3\n"
        "CH2 SQUARE 1/8 D4 V3\n"
        "CH2 SQUARE 1/2 C4 V3\n"
        "\n"
        "CH3 REST 1/8 V3\n"
        "CH3 NOISE 1/8 V3\n"
        "CH3 REST 1/8 V3\n"
        "CH3 NOISE 1/8 V3\n"
        "CH3 REST 1/8 V3\n"
        "CH3 NOISE 1/8 V3\n"
        "CH3 REST 1/8 V3\n"
        "CH3 NOISE 1/8 V3\n"
        "\n"
        "CH4 SAW 1/4 G2 V3\n"
        "CH4 SAW 1/4 E3 V3\n"
        "CH4 SAW 1/4 G2 V3\n"
        "CH4 SAW 1/4 E3 V3\n"
        "\n"
        "CH4 SAW 1/4 F2 V3\n"
        "CH4 SAW 1/4 D3 V3\n"
        "CH4 SAW 1/4 F2 V3\n"
        "CH4 SAW 1/4 D3 V3\n"
        );

}

void audio_cleanup(){
    Mix_CloseAudio();
    for (int i = 0; i < 5; i++) {
        free(sound_buffers[i]);
    }
}

void queue_freq_sin(int channel, float freq, int samples, int vol) {
    if(vol < 0 || vol > 10) {
        return;
    }
    float x = 0;
    for (int i = sound_buffers_len[channel]; i < sound_buffers_len[channel] + samples; i++) {
        x += 2 * M_PI * freq/SAMPLERATE;
        sound_buffers[channel][i] = sin(x) * GAIN * vol;
    }
    sound_buffers_len[channel] += samples;
}

void queue_freq_saw(int channel, float freq, int samples, int vol) {
    if(vol < 0 || vol > 10) {
        return;
    }
    float x = 0;
    for (int i = sound_buffers_len[channel]; i < sound_buffers_len[channel] + samples; i++) {
        x += freq/SAMPLERATE;
        if (x >= 1.0f) x -= 1.0f;
        sound_buffers[channel][i] = (x * 2 - 1) * GAIN * vol;
    }
    sound_buffers_len[channel] += samples;
}

void queue_freq_square(int channel, float freq, int samples, int vol) {
    if(vol < 0 || vol > 10) {
        return;
    }
    float x = 0;
    for (int i = sound_buffers_len[channel]; i < sound_buffers_len[channel] + samples; i++) {
        x += freq/SAMPLERATE;
        if (x >= 1.0f) x -= 1.0f;
        sound_buffers[channel][i] = (x < 0.5f ? -1 : 1) * GAIN * vol;
    }
    sound_buffers_len[channel] += samples;
}

void queue_noise(int channel, int samples, int vol) {
    if(vol < 0 || vol > 10) {
        return;
    }
    for (int i = sound_buffers_len[channel]; i < sound_buffers_len[channel] + samples; i++) {
        sound_buffers[channel][i] = (rand() % ((GAIN * vol) * 2)) - (GAIN * vol);
    }
    sound_buffers_len[channel] += samples;
}

void queue_rest(int channel, int samples) {
    for (int i = sound_buffers_len[channel]; i < sound_buffers_len[channel] + samples; i++) {
        sound_buffers[channel][i] = 0;
    }
    sound_buffers_len[channel] += samples;
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
    lua_pushinteger(L, SINE);
    lua_setglobal(L, "SINE");
    lua_pushinteger(L, SAW);
    lua_setglobal(L, "SAW");
    lua_pushinteger(L, SQUARE);
    lua_setglobal(L, "SQUARE");
}

void play_noise(int ms, int vol) {

    if (vol < 0 || vol > 10) {
        return;
    }

    int samples = (SAMPLERATE * ms) / 1000;
    int16_t* buffer = (int16_t*)malloc(samples * sizeof(int16_t));

    sound_buffers_len[0] = 0;
    queue_noise(0, samples, vol);
    Mix_Chunk* chunk = Mix_QuickLoad_RAW((Uint8*)sound_buffers[0], samples * sizeof(int16_t));
    Mix_PlayChannel(0, chunk, 0);
}

void play_tone(TONE tone, int octave, int ms, WAVEFORM w, int vol) {

    if (octave < 0 || octave > 6 || tone < 0 || tone > 11 || vol < 0 || vol > 10) {
        return;
    }

    int samples = (SAMPLERATE * ms) / 1000;
    float freq = frequencies[tone] * pow(2, octave);

    sound_buffers_len[0] = 0;

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
    Mix_PlayChannel(0, chunk, 0);
}

bool parse_and_play(const char* input) {
    char line[5120];
    char* input_copy = strdup(input); // Create a modifiable copy of the input
    char* end_str;
    char* line_ptr = strtok_s(input_copy, ";\n", &end_str);

    // clear sound channels
    for (int i = 0; i < 5; i++) {
        sound_buffers_len[i] = 0;
        memset(sound_buffers[i], 0, 35000000 * sizeof(int16_t));
    }

    while (line_ptr != NULL) {
        strncpy(line, line_ptr, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0'; // Ensure null-termination

        // Skip empty lines and lines starting with "BPM"
        if (strlen(line) == 0) {
            line_ptr = strtok_s(NULL, ";\n", &end_str);
            printf("empty line\n");
            continue;
        }

        // Initialize default values
        int chan = -1; // Required
        WAVEFORM waveform = SINE; // Default waveform
        int eights = 1; // Default duration
        TONE tone = C; // Default tone
        int octave = 4; // Default octave
        int local_volume = volume; // Default volume

        // Parse the line
        char* end_token;
        char* token = strtok_s(line, " ", &end_token);
        while (token != NULL) {
            if (token[0] == 'C' && token[1] == 'H' && isdigit(token[2])) {
                chan = atoi(&token[2]);
                if(chan < 1 || chan > 4) {
                    printf("invalid channel: %s\n", token);
                    free(input_copy);
                    return false; // Invalid token
                }
            } else if (strcmp(token, "SINE") == 0) {
                waveform = SINE;
            } else if (strcmp(token, "SQUARE") == 0) {
                waveform = SQUARE;
            } else if (strcmp(token, "SAW") == 0) {
                waveform = SAW;
            } else if (strcmp(token, "NOISE") == 0) {
                waveform = NOISE;
            } else if (strcmp(token, "REST") == 0) {
                waveform = REST;
            } else if (token[0] == 'V' && isdigit(token[1])) {
                local_volume = atoi(&token[1]);
                if (local_volume < 1 || local_volume > 10) {
                    printf("invalid volume: %s\n", token);
                    free(input_copy);
                    return false; // Invalid token
                }
            } else if (strchr(token, '/')) {
                int numerator, denominator;
                sscanf(token, "%d/%d", &numerator, &denominator);
                eights = 8 * numerator / denominator;
            } else if (strlen(token) >= 2 && token[1] != 'H' && isalpha(token[0]) && isdigit(token[strlen(token) - 1])) {
                char note_char = token[0];
                octave = atoi(&token[strlen(token) - 1]);
                switch (note_char) {
                case 'A': tone = (token[1] == '#' || token[1] == 's') ? As : (token[1] == 'b' || token[1] == 'f') ? Ab : A; break;
                case 'B': tone = (token[1] == '#' || token[1] == 's') ? Bs : (token[1] == 'b' || token[1] == 'f') ? Bb : B; break;
                case 'C': tone = (token[1] == '#' || token[1] == 's') ? Cs : (token[1] == 'b' || token[1] == 'f') ? Cb : C; break;
                case 'D': tone = (token[1] == '#' || token[1] == 's') ? Ds : (token[1] == 'b' || token[1] == 'f') ? Db : D; break;
                case 'E': tone = (token[1] == '#' || token[1] == 's') ? Es : (token[1] == 'b' || token[1] == 'f') ? Eb : E; break;
                case 'F': tone = (token[1] == '#' || token[1] == 's') ? Fs : (token[1] == 'b' || token[1] == 'f') ? Fb : F; break;
                case 'G': tone = (token[1] == '#' || token[1] == 's') ? Gs : (token[1] == 'b' || token[1] == 'f') ? Gb : G; break;
                default: 
                    printf("invalid tone: %s\n", line); 
                    free(input_copy);
                    return false; // Invalid note
                }
            } else {
                printf("invalid token: %s\n", token);
                free(input_copy);
                return false; // Invalid token
            }
            token = strtok_s(NULL, " ", &end_token);
        }

        if (chan == -1) {
            printf("missing channel: %s\n", line);
            free(input_copy);
            return false; // Missing channel
        }

        int ms = (60000 / bpm) * eights;
        int samples = (SAMPLERATE * ms) / 1000;
        float freq = frequencies[tone] * pow(2, octave - 1);

        // printf("line: %s, channel %d, tone: %d, octave: %d, waveform: %d, eights: %d, ms: %d, volume: %d\n", line, chan, tone, octave, waveform, eights, ms, local_volume);

        // Call play_tone
        switch (waveform) {
        case SINE:
            queue_freq_sin(chan, freq, samples, local_volume);
            break;
        case SQUARE:
            queue_freq_square(chan, freq, samples, local_volume);
            break;
        case SAW:
            queue_freq_saw(chan, freq, samples, local_volume);
            break;
        case NOISE:
            queue_noise(chan, samples, local_volume);
            break;
        case REST:
            queue_rest(chan, samples);
            break;
        default:
            printf("invalid waveform: %s\n", line);
            free(input_copy);
            return false; // Invalid waveform
        }

        line_ptr = strtok_s(NULL, ";\n", &end_str);
    }

    for (int i = 1; i <= 4; i++) {
        Mix_Chunk* chunk = Mix_QuickLoad_RAW((Uint8*)sound_buffers[i], sound_buffers_len[i] * sizeof(int16_t));
        Mix_PlayChannel(i, chunk, -1);
    }

    free(input_copy);
    return true;
}

void set_bpm(int new_bpm) {
    bpm = new_bpm;
}

void set_volume(int new_vol) {
    if(new_vol < 0 || new_vol > 10) {
        return;
    }
    volume = new_vol;
}