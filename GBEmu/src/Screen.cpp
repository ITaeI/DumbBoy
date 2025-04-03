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

        // Initialize SDL
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        // Create Window
        window = SDL_CreateWindow(Name.c_str(), SCREEN_WIDTH, SCREEN_HEIGHT, SDLwindowFlags );

        // Create Renderer
        renderer = SDL_CreateRenderer(window, NULL);
        SDL_SetRenderVSync(renderer, 1);

        // Create Separate Textures and surfaces for LCD BG and Tile Windows
        tileTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, imageWidths[Tiles], imageHeights[Tiles]);
        tileSurface = SDL_CreateSurface(imageWidths[Tiles], imageHeights[Tiles], SDL_PIXELFORMAT_ABGR8888);

        LCDTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, imageWidths[GBScreen], imageHeights[GBScreen]);
        LCDSurface = SDL_CreateSurface(imageWidths[GBScreen], imageHeights[GBScreen], SDL_PIXELFORMAT_ABGR8888);

        BGTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, imageWidths[Background], imageHeights[Background]);
        BGSurface = SDL_CreateSurface(imageWidths[Background], imageHeights[Background], SDL_PIXELFORMAT_ABGR8888);

        // init imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io{ImGui::GetIO()};

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        io.FontGlobalScale = 2.0f;

        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);

        return SDL_APP_CONTINUE;
    }

    void Screen::Update()
    {

        Uint64 start = SDL_GetTicks();

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Main ImGui Render Windows
        renderMainWindow();
        if(GBWindowReady)
            renderGBScreen();
        if(ViewRegisters)
            renderRegistersWindow();
        if (ViewTiles)
            rendertiles(); 
        if (ViewBackground)
            renderBG();
        if (ViewSettings)
            renderSettings();
        if (ViewSpriteData)
            renderSprites();
 

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 0 , 0 , 0 , 0);
        SDL_RenderClear(renderer);

        ImGui_ImplSDLRenderer3_RenderDrawData (ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);


        pollForEvents();

        Uint64 ElapsedTime = SDL_GetTicks() - start;
        if (ElapsedTime < 1000/TargetFPS)
            SDL_Delay( (1000/TargetFPS) - (SDL_GetTicks() - start) ); 
    }

    void Screen::renderMainWindow()
    {

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        if(ImGui::Begin("DockWindow", nullptr, main_window_flags))
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
                        Emu->cartridge.load(const_cast<char*>("Super Mario Land (JUE) (V1.1) [!].gb"));
                        // run CPU on a separate thread
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
                        Emu->cartridge.load(const_cast<char*>("Tetris (JUE) (V1.1) [!].gb"));
                        // run CPU on a separate thread
                        Emu->cpu_thread = std::thread (&Emulator::runCPU, Emu);

                        // Signal UI to render GB Screen
                        GBWindowReady = true;
                    }
                    else if(ImGui::MenuItem("Run"))
                    {
                        Emu->running = true;
                    }
                    else if(ImGui::MenuItem("Exit"))
                    {
                        Emu->exit = true;
                    }
                    ImGui::EndMenu();
                }
                else if(ImGui::BeginMenu("View"))
                {
                    if(ImGui::MenuItem("Registers"))
                    {
                        ViewRegisters = !ViewRegisters;
                    }
                    else if(ImGui::MenuItem("Tiles"))
                    {
                        ViewTiles = !ViewTiles;
                    }
                    else if(ImGui::MenuItem("Settings"))
                    {
                        ViewSettings = !ViewSettings;
                    }
                    else if(ImGui::MenuItem("Background"))
                    {
                        ViewBackground = !ViewBackground;
                    }
                    else if(ImGui::MenuItem("Sprites"))
                    {
                        ViewSpriteData = !ViewSpriteData;
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

    void Screen::DrawPixel(u8 x, u8 y, u8 color)
    {
        SDL_Rect rect;
        rect.x = x * 4;
        rect.y = y * 4;
        rect.h = 4;
        rect.w = 4;

        SDL_FillSurfaceRect(LCDSurface, &rect, colors[color]);

    }

    void Screen::renderGBScreen()
    {
        for(int y = 0; y < 144; y++)
        {
            for(int x = 0; x< 160; x++)
            {
                DrawPixel(x,y,Emu->ppu.ScreenBuffer[y*160 + x]);
            }
        }

        // Set Render target to texture
        SDL_SetRenderTarget(renderer, LCDTexture);

        //Update texture with surface
        SDL_UpdateTexture(LCDTexture, NULL, LCDSurface->pixels, LCDSurface->pitch);

        SDL_SetRenderTarget(renderer, NULL);

        
        ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);
        if(ImGui::Begin(Emu->cartridge.cart_filename , nullptr, ImGuiWindowFlags_NoCollapse))
        {   
            ImVec2 WindowSize = CalculateImageSize(160,144);

            ImGui::Image((ImTextureID)LCDTexture, WindowSize); 
        }

        ImGui::End();
    }

    void Screen::renderRegistersWindow()
    {
        ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Registers", nullptr, ImGuiWindowFlags_NoCollapse))
        {

            ImGui::Checkbox("Cpu Registers", &showCpuRegs);
            ImGui::SameLine();
            ImGui::Checkbox("LCD Registers", &showLcdRegs);
            ImGui::SameLine();
            ImGui::Checkbox("Timer Registers", &showTimerRegs);

            ImGui::Separator();
            if(showCpuRegs)
            {
                ImGui::Text("Opcode : 0x%02X \t", Emu->processor.opcode);
                ImGui::SameLine();
                ImGui::Text("Instruction : %s", Emu->processor.currentInstruction.mnemonic.c_str());
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
                    ImGui::Text("LCDC");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.LCDC.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("STAT");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.STAT.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("SCY");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.SCY.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("SCX");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.SCX.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("LY");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.LY.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("LYC");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.LYC.read());

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("DMA");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.DMA.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("BGP");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.BGP.read());
                    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("OBP0");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.OBP0.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("OBP1");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.OBP1.read());

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("WY");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.WY.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("WX");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("0x%02X", Emu->ppu.lcdRegs.WX.read());
    
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
                    ImGui::Text("DIV");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("[ 0x%02X ]", Emu->timer.timerRegs.DIV.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("TIMA");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("[ 0x%02X ]", Emu->timer.timerRegs.TIMA.read());
    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("TMA");
    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("[ 0x%02X ]", Emu->timer.timerRegs.TMA.read());
    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("TAC");
    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("[ 0x%02X ]", Emu->timer.timerRegs.TAC.read());

    
                }
                ImGui::EndTable();
            }

            ImGui::Separator();
            if(ImGui::Button("Step"))
            {
                Emu->step = true;
            }
            else
            {
                Emu->step = false;
            }
            ImGui::SameLine();
            ImGui::Checkbox("Debug", &Emu->debug);

        }
        ImGui::End();
    }

    void Screen::rendertiles()
    {
        // there are 384 tiles in total each 16 bytes each

        // Grid Pattern: 16x24
        u8 VramAdress = 0x8000;
        int tile_number = 0;

        // Update the Tile Screen less often
        static auto lastTileUpdate = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTileUpdate).count();

        if (elapsed >= 128)
        {
            for(int y = 0; y < 24; y++)
            {
                for(int x = 0; x < 16; x++)
                {
                    // Draw tile to surface
                    SDL_Rect rect;
                    for(int tileLine = 0; tileLine < 16; tileLine += 2)
                    {
                        u8 lo = Emu->ppu.VRAM[tile_number*16 + tileLine];
                        u8 hi = Emu->ppu.VRAM[tile_number*16 + tileLine + 1];

                        for(int k = 0; k < 8; k++)
                        {
                            u8 lo_bit = (lo >> (7-k)) & 1;
                            u8 hi_bit = (hi >> (7-k)) & 1;
                            u8 color_byte = (hi_bit << 1) | lo_bit;

                            // Write Location of Rectangle
                            rect.x = (x*8 + k)*4;
                            rect.y = (y*8 + tileLine/2)*4;
                            rect.w = 4;
                            rect.h = 4;
                            SDL_FillSurfaceRect(tileSurface, &rect, colors[color_byte]);
                        }
                    }
                    tile_number++;
                } 
            }
            // Set Render target to texture
            SDL_SetRenderTarget(renderer, tileTexture);

            //Update texture with surface

            SDL_UpdateTexture(tileTexture, NULL, tileSurface->pixels, tileSurface->pitch);

            SDL_SetRenderTarget(renderer, NULL);

            lastTileUpdate = currentTime;

        }
        // Draw texture to screen

        ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Tiles", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImVec2 WindowSize = CalculateImageSize(16.0f,24.0f);
            ImGui::Image((ImTextureID)tileTexture, WindowSize); 
        }
        ImGui::End();

    }

    void Screen::renderBG()
    {
        static auto lastTileUpdate = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTileUpdate).count();

        // Used For manual swapping between Tile Maps
        u16 BG_Data_Start = 0x8000;
        if(DataSelect)
        {
            BG_Data_Start = 0x9000;
        }

        u16 BG_Map_Start = 0x9800;
        if(MapSelect)
        {
            BG_Map_Start = 0x9C00;
        }

        // 0 = 9800–9BFF; 1 = 9C00–9FFF MAP
        if(elapsed >= 128)
        {
            // 32 By 32 Tile Map Made Up of Window and Background Tiles
            for(int y = 0; y<32; y++)
            {
                for(int x = 0; x<32; x++)
                {

                    Sint16 tileIndex = 0;
                    if (DataSelect)
                    {

                        tileIndex = (Sint8)Emu->ppu.VRAM[(BG_Map_Start + x + y*32) - 0x8000]; 
                    }
                    else
                    {
                        tileIndex = (u8)Emu->ppu.VRAM[(BG_Map_Start + x + y*32) - 0x8000]; 
                    } 
    
                    SDL_Rect rect;
                    for(int i = 0; i < 8; i++)
                    {
                        u8 lo = Emu->ppu.VRAM[(BG_Data_Start+(tileIndex*16) + i*2) - 0x8000];
                        u8 hi = Emu->ppu.VRAM[(BG_Data_Start+(tileIndex*16) + i*2 +1) - 0x8000];

                        for(int j = 0; j < 8; j++)
                        {

                            u8 lo_bit = (lo >> (7-j)) & 1;
                            u8 hi_bit = (hi >> (7-j)) & 1;
                            u8 color_byte = (hi_bit << 1) | lo_bit;

                            rect.x = (x*8 + j)*4;
                            rect.y = (y*8 + i)*4;
                            rect.w = 4;
                            rect.h = 4;
                            SDL_FillSurfaceRect(BGSurface, &rect, colors[color_byte]);

                        }
                    }
                } 
            }
            // Set Render target to texture
            SDL_SetRenderTarget(renderer, BGTexture);
            //Update texture with surface
            SDL_UpdateTexture(BGTexture, NULL, BGSurface->pixels, BGSurface->pitch);
            SDL_SetRenderTarget(renderer, NULL);
            lastTileUpdate = currentTime;
        }

        ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Background", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
        {
            ImVec2 WindowSize = CalculateImageSize(32,32);
            ImGui::Image((ImTextureID)BGTexture, WindowSize); 
            if (ImGui::RadioButton("Data 0x8000", DataSelect == 0))
            {
                DataSelect = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Data 0x8800", DataSelect == 1))
            {
                DataSelect = 1;
            }

            if (ImGui::RadioButton("Map 9800", MapSelect == 0))
            {
                MapSelect = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Map 9C00", MapSelect == 1))
            {
                MapSelect = 1;
            }
        }
        ImGui::End();
    }

    ImVec2 Screen::CalculateImageSize(float W, float H)
    {
        ImVec2 avail= ImGui::GetContentRegionAvail();
        // Compute the ratio (width / height) for your image.
        float targetAspect = W / H;

        // Choose size based on available width and height.
        float width = avail.x;
        float height = width / targetAspect;
        if (height > avail.y)
        { 
            height = avail.y;
            width = height * targetAspect;
        }
        return ImVec2(width,height);
    }

    void Screen::renderSettings()
    {
        ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::InputInt("Target FPS", &TargetFPS, 5, 100, ImGuiInputTextFlags_None);
        }
        ImGui::End();
    }

    void Screen::renderSprites()
    {
        ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Sprites", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            if(ImGui::BeginTable("Sprite Objects", 5, ImGuiTableFlags_None))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("X");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Y");
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Tile");
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("XFlip");
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("XFlip");
                for(int i = 0; i < 40; i++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", (int)Emu->ppu.oam.o[i].X);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", (int)Emu->ppu.oam.o[i].Y);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", (int)Emu->ppu.oam.o[i].tile);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", (int)Emu->ppu.oam.o[i].XFlip);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%d", (int)Emu->ppu.oam.o[i].YFlip);
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();

    }

    void Screen::pollForEvents()
    {
        SDL_Event event;
        while (SDL_WaitEventTimeout(&event,1))
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
            }
        }
    }

}