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

        // Tile Texture and Surface
        SDL_Texture *tileTexture;
        SDL_Surface *tileSurface;

        // LCD Texture and surface
        SDL_Texture *LCDTexture;
        SDL_Surface *LCDSurface;

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

            // Tile Texture and Surfaces
            tileTexture = nullptr;
            tileSurface = nullptr;

            // LCD Texture and Surfaces
            LCDTexture = nullptr;
            LCDSurface = nullptr;
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


            SDL_DestroyTexture(tileTexture);
            SDL_DestroySurface(tileSurface);

            SDL_DestroyTexture(LCDTexture);
            SDL_DestroySurface(LCDSurface);
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

        // render Settings
        void renderSettings();
        bool ViewSettings  = false;
        int TargetFPS = 60;

        //Debug Window
        void renderDebugWindow();
        bool ViewRegisters = false;
        bool showCpuRegs = false;
        bool showLcdRegs = false;
        bool showTimerRegs = false;

        // Render Tiles
        bool ViewTiles = false;
        void rendertiles();

        // GB colors
        Uint32 colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

        //Event Handling
        void pollForEvents();
    };
}
