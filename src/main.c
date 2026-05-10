
#ifdef _WIN32
    #include <SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "tinybit/tinybit.h"
#include "platform.h"
#include "cartridge_io.h"
#include "games.h"

struct TinyBitMemory tb_mem = {0};

#define TB_DEFAULT_FORMAT_VERSION 1
#define TB_DEFAULT_GAME_VERSION 1

static void print_usage(FILE* stream) {
    fprintf(stream, "TinyBit Virtual Console\n\n");
    fprintf(stream, "Usage:\n");
    fprintf(stream, "  tinybit\n");
    fprintf(stream, "  tinybit <file.tb.png>\n");
    fprintf(stream, "  tinybit -c [--title T] [--author A] [--flags N] [--game-version N]\n");
    fprintf(stream, "             [--format-version N] [--date UNIX_TS]\n");
    fprintf(stream, "             <sprites.png> <script.lua> <cover.png> <out.tb.png>\n");
    fprintf(stream, "  tinybit -i\n\n");
    fprintf(stream, "Options:\n");
    fprintf(stream, "  -h, --help         Show this help message\n");
    fprintf(stream, "  -c                 Export a cartridge from spritesheet, script, and cover image\n");
    fprintf(stream, "  -i                 Interactive cartridge builder\n\n");
    fprintf(stream, "Header options (used with -c; all optional):\n");
    fprintf(stream, "  --title T          Cartridge title (max 63 chars)\n");
    fprintf(stream, "  --author A         Cartridge author (max 63 chars)\n");
    fprintf(stream, "  --flags N          16-bit flags (decimal or 0x-prefixed hex)\n");
    fprintf(stream, "  --game-version N   16-bit game version (default %d)\n", TB_DEFAULT_GAME_VERSION);
    fprintf(stream, "  --format-version N 16-bit format version (default %d)\n", TB_DEFAULT_FORMAT_VERSION);
    fprintf(stream, "  --date UNIX_TS     32-bit unix timestamp (default: now)\n\n");
    fprintf(stream, "When run without arguments, starts the game selector.\n");
}

static void register_callbacks() {
    tinybit_log_cb(printf);
    tinybit_gamecount_cb(count_games);
    tinybit_gameload_cb(load_game_by_index);
    tinybit_render_cb(render_frame);
    tinybit_poll_input_cb(handle_input);
    tinybit_get_ticks_ms_cb(get_ticks_ms);
    tinybit_audio_queue_cb(audio_queue_frame);
}

static int parse_u32_arg(const char* s, uint32_t* out) {
    if (!s || !*s) return 0;
    char* end = NULL;
    errno = 0;
    unsigned long long v = strtoull(s, &end, 0);
    if (errno != 0 || end == s || *end != '\0') return 0;
    *out = (uint32_t)v;
    return 1;
}

static int parse_u16_arg(const char* s, uint16_t* out) {
    uint32_t v;
    if (!parse_u32_arg(s, &v) || v > 0xffff) return 0;
    *out = (uint16_t)v;
    return 1;
}

// Strip trailing newline and any surrounding whitespace in-place.
static void trim(char* s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' ' || s[len-1] == '\t')) {
        s[--len] = '\0';
    }
    size_t start = 0;
    while (s[start] == ' ' || s[start] == '\t') start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
}

static int prompt_line(const char* label, const char* fallback, char* out, size_t out_size) {
    printf("%s", label);
    if (fallback && *fallback) printf(" [%s]", fallback);
    printf(": ");
    fflush(stdout);
    if (!fgets(out, (int)out_size, stdin)) return 0;
    trim(out);
    if (!*out && fallback) {
        strncpy(out, fallback, out_size - 1);
        out[out_size - 1] = '\0';
    }
    return 1;
}

static int export_with_cli(int argc, char* argv[]) {
    // Defaults
    struct CartridgeHeaderOpts opts = {
        .format_version = TB_DEFAULT_FORMAT_VERSION,
        .flags = 0,
        .title = "",
        .author = "",
        .game_version = TB_DEFAULT_GAME_VERSION,
        .package_date = (uint32_t)time(NULL),
    };

    const char* positional[4] = {0};
    int pos_count = 0;

    for (int i = 2; i < argc; i++) {
        const char* a = argv[i];
        if (strcmp(a, "--title") == 0) {
            if (++i >= argc) { fprintf(stderr, "Error: --title needs a value\n"); return 1; }
            opts.title = argv[i];
        } else if (strcmp(a, "--author") == 0) {
            if (++i >= argc) { fprintf(stderr, "Error: --author needs a value\n"); return 1; }
            opts.author = argv[i];
        } else if (strcmp(a, "--flags") == 0) {
            if (++i >= argc || !parse_u16_arg(argv[i], &opts.flags)) {
                fprintf(stderr, "Error: --flags expects a 16-bit number\n"); return 1;
            }
        } else if (strcmp(a, "--game-version") == 0) {
            if (++i >= argc || !parse_u16_arg(argv[i], &opts.game_version)) {
                fprintf(stderr, "Error: --game-version expects a 16-bit number\n"); return 1;
            }
        } else if (strcmp(a, "--format-version") == 0) {
            if (++i >= argc || !parse_u16_arg(argv[i], &opts.format_version)) {
                fprintf(stderr, "Error: --format-version expects a 16-bit number\n"); return 1;
            }
        } else if (strcmp(a, "--date") == 0) {
            if (++i >= argc || !parse_u32_arg(argv[i], &opts.package_date)) {
                fprintf(stderr, "Error: --date expects a 32-bit unix timestamp\n"); return 1;
            }
        } else if (a[0] == '-' && a[1] == '-') {
            fprintf(stderr, "Error: unknown option '%s'\n\n", a);
            print_usage(stderr);
            return 1;
        } else {
            if (pos_count >= 4) {
                fprintf(stderr, "Error: too many positional arguments\n\n");
                print_usage(stderr);
                return 1;
            }
            positional[pos_count++] = a;
        }
    }

    if (pos_count != 4) {
        fprintf(stderr, "Error: -c requires exactly 4 positional arguments\n\n");
        print_usage(stderr);
        return 1;
    }

    export_cartridge(positional[0], positional[1], positional[2], positional[3], &opts);
    return 0;
}

static int export_interactive(void) {
    char sprite[1024], script[1024], cover[1024], out[1024];
    char title[TB_HEADER_TITLE_SIZE], author[TB_HEADER_AUTHOR_SIZE];
    char flags_s[64], gv_s[64], fv_s[64], date_s[64];

    printf("TinyBit interactive cartridge builder\n");
    printf("Press enter to accept defaults shown in [brackets].\n\n");

    if (!prompt_line("Spritesheet PNG path", NULL, sprite, sizeof(sprite)) || !*sprite) {
        fprintf(stderr, "Error: spritesheet path is required\n"); return 1;
    }
    if (!prompt_line("Lua script path", NULL, script, sizeof(script)) || !*script) {
        fprintf(stderr, "Error: script path is required\n"); return 1;
    }
    if (!prompt_line("Cover PNG path", NULL, cover, sizeof(cover)) || !*cover) {
        fprintf(stderr, "Error: cover path is required\n"); return 1;
    }
    if (!prompt_line("Output cartridge path (.tb.png)", NULL, out, sizeof(out)) || !*out) {
        fprintf(stderr, "Error: output path is required\n"); return 1;
    }

    prompt_line("Title", "", title, sizeof(title));
    prompt_line("Author", "", author, sizeof(author));

    struct CartridgeHeaderOpts opts = {
        .format_version = TB_DEFAULT_FORMAT_VERSION,
        .flags = 0,
        .title = title,
        .author = author,
        .game_version = TB_DEFAULT_GAME_VERSION,
        .package_date = (uint32_t)time(NULL),
    };

    char def_fv[16]; snprintf(def_fv, sizeof(def_fv), "%d", TB_DEFAULT_FORMAT_VERSION);
    char def_gv[16]; snprintf(def_gv, sizeof(def_gv), "%d", TB_DEFAULT_GAME_VERSION);
    char def_date[16]; snprintf(def_date, sizeof(def_date), "%u", opts.package_date);

    prompt_line("Format version", def_fv, fv_s, sizeof(fv_s));
    if (!parse_u16_arg(fv_s, &opts.format_version)) {
        fprintf(stderr, "Error: invalid format version\n"); return 1;
    }

    prompt_line("Flags (e.g. 0 or 0x01)", "0", flags_s, sizeof(flags_s));
    if (!parse_u16_arg(flags_s, &opts.flags)) {
        fprintf(stderr, "Error: invalid flags\n"); return 1;
    }

    prompt_line("Game version", def_gv, gv_s, sizeof(gv_s));
    if (!parse_u16_arg(gv_s, &opts.game_version)) {
        fprintf(stderr, "Error: invalid game version\n"); return 1;
    }

    prompt_line("Package date (unix timestamp)", def_date, date_s, sizeof(date_s));
    if (!parse_u32_arg(date_s, &opts.package_date)) {
        fprintf(stderr, "Error: invalid package date\n"); return 1;
    }

    printf("\nBuilding cartridge...\n");
    export_cartridge(sprite, script, cover, out, &opts);
    return 0;
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
        return export_with_cli(argc, argv);
    }

    // interactive cartridge builder
    else if (strcmp(argv[1], "-i") == 0) {
        return export_interactive();
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
