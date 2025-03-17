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
        timer.connectTimer(this);
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
        //cartridge.cart_load("01-special.gb"); //Passed
        cartridge.cart_load("02-interrupts.gb");
        //cartridge.cart_load("08-misc instrs.gb"); //Passed
        //cartridge.cart_load("07-jr,jp,call,ret,rst.gb"); //Passed
        //cartridge.cart_load("06-ld r,r.gb");//Passed
        //cartridge.cart_load("03-op sp,hl.gb"); // Passed
        //cartridge.cart_load("04-op r,imm.gb"); // Passed
        //cartridge.cart_load("05-op rp.gb"); // Passed
        //cartridge.cart_load("09-op r,r.gb"); // Passed
        //cartridge.cart_load("10-bit ops.gb"); // Passed
        //cartridge.cart_load("11-op a,(hl).gb"); // Passed
        //cartridge.cart_load("cpu_instrs.gb"); //Passed

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

    void Emulator::ClockCycle(int M_Cycles)
    {
        // Note The gameboy has two cycle lengths M_Cycles are 4 T_Cycles
        // https://gekkio.fi/files/gb-docs/gbctr.pdf CH5

        for(int i = M_Cycles; i > 0; i--)  
        {
            for(int j = 0; j < 4; j++)
            {
                ticks++;
                timer.timer_tick();
            }
        }
    }
}
