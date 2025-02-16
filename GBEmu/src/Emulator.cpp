#include "Emulator.h"
#include <thread>
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
        systemRam.connectRAM(this);
        screen.connectScreen(this);
    }

    int Emulator::runCPU()
    {
        //Intialize the processor(cpu)
        processor.init();

        while (running || !exit)
        {

            if(!processor.step())
            {

            }

        }
        return 0;
    }

    int Emulator::run()
    {
        // Load Rom Cartridge
        cartridge.cart_load("Super Mario Land (JUE) (V1.1) [!].gb");

        // Initialize the Screen
        screen.InitializeScreen(cartridge.header->title, 1280, 720);

        // Detach CPU into separate thread
        std::thread t(&Emulator::runCPU, this);
        t.detach();

        std::cout << "CPU Detached" << std:: endl;

        while(!exit)
        {
            screen.pollForEvents();
        }
        return 0;
    }
}
