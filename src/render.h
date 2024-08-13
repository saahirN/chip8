#include <SDL2/SDL.h>
#ifndef RENDER_H
#define RENDER_H

#define WIDTH 64
#define HEIGHT 32

class Screen {
    public:
        Screen();
        ~Screen(); 
        void update(int64_t* buffer); 
        
    private:
        SDL_Window* screen;
        SDL_Renderer* renderer; 
        SDL_Texture* texture; 
}; 

#endif