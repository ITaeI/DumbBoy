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
#include "joypad.h"
#include "apu.h"
#include <thread>

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
        Joypad joypad;
        APU apu;


        // Emulator Constructor
        Emulator();
        // Emulator Deconstructor
        ~Emulator();

        // Emulator Context
        bool paused;
        bool running = false;
        bool exit = false;
        bool cpu_reset = false;
        u64 ticks = 0;

        // Debug Bool
        bool debug = false;
        // Stepping Bool
        bool step_request = false;
        bool step = false;


        int run();
        int runCPU();
        void stopCPU();
        void InitializeEmu();
        std::thread cpu_thread;
        void ClockCycle(int cpu_cycles);
    };
}