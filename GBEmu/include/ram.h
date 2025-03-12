#pragma once
#include "common.h"

namespace GBEmu
{
    class Emulator;
    class RAM
    {
        private:

        u8 wram[0x2000] = {0x00};
        u8 hram[0x80] = {0x00};

        Emulator *Emu;
        public:

        // Connects the RAM to the Emulator
        void RAM::connectRAM(Emulator* emu);

        // Getters and setters for ram
        void write_wram(u16 adress, u8 data);
        u8 read_wram(u16 adress);

        void write_hram(u16 adress, u8 data);
        u8 read_hram(u16 adress);

    };

}