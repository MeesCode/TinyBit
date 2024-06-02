#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <time.h>

// Screen dimensions
const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Log("Unable to initialize SDL_image: %s", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL Random Colors with Image Overlay", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture) {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* image = IMG_Load("Untitled.png");
    if (!image) {
        SDL_Log("Unable to load image: %s", IMG_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    srand((unsigned int)time(NULL));
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);

        Uint32* pixelData = (Uint32*)pixels;
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            for (int x = 0; x < SCREEN_WIDTH; ++x) {
                Uint8 r = rand() % 256;
                Uint8 g = rand() % 256;
                Uint8 b = rand() % 256;
                pixelData[y * (pitch / 4) + x] = (r << 24) | (g << 16) | (b << 8) | 255;
            }
        }

        Uint32* imageData = (Uint32*)image->pixels;
        int imageWidth = image->w;
        int imageHeight = image->h;

        int startX = 50;
        int startY = 100;

        for (int y = startY; y < imageHeight + startY; ++y) {
            for (int x = startX; x < imageWidth + startX; ++x) {


                Uint32 randomPixel = pixelData[y * (pitch / 4) + x];
                Uint32 imagePixel = imageData[((y - startY) % imageHeight) * imageWidth + ((x - startX) % imageWidth)];

                Uint8 r1 = (randomPixel >> 24) & 0xFF;
                Uint8 g1 = (randomPixel >> 16) & 0xFF;
                Uint8 b1 = (randomPixel >> 8) & 0xFF;
                Uint8 a1 = (randomPixel) & 0xFF;

                Uint8 a2 = (imagePixel >> 24) & 0xFF;
                Uint8 r2 = (imagePixel >> 16) & 0xFF;
                Uint8 g2 = (imagePixel >> 8) & 0xFF;
                Uint8 b2 = (imagePixel) & 0xFF;

                Uint8 r = (r2 * a2 + r1 * (255 - a2)) / 255;
                Uint8 g = (g2 * a2 + g1 * (255 - a2)) / 255;
                Uint8 b = (b2 * a2 + b1 * (255 - a2)) / 255;
                Uint8 a = (a1 * (255 - a2) + a2) / 255;

                pixelData[y * (pitch / 4) + x] = (r << 24) | (g << 16) | (b << 8) | a;
            }
        }

        SDL_UnlockTexture(texture);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

    }

    SDL_FreeSurface(image);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
