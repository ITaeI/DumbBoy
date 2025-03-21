#pragma once 
#include "common.h"

namespace GBEmu
{
    class Emulator; // forward declaration .. fun

    class Screen
    {

        private:
        //Screen dimensions
        std::string title;
        int SCREEN_WIDTH;
        int SCREEN_HEIGHT;
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *texture;

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
        }

        //Screen destructor
        ~Screen(){
            //Stop ImGui
            ImGui_ImplSDLRenderer3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();

            //stop SDL3
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            std::cout << "Quitting SDL" << std::endl;
            SDL_Quit();
        }

        void connectScreen(Emulator* emu);


        //Initialize the screen
        SDL_AppResult InitializeScreen(std::string Name, int Width, int Height);
        void Update();
        void pollForEvents();
    };
}
