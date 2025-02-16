#include "common.h"
#include "Screen.h"
#include "Emulator.h"

namespace GBEmu
{

    void Screen::connectScreen(Emulator* emu)
    {
        Emu = emu;
    }

    SDL_Window* Screen::window = nullptr;
    SDL_Renderer* Screen::renderer = nullptr;
    SDL_Texture *Screen::texture = nullptr;
    SDL_Surface *Screen::surface = nullptr;

    SDL_AppResult Screen::InitializeScreen(std::string Name, int Width, int Height)
    {
        title = Name; SCREEN_WIDTH = Width; SCREEN_HEIGHT = Height;
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

    void Screen::pollForEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                Emu->exit = true;
            }
        }
    }

}