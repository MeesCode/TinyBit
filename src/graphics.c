
#include <SDL.h>
#include <SDL_image.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "graphics.h"
#include "memory.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

COLOR fillColor = 0;
COLOR strokeColor = 0;
int strokeWidth = 0;

void lua_setup_draw() {
    lua_pushinteger(L, SCREEN_WIDTH);
    lua_setglobal(L, "SCREEN_WIDTH");
    lua_pushinteger(L, SCREEN_HEIGHT);
    lua_setglobal(L, "SCREEN_HEIGHT");
}

void blend(uint8_t result[4], uint8_t fg[4], uint8_t bg[4]) {
    float alpha_fg = fg[3] / 255.0f;
    float alpha_bg = bg[3] / 255.0f;
    float alpha_out = alpha_fg + alpha_bg * (1 - alpha_fg);

    if (alpha_out > 0) {
        result[0] = (uint8_t)((fg[0] * alpha_fg + bg[0] * alpha_bg * (1 - alpha_fg)) / alpha_out);
        result[1] = (uint8_t)((fg[1] * alpha_fg + bg[1] * alpha_bg * (1 - alpha_fg)) / alpha_out);
        result[2] = (uint8_t)((fg[2] * alpha_fg + bg[2] * alpha_bg * (1 - alpha_fg)) / alpha_out);
    } else {
        result[0] = result[1] = result[2] = 0;
    }
    result[3] = (uint8_t)(alpha_out * 255);
}

int millis() {
    return clock() / (CLOCKS_PER_SEC / 1000);
}

void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH) {


    float scaleX = (float)sourceW / (float)targetW;
    float scaleY = (float)sourceH / (float)targetH;

    for (int y = 0; y < targetH; ++y) {
        for (int x = 0; x < targetW; ++x) {

            if(x + targetX < 0 || x + targetX >= SCREEN_WIDTH || y + targetY < 0 || y + targetY >= SCREEN_HEIGHT) {
                continue;
            }

            int sourcePixelX = sourceX + (int)(x * scaleX);
            int sourcePixelY = sourceY + (int)(y * scaleY);

            if(sourcePixelX < 0 || sourcePixelX >= SCREEN_WIDTH || sourcePixelY < 0 || sourcePixelY >= SCREEN_HEIGHT) {
                continue;
            }

            uint32_t *fg = &memory[MEM_SPRITESHEET_START + (sourcePixelY * SCREEN_WIDTH + sourcePixelX) * 4];
            uint32_t *bg = &memory[MEM_DISPLAY_START + ((targetY + y) * SCREEN_WIDTH + targetX + x) * 4];
            blend(bg, fg, bg);
        }
    }
}

void draw_rect(int x, int y, int w, int h) {

    uint32_t fillBackup = fillColor;
    fillColor = strokeColor;

    for (int i = 0; i < strokeWidth; i++) {
        for (int j = 0; j < w; j++) {
            draw_pixel(x + j, y + i);
            draw_pixel(x + j, y + h - i - 1);
        }
        for (int j = 0; j < h; j++) {
            draw_pixel(x + i, y + j);
            draw_pixel(x + w - i - 1, y + j);
        }
    }

    fillColor = fillBackup;

    for (int i = strokeWidth; i < w - strokeWidth; i++) {
        for (int j = strokeWidth; j < h - strokeWidth; j++) {
            draw_pixel(x + i, y + j);
        }
    }
}

// draw an oval with outline, specified by x, y, w, h
void draw_oval(int x, int y, int w, int h) {
    // backup fill color
    uint32_t fillBackup = fillColor;

    // draw the fill
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {

            bool instroke = false;
            bool infill = false;

            // check if the point is inside the oval
            if ((i - w / 2) * (i - w / 2) * h * h + (j - h / 2) * (j - h / 2) * w * w <= w * w * h * h / 4) {
                infill = true;
            }

            // check if we are in the stroke area
            if ((i - w / 2) * (i - w / 2) * h * h + (j - h / 2) * (j - h / 2) * w * w >= (w - strokeWidth) * (w - strokeWidth) * h * h / 4) {
                instroke = true;
            }

            if(!infill) {
                continue;
            }

            // if the point is inside the oval but outside the stroke, draw the fill
            if (!instroke) {
                fillColor = fillBackup;
                draw_pixel(x + i, y + j);
            } else {
                fillColor = strokeColor;
                draw_pixel(x + i, y + j);
            }
        }
    }
}

void set_stroke(int width, int r, int g, int b, int a) {
    strokeWidth = width >= 0 ? width : 0;
    strokeColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void set_fill(int r, int g, int b, int a) {
    fillColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void draw_pixel(int x, int y) {
    uint32_t* bg = &memory[MEM_DISPLAY_START + (x + (y * SCREEN_WIDTH)) * 4];
    blend(bg, &fillColor, bg);
}

void draw_cls() {
    memset(&memory[MEM_DISPLAY_START], 0, MEM_DISPLAY_SIZE);
}