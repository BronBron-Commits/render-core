#include "core/app.hpp"
#include <GL/glew.h>
#include <cstdio>

bool App::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init failed\n");
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(
        "Lumen Social",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::fprintf(stderr, "Window creation failed\n");
        return false;
    }

    gl = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::fprintf(stderr, "GLEW init failed\n");
        return false;
    }

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    last_ticks = SDL_GetPerformanceCounter();
    return true;
}

void App::shutdown()
{
    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool App::running() const
{
    return alive;
}

float App::frame_dt()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            alive = false;
        }
    }

    Uint64 now = SDL_GetPerformanceCounter();
    float dt = float(now - last_ticks) /
               float(SDL_GetPerformanceFrequency());
    last_ticks = now;

    return dt;
}

void App::present()
{
    SDL_GL_SwapWindow(window);
}
