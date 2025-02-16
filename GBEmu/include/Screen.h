#pragma once 
#include "common.h"

namespace GBEmu
{
    class Emulator; // forward declaration

    class Screen
    {

        private:
        //Screen dimensions
        std::string title;
        int SCREEN_WIDTH;
        int SCREEN_HEIGHT;
        static SDL_Window *window;
        static SDL_Renderer *renderer;
        static SDL_Texture *texture;
        static SDL_Surface *surface;

        Emulator *Emu;
        public:

        //Screen constructor
        Screen()
        {
            title = "Empty";
            SCREEN_WIDTH = 0;
            SCREEN_HEIGHT = 0;
            window = nullptr;
            renderer = nullptr;
            texture = nullptr;
            surface = nullptr;
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

        void connectScreen(Emulator* emu);


        //Initialize the screen
        SDL_AppResult InitializeScreen(std::string Name, int Width, int Height);
        void pollForEvents();
    };
}
