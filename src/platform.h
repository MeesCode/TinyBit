#ifndef PLATFORM_H
#define PLATFORM_H

#include "tinybit/tinybit.h"

extern struct TinyBitMemory tb_mem;

void handle_input();
void render_frame();
void audio_init();
void audio_cleanup();
void audio_queue_frame();
long get_ticks_ms();
void play_game();

#endif
