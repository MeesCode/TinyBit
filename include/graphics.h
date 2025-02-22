#ifndef graphics_H
#define graphics_H

#include"main.h"
#include <stdint.h>

extern uint32_t fillColor;
extern uint32_t fillColor2;
extern uint32_t strokeColor;
extern int strokeWidth;
extern uint16_t fillPattern;

void set_fill_color(uint32_t);
void set_stroke_color(uint32_t);
void set_stroke_width(uint32_t);

void lua_setup_draw();

int millis();
int random_range(int, int);
void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH);
void draw_rect(int x, int y, int w, int h);
void draw_oval(int x, int y, int w, int h);
void set_stroke(int width, int r, int g, int b, int a);
void set_fill(int r, int g, int b, int a);
void set_fill2(int r, int g, int b, int a);
void draw_pixel(int x, int y);
void draw_pixel_fill(int x, int y);
void draw_cls();
void set_fill_pattern(uint16_t);

#endif