#pragma once
#include "common.h"
#include "Screen.h"
#include "cart.h"
#include "cpu.h"
#include "bus.h"
#include "ram.h"
#include "IO.h"
#include "timer.h"
#include "ppu.h"
#include "dma.h"

namespace GBEmu
{
    class Emulator
    {
        private:


        public:
        //Emulator Components
        cpu processor;
        cart cartridge;
        bus systemBus;
        RAM systemRam;
        Screen screen;
        IO io;
        EmuTimer timer;
        PPU ppu;
        DMA dma;



        // Emulator Constructor
        Emulator();

        // Emulator Context
        bool paused;
        bool running = true;
        bool exit = false;
        u64 ticks;


        int run();
        int runCPU();
        void ClockCycle(int cpu_cycles);
    };
}