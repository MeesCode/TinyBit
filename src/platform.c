
#ifdef _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "tinybit/tinybit.h"
#include "platform.h"

static SDL_Renderer* renderer;
static SDL_Window* window;
static SDL_Texture* render_target;
static SDL_AudioDeviceID audio_device = 0;
static bool game_running = true;

#ifdef _POSIX_C_SOURCE
static struct timespec start_time = { 0, 0 };
#else
static clock_t start_time = 0;
#endif

void handle_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event));
    const char* state = (const char*)SDL_GetKeyboardState(NULL);

    if (event.type == SDL_QUIT || state[(SDL_Scancode) SDL_SCANCODE_ESCAPE] == 1) {
        game_running = false;
    }

    tb_mem.button_input[TB_BUTTON_A] = (state[(SDL_Scancode) SDL_SCANCODE_A] == 1);
    tb_mem.button_input[TB_BUTTON_B] = (state[(SDL_Scancode) SDL_SCANCODE_B] == 1);
    tb_mem.button_input[TB_BUTTON_UP] = (state[(SDL_Scancode) SDL_SCANCODE_UP] == 1);
    tb_mem.button_input[TB_BUTTON_DOWN] = (state[(SDL_Scancode) SDL_SCANCODE_DOWN] == 1);
    tb_mem.button_input[TB_BUTTON_LEFT] = (state[(SDL_Scancode) SDL_SCANCODE_LEFT] == 1);
    tb_mem.button_input[TB_BUTTON_RIGHT] = (state[(SDL_Scancode) SDL_SCANCODE_RIGHT] == 1);
    tb_mem.button_input[TB_BUTTON_START] = (state[(SDL_Scancode) SDL_SCANCODE_RETURN] == 1);
    tb_mem.button_input[TB_BUTTON_SELECT] = (state[(SDL_Scancode) SDL_SCANCODE_BACKSPACE] == 1);
}

void render_frame() {
    SDL_SetRenderTarget(renderer, render_target);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    uint32_t* pixels;
    int pitch;
    SDL_LockTexture(render_target, NULL, (void**)&pixels, &pitch);

    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {
            uint8_t r = (tb_mem.display[(y * TB_SCREEN_WIDTH + x) * 2 + 0] << 0) & 0xf0;
            uint8_t g = (tb_mem.display[(y * TB_SCREEN_WIDTH + x) * 2 + 0] << 4) & 0xf0;
            uint8_t b = (tb_mem.display[(y * TB_SCREEN_WIDTH + x) * 2 + 1] << 0) & 0xf0;
            uint8_t a = (tb_mem.display[(y * TB_SCREEN_WIDTH + x) * 2 + 1] << 4) & 0xf0;

            pixels[y * (pitch / 4) + x] = r << 24 | g << 16 | b << 8 | a;
        }
    }

    SDL_UnlockTexture(render_target);

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, render_target, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void audio_init() {
    SDL_AudioSpec desired, obtained;

    SDL_memset(&desired, 0, sizeof(desired));
    desired.freq = TB_AUDIO_SAMPLE_RATE;
    desired.format = AUDIO_S16LSB;
    desired.channels = 1;
    desired.samples = TB_AUDIO_FRAME_SAMPLES;
    desired.callback = NULL;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (audio_device == 0) {
        printf("Failed to open audio device: %s\n", SDL_GetError());
        return;
    }

    SDL_ClearQueuedAudio(audio_device);
    SDL_PauseAudioDevice(audio_device, 0);
}

void audio_cleanup() {
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
}

void audio_queue_frame() {
    if (audio_device == 0) {
        return;
    }

    if (SDL_QueueAudio(audio_device, tb_mem.audio_buffer, TB_AUDIO_FRAME_SAMPLES * sizeof(int16_t)) != 0) {
        printf("Failed to queue audio: %s\n", SDL_GetError());
    }
}

long get_ticks_ms() {
    #ifdef _POSIX_C_SOURCE
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        if (start_time.tv_sec == 0 && start_time.tv_nsec == 0) {
            start_time = current_time;
            return 0;
        }

        long elapsed_sec = current_time.tv_sec - start_time.tv_sec;
        long elapsed_nsec = current_time.tv_nsec - start_time.tv_nsec;

        return (elapsed_sec * 1000) + (elapsed_nsec / 1000000);
    #else
        clock_t current_time = clock();

        if (start_time == 0) {
            start_time = current_time;
            return 0;
        }

        return (current_time - start_time) * 1000 / CLOCKS_PER_SEC;
    #endif
}

void play_game() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow("TinyBit Virtual Console", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    render_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(render_target, SDL_BLENDMODE_BLEND);

    audio_init();

    while (game_running) {
        tinybit_loop();

        while (SDL_GetQueuedAudioSize(audio_device) > TB_AUDIO_FRAME_SAMPLES * sizeof(int16_t) * 2) {
            SDL_Delay(1);
        }
    }

    tinybit_stop();
    SDL_DestroyTexture(render_target);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    audio_cleanup();
    SDL_Quit();
}
