#include "common.h"
#include "Screen.h"
#include "Emulator.h"
#include <thread>

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

        // Create Window
        window = SDL_CreateWindow(Name.c_str(), SCREEN_WIDTH, SCREEN_HEIGHT, SDLwindowFlags );

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

        // Main ImGui Render Windows
        renderMainWindow();
        if(GBWindowReady)
            renderGBScreen();
        if(DebugWindowReady)
            renderDebugWindow();


        SDL_SetRenderDrawColor(renderer, 0 , 0 , 0 , 0);
        SDL_RenderClear(renderer);

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData (ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);


        pollForEvents();

    }

    void Screen::renderMainWindow()
    {

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        if(ImGui::Begin("Basic Window", nullptr, main_window_flags))
        {
            if(ImGui::BeginMenuBar())
            {
                if(ImGui::BeginMenu("File"))
                {
                    if(ImGui::MenuItem("Reset Rom"))
                    {
                        // Stop Previous cpu_thread
                        Emu->stopCPU();
                        // Reinitialize Emulator
                        Emu->InitializeEmu();
                        
                        GBWindowReady = false;
                        // ToDo: change how to get roms into program
                        Emu->cartridge.load("Super Mario Land (JUE) (V1.1) [!].gb");
                        // run CPU on a separate thread
                        Emu->running = true;
                        Emu->cpu_thread = std::thread (&Emulator::runCPU, Emu);

                        // Signal UI to render GB Screen
                        GBWindowReady = true;

                    }
                    else if(ImGui::MenuItem("Load Rom"))
                    {
                        // Stop Previous cpu_thread
                        Emu->stopCPU();
                        // Reinitialize Emulator
                        Emu->InitializeEmu();
                        
                        GBWindowReady = false;
                        // ToDo: change how to get roms into program
                        Emu->cartridge.load("02-interrupts.gb");
                        // run CPU on a separate thread
                        Emu->running = true;
                        Emu->cpu_thread = std::thread (&Emulator::runCPU, Emu);

                        // Signal UI to render GB Screen
                        GBWindowReady = true;
                    }
                    else if(ImGui::MenuItem("Exit"))
                    {
                        Emu->exit = true;
                    }
                    ImGui::EndMenu();
                }
                else if(ImGui::BeginMenu("Debug"))
                {
                    if(ImGui::MenuItem("Debug Window"))
                    {
                        DebugWindowReady = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            
        }

        
        dockID = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockID, ImVec2(0.0f,0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();


    }

    void Screen::renderGBScreen()
    {
        ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);
        ImGui::Begin(Emu->cartridge.cart_filename , nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::End();
    }

    void Screen::renderDebugWindow()
    {
        ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_NoCollapse))
        {

            ImGui::Checkbox("Cpu Registers", &showCpuRegs);
            ImGui::SameLine();
            ImGui::Checkbox("LCD Registers", &showLcdRegs);
            ImGui::SameLine();
            ImGui::Checkbox("Timer Registers", &showTimerRegs);

            ImGui::Separator();
            if(showCpuRegs)
            {
                if(ImGui::BeginTable("CPU Registers", 4, ImGuiTableFlags_None))
                {
                    ImGui::TableSetupColumn("CPU Registers", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Value 1", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Regs 2", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Value 2", ImGuiTableColumnFlags_NoResize);
    
                    ImGui::TableHeadersRow();
                    ImGui::Separator();
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("AF");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%04X", Emu->processor.reg.af.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("BC");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%04X", Emu->processor.reg.bc.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("DE");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%04X", Emu->processor.reg.de.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("HL");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%04X", Emu->processor.reg.hl.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("SP");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%04X", Emu->processor.reg.sp.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("PC");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%04X", Emu->processor.reg.pc.read());
    
    
                }
                ImGui::EndTable();
            }

            if(showLcdRegs)
            {
                if(ImGui::BeginTable("LCD Registers", 4, ImGuiTableFlags_None))
                {
                    ImGui::TableSetupColumn("LCD Registers", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Value 1", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Regs 2", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Value 2", ImGuiTableColumnFlags_NoResize);
    
                    ImGui::TableHeadersRow();
    
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("AF");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->processor.reg.af.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("BC");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->processor.reg.bc.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("DE");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->processor.reg.de.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("HL");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->processor.reg.hl.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("SP");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->processor.reg.sp.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("PC");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->processor.reg.pc.read());
    
    
                }
                ImGui::EndTable();
            }

            if(showTimerRegs)
            {
                if(ImGui::BeginTable("Timer Registers", 4, ImGuiTableFlags_None))
                {
                    ImGui::TableSetupColumn("Timer Registers", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Value 1", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Regs 2", ImGuiTableColumnFlags_NoResize);
                    ImGui::TableSetupColumn("##Value 2", ImGuiTableColumnFlags_NoResize);
    
                    ImGui::TableHeadersRow();
    
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("AF");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->processor.reg.af.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("BC");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->processor.reg.bc.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("DE");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->processor.reg.de.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("HL");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->processor.reg.hl.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("SP");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->processor.reg.sp.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("PC");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->processor.reg.pc.read());
    
    
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
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