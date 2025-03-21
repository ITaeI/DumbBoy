#include "common.h"
#include "Screen.h"
#include "Emulator.h"

namespace GBEmu
{

    void Screen::connectScreen(Emulator* emu)
    {
        Emu = emu;
    }


    SDL_AppResult Screen::InitializeScreen(std::string Name, int Width, int Height)
    {
        title = Name; SCREEN_WIDTH = Width; SCREEN_HEIGHT = Height;
        SDL_SetAppMetadata("GBEmu", "1.0", "ITaeI");

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        // define window flags (Window is now resizable)
        SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE;

        // Create Window
        window = SDL_CreateWindow("DumbBoy", SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags );

        // Create Renderer
        renderer = SDL_CreateRenderer(window, NULL);

        // init imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io{ImGui::GetIO()};

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);

        return SDL_APP_CONTINUE;
    }

    void Screen::Update()
    {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        //ImGui::DockSpaceOverViewport(); // This allows Us to dock on the Main SDL Window // Very Cool


        ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin("Basic Window", nullptr, main_window_flags);


        if(ImGui::BeginMenuBar())
        {
            if(ImGui::BeginMenu("Sample"))
            {
                if(ImGui::MenuItem("Sample"))
                {
                    std::cout << "Huzzah" << std::endl;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        ImGui::End();

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 0 , 0 , 0 , 0);
        SDL_RenderClear(renderer);

        ImGui_ImplSDLRenderer3_RenderDrawData (ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);


        pollForEvents();

    }

    void Screen::pollForEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // Process All IMGUI Window events
            ImGui_ImplSDL3_ProcessEvent(&event);

            switch(event.type)
            {
                case SDL_EVENT_QUIT:
                    Emu->exit = true;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch(event.key.scancode)
                    {
                        case SDL_SCANCODE_W:
                            std::cout << "W" << std::endl;
                            break;
                        case SDL_SCANCODE_A:
                            std::cout << "A" << std::endl;
                            break;
                        case SDL_SCANCODE_S:
                            std::cout << "S" << std::endl;
                            break;
                        case SDL_SCANCODE_D:
                            std::cout << "D" << std::endl;
                            break;
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    switch(event.key.scancode)
                    {
                        case SDL_SCANCODE_W:
                            std::cout << "W UP" << std::endl;
                            break;
                        case SDL_SCANCODE_A:
                            std::cout << "A UP" << std::endl;
                            break;
                        case SDL_SCANCODE_S:
                            std::cout << "S UP" << std::endl;
                            break;
                        case SDL_SCANCODE_D:
                            std::cout << "D UP" << std::endl;
                            break;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    switch(event.button.button)
                    {
                        case SDL_BUTTON_LEFT:
                            std::cout << "Left Mouse Down" << std::endl;
                            break;
                        case SDL_BUTTON_RIGHT:
                            std::cout << "Right Mouse Down" << std::endl;
                            break;
                    }
                    break;
                
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    switch(event.button.button)
                    {
                        case SDL_BUTTON_LEFT:
                            std::cout << "Left Mouse UP" << std::endl;
                            break;
                        case SDL_BUTTON_RIGHT:
                            std::cout << "Right Mouse UP" << std::endl;
                            break;
                    }
                    break;

            }
        }
    }

}