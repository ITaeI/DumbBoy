#include "bus.h"
#include "Emulator.h"


namespace GBEmu
{

    // Gives us access to the Emulator
    void bus::connectBus(Emulator* emu)
    {
        Emu = emu;
    }


    u8 bus::read(u16 addr)
    {
        if (addr < 0x8000)
        {
            return Emu->cartridge.read(addr);
        }
        return 0x00;
    }

    u8 bus::write(u16 addr, u8 data)
    {
        return Emu->cartridge.write(addr, data);
        
    }
}