#pragma once
#include "common.h"

namespace GBEmu
{

    class Emulator;

    class bus
    {
        private:
        Emulator* Emu;

        public:


        u8 read(u16 addr);
        void write(u16 addr, u8 data);
        void connectBus(Emulator* emu);

    };
}