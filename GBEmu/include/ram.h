#pragma once
#include "common.h"

namespace GBEmu
{
    class Emulator;
    class RAM
    {
        private:

        // Switchable WRAM
        u8 wram[0x1000];
        // DMG Bank 1 only, CGB Banks 1-7
        u8 wramBanks[0x7000];
        u8 hram[0x80];

        Emulator *Emu;
        public:

        u8 CurrentWramBank = 1;

        // Connects the RAM to the Emulator
        void connectRAM(Emulator* emu);
        void init();
        // Getters and setters for ram
        void write_wram(u16 adress, u8 data);
        u8 read_wram(u16 adress);

        void write_hram(u16 adress, u8 data);
        u8 read_hram(u16 adress);

    };

}