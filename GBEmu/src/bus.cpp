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
            //Rom Data
            return Emu->cartridge.read(addr); 
        }
        else if (addr < 0xA000 )
        {
            //Char Map Data
            //std::cout << "Char Map Data" << std::endl;
            //NO_IMPL
            return 0;
        }
        else if (addr < 0xC000)
        {
            //Cartridge Ram
            return Emu->cartridge.read(addr);
        }
        else if (addr < 0xE000)
        {
           //Working Ram 
           return Emu->systemRam.read_wram(addr);
        }
        else if (addr < 0xFE00)
        {
            // Echo Ram
            //std::cout << "Echo Ram" << std::endl;
            return 0;
            //NO_IMPL
        }
        else if (addr < 0xFEA0)
        {
            //OAM
            //std::cout << "OAM" << std::endl;
            return 0;
            //NO_IMPL
        }
        else if (addr < 0xFF00)
        {
            //reserved unusable
            //std::cout << "Unusable" << std::endl;
            return 0;
            //NO_IMPL
        }
        else if (addr < 0xFF80)
        {
            // IO Registers
            //std::cout << "IO Registers" << " ";
            return Emu->io.read(addr);
        }
        else if (addr == 0xFFFF)
        {
            //CPU Enable Register
            return Emu->processor.IE.read();
        }
        else
        {
            return Emu->systemRam.read_hram(addr);
        }
        return 0;
    }

    void bus::write(u16 addr, u8 data)
    {
        if (addr < 0x8000)
        {
            Emu->cartridge.write(addr, data);
        }
        else if (addr < 0xA000 )
        {
            //Char Map Data
            //std::cout << "Char Map Data" << std::endl;
            //NO_IMPL
        }
        else if (addr < 0xC000)
        {
            //Cartridge Ram
            Emu->cartridge.write(addr, data);
        }
        else if (addr < 0xE000)
        {
           //Working Ram
           if(addr == 0xD81B)
           {
            std::cout << "Bug" << std::endl;
           }
           Emu->systemRam.write_wram(addr,data);
        }
        else if (addr < 0xFE00)
        {
            // Echo Ram
            //std::cout << "Echo Ram" << std::endl;
            //NO_IMPL
        }
        else if (addr < 0xFEA0)
        {
            //OAM
            //std::cout << "OAM" << std::endl;
            //NO_IMPL
        }
        else if (addr < 0xFF00)
        {
            //reserved unusable
            //std::cout << "Unusable" << std::endl;
            //NO_IMPL
        }
        else if (addr < 0xFF80)
        {
            // IO Registers
            //std::cout << "IO Registers" << " ";
            Emu->io.write(addr,data);
        }
        else if (addr == 0xFFFF)
        {
            //CPU Enable Register
            Emu->processor.IE.write(data);
        }
        else if (addr >= 0xFF80)
        {
            // High Ram (between 0xFFFF and 0xFF80)
            Emu->systemRam.write_hram(addr,data);
        }
        
    }
}