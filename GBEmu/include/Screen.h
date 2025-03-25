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
        SDL_Surface *surface;

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
            //Stop ImGui
            ImGui_ImplSDLRenderer3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();

            //stop SDL3
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_DestroyTexture(texture);
            SDL_DestroySurface(surface);
            std::cout << "Quitting SDL" << std::endl;
            SDL_Quit();
        }

        void connectScreen(Emulator* emu);


        //Initialize the screen
        SDL_AppResult InitializeScreen(std::string Name, int Width, int Height);


        /*********************************************WINDOW FLAGS*************************************************/

        // Imgui Window Flags
        ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

        ImGuiWindowFlags textureWindows = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
        // SDL Window Flags
        SDL_WindowFlags SDLwindowFlags = SDL_WINDOW_RESIZABLE;

        /**********************************************************************************************************/


        // --- Main Screen Update Loop --- //
        void Update();

        //Update Components:
        
        // ImGui Renders:

        // Main Window - Primarily for docking, and Main Menu Bar
        void renderMainWindow();
        ImGuiID dockID;

        //Game Screen - Output of GB Tiles and Sprites
        void renderGBScreen();
        bool GBWindowReady = false;

        //Debug Window
        void renderDebugWindow();
        bool DebugWindowReady = false;
        bool showCpuRegs = false;
        bool showLcdRegs = false;
        bool showTimerRegs = false;

        // Render Tiles
        void rendertiles();

        // GB colors
        Uint32 colors[4] = {0xFF000000, 0xFF555555, 0xFFAAAAAA, 0xFFFFFFFF};

        //Event Handling
        void pollForEvents();
    };
}
