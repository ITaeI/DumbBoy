#include "Emulator.h"
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
        ppu.connectPPU(this);
        dma.connectDMA(this);

    }

    Emulator::~Emulator()
    {
        stopCPU();
    }

    int Emulator::runCPU()
    {

        while (!exit && !cpu_reset)
        {
            if (running)
            {
                if (debug)
                {
                    if (Prev_step == false && step == true)
                    {
                        processor.step();
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                    Prev_step = step;
                }
                else
                {
                    processor.step();
                }

            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        return 0;
    }

    void Emulator::stopCPU()
    {
        running = false;
        cpu_reset = true;
        if(cpu_thread.joinable())
        {
            cpu_thread.join();
        }
        cpu_reset = false;
    }

    void Emulator::InitializeEmu()
    {
        //Intialize the processor(cpu)
        processor.init();

        // Reset Cartridge
        cartridge.freeRomData();

        // Reset DMA
        dma.init();

        // Reset RAM
        systemRam.init();

        // Reset Timer
        timer.timer_init();

        // Reset PPU
        ppu.init();
    }

    int Emulator::run()
    {
        InitializeEmu();
        screen.InitializeScreen("DumbBoy", 1280,800);

        while(!exit)
        {
            screen.Update();
        }

        return 0;
    }

    void Emulator::ClockCycle(int M_Cycles)
    {
        // Note The gameboy has two cycle lengths M_Cycles are 4 T_Cycles
        // The PPU and timer tick every Dot or T Cycle
        // https://gekkio.fi/files/gb-docs/gbctr.pdf CH5

        for(int i = M_Cycles; i > 0; i--)  
        {
            for(int j = 0; j < 4; j++)
            {
                ticks++;
                timer.timer_tick();
                ppu.tick();
            }
            dma.tick();
        }
    }
}
