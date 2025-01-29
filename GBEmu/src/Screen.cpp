#include "common.h"
#include "Screen.h"


SDL_Window* Screen::window = nullptr;
SDL_Renderer* Screen::renderer = nullptr;

SDL_AppResult Screen::InitializeScreen()
{
    SDL_SetAppMetadata("GBEmu", "1.0", "ITaeI");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(title.c_str(), SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}