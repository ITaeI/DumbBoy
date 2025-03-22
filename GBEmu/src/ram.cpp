#include "ram.h"
#include "Emulator.h"

namespace GBEmu
{

    void RAM::connectRAM(Emulator* emu)
    {
        Emu = emu;
    }

    void RAM::init()
    {
        wram[0x2000] = {0x00};
        hram[0x80] = {0x00};
    }

    u8 RAM::read_wram(u16 adress)
    {
        return wram[adress - 0xC000];
    }

    void RAM::write_wram(u16 adress, u8 data)
    {
        wram[adress - 0xC000] = data;
        
    }

    u8 RAM::read_hram(u16 adress)
    {
        return hram[adress - 0xFF80];
    }

    void RAM::write_hram(u16 adress, u8 data)
    {
        hram[adress-0xFF80] = data;
    }
}

