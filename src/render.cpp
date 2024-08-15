#include <SDL2/SDL.h>
#include <iostream> 
#include "render.h"

Screen::Screen() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL\n"; 
        return; 
    }

    screen = SDL_CreateWindow("Chip8", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED,
        WIDTH * 10,
        HEIGHT * 10,
        SDL_WINDOW_SHOWN);
    if(screen == nullptr) {
        std::cerr << "Failed to create screen\n"; 
        return; 
    }

    renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED); 
    if(renderer == nullptr) {
        std::cerr << "Failed to create renderer\n"; 
        return; 
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if(renderer == nullptr) {
        std::cerr << "Failed to create texture\n"; 
        return; 
    }


}

Screen::~Screen() {
    SDL_DestroyTexture(texture); 
    SDL_DestroyRenderer(renderer); 
    SDL_DestroyWindow(screen); 
    SDL_Quit();
    return; 
}

void Screen::update(int64_t *buffer) {
    SDL_UpdateTexture(texture, nullptr, (void *) buffer, WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer); 
    SDL_RenderCopy(renderer, texture, nullptr, nullptr); 
    SDL_RenderPresent(renderer); 
    return; 
}
