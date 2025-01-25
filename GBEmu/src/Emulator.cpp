#include "common.h"
/*
    Emulator Components:

    cpu - The Gameboy CPU
    cart - The Gameboy Cartridge
    adressBus - The Gameboy Address Bus
    PPU - The Gameboy Pixel Processing Unit
    Timer - The Gameboy Timer

*/ 

    Emulator::EmulatorState Emulator::emu_ctx;

int Emulator::run(int argc, char *argv[])
{
    if(argc<2)
    {
        std::cout << "Usage: " << argv[0] << " <rom file>" << std::endl;
        return -1;
    }
    cart cart;
    if (cart.cart_load(argv[1]) != 0)
    {
        std::cout << "Failed to load ROM" << std::endl;
        return -2;
    }

    std::cout << "ROM Loaded" << std::endl;

    // Initialize the SDL, Window, and Renderer
    Screen display("GBEmu", 1280, 720);
    if(display.InitializeScreen() != SDL_APP_CONTINUE)
    {
        std::cout << "Failed to initialize screen" << std::endl;
        return -3;
    }

    // Initialize the CPU
    cpu cpu;

    emu_ctx.running = true;
    emu_ctx.paused = false;
    emu_ctx.ticks = 0;

    while(emu_ctx.running)
    {
        if(emu_ctx.paused)
        {
 
            SDL_Delay(10);
            continue;
        }

        if(!emu_ctx.paused)
        {
            //Run the CPU
            if(!cpu.step())
            {
                std::cout << "CPU Error" << std::endl;
                return -3;
            }

        }
    }
    return 0;
}

Emulator::EmulatorState Emulator::getState()
{
    return emu_ctx;
}