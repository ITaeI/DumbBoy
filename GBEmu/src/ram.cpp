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
        memset(wram, 0, sizeof(wram));
        memset(wramBanks,0,sizeof(wramBanks));
        memset(hram, 0, sizeof(hram));
        CurrentWramBank = 1;
    }

    u8 RAM::read_wram(u16 adress)
    {
        if(adress-0xC000 < 0x1000)
        {
            return wram[adress - 0xC000];
        }
        else
        {
            return wramBanks[adress - 0xD000 + (CurrentWramBank * 0x1000)];
        }
    }

    void RAM::write_wram(u16 adress, u8 data)
    {
        if(adress-0xC000 < 0x1000)
        {
            wram[adress - 0xC000] = data;
        }
        else
        {
           wramBanks[adress - 0xD000 + (CurrentWramBank * 0x1000)] = data;
        }
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

