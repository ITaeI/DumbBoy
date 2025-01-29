#include "Emulator.h"
#include "Screen.h"
/*
    Emulator Components:

    cpu - The Gameboy CPU
    cart - The Gameboy Cartridge
    adressBus - The Gameboy Address Bus
    PPU - The Gameboy Pixel Processing Unit
    Timer - The Gameboy Timer

*/ 

namespace GBEmu
{
    Emulator::Emulator()
    {
        processor.connectCPU(this);
        systemBus.connectBus(this);
        cartridge.connectCartridge(this);
    }


    int Emulator::run()
    {
        // Load Rom Cartridge
        cartridge.cart_load("Super Mario Land (JUE) (V1.1) [!].gb");

        // Initialize the Screen
        Screen screen("GBEmu", 1280, 720);
        screen.InitializeScreen();

        // Main Emulator Loop
        while (running)
        {
            // poll for events
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    running = false;
                }
            }

        }
        return 0;
    }
}
