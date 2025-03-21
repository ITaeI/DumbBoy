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

        struct{
            Register8Bit P1; // joypad 0xFF00
            Register8Bit DIV; // divider register 0xFF04
            Register8Bit TIMA; // timer counter 0xFF05
            Register8Bit TMA; // timer modulo 0xFF06
            Register8Bit TAC; // timer control 0xFF07
            // IF already implemented in CPU
        }TempRegs;

        u8 ly = 0x0; // do not know what this does

        void connectIO(Emulator* emu);
    };
}