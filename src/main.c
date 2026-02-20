
#ifdef _WIN32
    #include <SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tinybit/tinybit.h"
#include "platform.h"
#include "cartridge_io.h"
#include "games.h"

struct TinyBitMemory tb_mem = {0};

static void print_usage(FILE* stream) {
    fprintf(stream, "TinyBit Virtual Console\n\n");
    fprintf(stream, "Usage:\n");
    fprintf(stream, "  tinybit\n");
    fprintf(stream, "  tinybit <file.tb.png>\n");
    fprintf(stream, "  tinybit -c <sprites.png> <script.lua> <cover.png> <out.tb.png>\n\n");
    fprintf(stream, "Options:\n");
    fprintf(stream, "  -h, --help  Show this help message\n");
    fprintf(stream, "  -c          Export a cartridge from spritesheet, script, and cover image\n\n");
    fprintf(stream, "When run without arguments, starts the game selector.\n");
}

static void register_callbacks() {
    tinybit_log_cb(printf);
    tinybit_gamecount_cb(count_games);
    tinybit_gameload_cb(load_game_by_index);
    tinybit_render_cb(render_frame);
    tinybit_poll_input_cb(handle_input);
    tinybit_sleep_cb(SDL_Delay);
    tinybit_get_ticks_ms_cb(get_ticks_ms);
    tinybit_audio_queue_cb(audio_queue_frame);
}

int main(int argc, char* argv[]) {
    register_callbacks();

    // check for help flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(stdout);
            return 0;
        }
    }

    // start game selector ui
    if (argc == 1) {
        tinybit_init(&tb_mem);
        tinybit_start();
        play_game();
    }

    // export game to cartridge file
    else if (strcmp(argv[1], "-c") == 0) {
        if (argc != 6) {
            fprintf(stderr, "Error: -c requires exactly 4 arguments\n\n");
            print_usage(stderr);
            return 1;
        }
        export_cartridge(argv[2], argv[3], argv[4], argv[5]);
    }

    // reject unknown flags
    else if (argv[1][0] == '-') {
        fprintf(stderr, "Error: unknown option '%s'\n\n", argv[1]);
        print_usage(stderr);
        return 1;
    }

    // load game from file
    else if (argc == 2) {
        size_t len = strlen(argv[1]);
        if (len < 7 || strcmp(argv[1] + len - 7, ".tb.png") != 0) {
            fprintf(stderr, "Error: '%s' is not a valid cartridge file (expected .tb.png)\n\n", argv[1]);
            print_usage(stderr);
            return 1;
        }

        tinybit_init(&tb_mem);
        printf("Loading game: %s\n", argv[1]);
        load_game(argv[1]);
        tinybit_start();
        play_game();
    }

    // unexpected arguments
    else {
        fprintf(stderr, "Error: unexpected arguments\n\n");
        print_usage(stderr);
        return 1;
    }

    return 0;
}
