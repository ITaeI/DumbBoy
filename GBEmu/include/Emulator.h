#pragma once
#include "common.h"
#include "Screen.h"
#include "cart.h"
#include "cpu.h"
#include "bus.h"
#include "ram.h"

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


        // Emulator Constructor
        Emulator();

        // Emulator Context
        bool paused;
        bool running = true;
        bool exit = false;
        u64 ticks;


        int run();
        int runCPU();
    };
}