#pragma once

#include <SDL.h>

class App {
public:
    bool init();
    void shutdown();

    bool running() const;
    float frame_dt();
    void present();

private:
    SDL_Window* window = nullptr;
    SDL_GLContext gl = nullptr;

    bool alive = true;
    Uint64 last_ticks = 0;
};
