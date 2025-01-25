#pragma once 
#include "common.h"

class Screen
{
    private:
    //Screen dimensions
    const int SCREEN_WIDTH;
    const int SCREEN_HEIGHT;
    static SDL_Window *window;
    static SDL_Renderer *renderer;

    public:

    //Screen constructor
    Screen(std::string title, int width, int height) : SCREEN_WIDTH(width), SCREEN_HEIGHT(height)
    {
        window = nullptr;
        renderer = nullptr;
    }

    //Screen destructor
    ~Screen(){
        std::cout << "Destroying Screen" << std::endl;
        std::cout << "Destroying Renderer" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        std::cout << "Quitting SDL" << std::endl;
        SDL_Quit();
    }
    //Initialize the screen
    SDL_AppResult InitializeScreen();
};