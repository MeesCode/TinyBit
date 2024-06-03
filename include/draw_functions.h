
#ifndef DRAW_FUNCTIONS_H
#define DRAW_FUNCTIONS_H

#include <SDL.h>
#include"main.h"

extern COLOR fillColor;
extern COLOR strokeColor;
extern int strokeWidth;

void set_fill_color(COLOR);
void set_stroke_color(COLOR);
void set_stroke_width(COLOR);

int millis();
void draw_sprite_advanced(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH, int angle, FLIP flip);
void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH);
void draw_rect(int x, int y, int w, int h);
void set_stroke(int width, int r, int g, int b, int a);
void set_fill(int r, int g, int b, int a);

#endif