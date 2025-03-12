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
        io.connectIO(this);
    }

    int Emulator::runCPU()
    {
        //Intialize the processor(cpu)
        processor.init();

        while (running && !exit)
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
        cartridge.cart_load("01-special.gb");
        //cartridge.cart_load("08-misc instrs.gb");
        //cartridge.cart_load("07-jr,jp,call,ret,rst.gb");
        //cartridge.cart_load("06-ld r,r.gb");
        //cartridge.cart_load("cpu_instrs.gb");

        // Initialize the Screen
        screen.InitializeScreen(cartridge.header->title, 1280, 720);

        // run CPU on a separate thread
        std::thread t(&Emulator::runCPU, this);

        std::cout << "CPU Detached" << std:: endl;

        while(!exit)
        {
            screen.pollForEvents();
        }


        t.join();
        return 0;
    }
}
