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
            Register8Bit LCDC; // LCD Control 0xFF40
            Register8Bit STAT; // LCDC status 0xFF41
            Register8Bit SCY; // Scroll Y 0xFF42
            Register8Bit SCX; // Scroll X 0xFF43
            Register8Bit LY; // LCD Y Coord 0xFF44
            Register8Bit LYC; // LCD Y Coord 0xFF45
            Register8Bit DMA; // DMA Transfer 0xFF46
            Register8Bit BGP; // BG Palette Data 0xFF47
            Register8Bit OBP0; // object palette 0 0xFF48
            Register8Bit OBP1; // object palette1= 1 0xFF49
            Register8Bit WY;
            Register8Bit WX;
        }TempRegs;

        u8 ly = 0x0; // do not know what this does

        void connectIO(Emulator* emu);
    };
}