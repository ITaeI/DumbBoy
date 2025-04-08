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

        // BG Texture and surface
        SDL_Texture *BGTexture;
        SDL_Surface *BGSurface;

        float imageWidths[3] = {640, 512, 1024};
        float imageHeights[3] = {576, 768, 1024};

        enum images
        {
            GBScreen = 0,
            Tiles = 1,
            Background = 2,

        }images;

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

            BGTexture = nullptr;
            BGSurface = nullptr;
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

            SDL_DestroyTexture(BGTexture);
            SDL_DestroySurface(BGSurface);
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

        ImGuiWindowFlags textureWindows = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNavFocus;
        // SDL Window Flags
        SDL_WindowFlags SDLwindowFlags = SDL_WINDOW_RESIZABLE;

        /**********************************************************************************************************/


        // ------------------ Main Screen Update Loop -------------------------- //
        void Update();

        //Update Components:
        
        // ImGui Renders:

        // Main Window - Primarily for docking, and Main Menu Bar
        void renderMainWindow();
        ImGuiID dockID;

        // Game Screen - Output of GB Tiles and Sprites
        void renderGBScreen();
        void DrawPixel(u8 x, u8 y, u8 color_byte);
        bool GBWindowReady = false;

        // Settings Screen
        void renderSettings();
        bool ViewSettings  = false;
        int TargetFPS = 60;

        //  
        void renderRegistersWindow();
        bool ViewRegisters = false;
        bool showCpuRegs = false;
        bool showLcdRegs = false;
        bool showTimerRegs = false;

        // Render Tiles
        bool ViewTiles = false;
        void rendertiles();

        // Render Background
        bool ViewBackground = false;
        void renderBG();

        // Either Tile Map at 8800 0r 8000
        bool MapSelect = 0;
        bool DataSelect = 0;

        //Render Sprite Data
        bool ViewSpriteData = false;
        void renderSprites();

        //Rom List Scan
        bool LoadRom = false;
        void RenderRomFolder(); 
        void ScanForRoms(std::string Dir);
        std::vector<std::string> RomList;
        char DirBuffer[256];

        // GB colors

        // Grays
        //const Uint32 colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};
        // Original Green
        const Uint32 colors[4] = { 0xFF9BBC0F, 0xFF8BAC0F, 0xFF306230, 0xFF0F380F };
        // Aspect Ratio Calculation
        ImVec2 CalculateImageSize(float W, float H);
        ImVec2 CalculateScreenLocation();

        //Event Handling
        void pollForEvents();
    };
}
