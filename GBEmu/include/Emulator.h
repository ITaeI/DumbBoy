#pragma once
#include "common.h"
#include "Screen.h"
#include "cart.h"
#include "cpu.h"
#include "bus.h"

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

        // Emulator Constructor
        Emulator();

        // Emulator Context
        bool paused;
        bool running;
        u64 ticks;


        int run();
    };
}