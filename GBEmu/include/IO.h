#pragma once
#include "common.h"
#include "registers.h"
namespace GBEmu
{
    class Emulator;
    class IO
    {
        private:
        Emulator* Emu;

        public:
        u8 read(u16 adress);
        void write(u16 adress, u8 data);

        u8 serial_data[2]; // serial data registers for now


        void connectIO(Emulator* emu);
    };
}