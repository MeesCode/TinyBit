#ifndef graphics_H
#define graphics_H

#include <SDL.h>
#include"main.h"

extern COLOR fillColor;
extern COLOR strokeColor;
extern int strokeWidth;

void set_fill_color(COLOR);
void set_stroke_color(COLOR);
void set_stroke_width(COLOR);

void lua_setup_draw();

int millis();
void draw_sprite_advanced(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH, int angle, FLIP flip);
void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH);
void draw_rect(int x, int y, int w, int h);
void set_stroke(int width, int r, int g, int b, int a);
void set_fill(int r, int g, int b, int a);
void draw_pixel(int x, int y);
void draw_cls();

#endif