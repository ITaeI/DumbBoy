#include "IO.h"
#include "Emulator.h"

namespace GBEmu
{

    void IO::connectIO(Emulator* emu)
    {
        Emu = emu;
    }

    u8 IO::read(u16 adress)
    {
        if (adress == 0xFF00)
        {
            return Emu->joypad.read();
        }
        else if (adress <= 0xFF02)
        {
            if (adress == 0xFF01)
            {
                return serial_data[0];
            }
            else if (adress == 0xFF02)
            {
                return serial_data[1];
            }
            std::cout << "Serial Transfer" << std::endl;
            return 0;
        }
        else if (adress <= 0xFF07)
        {
            return Emu->timer.timer_read(adress);
            //NO_IMPL
        }
        else if (adress == 0xFF0F)
        {
            return Emu->processor.IF.read();
        }
        else if (adress >= 0xFF10 && adress <= 0xFF26 )
        {
            //std::cout << "Audio" << std::endl;
            return 0;
            //NO_IMPL
        }
        else if (adress >= 0xFF30 && adress <= 0xFF3F )
        {
            //std::cout << "Wave Pattern" << std::endl;
            return 0;
            //NO_IMPL
        }
        else if (adress >= 0xFF40 && adress <= 0xFF4B)
        {
            return Emu->ppu.lcd_read(adress);
        }
        else if (adress == 0xFF4D)
        {
            return (u8)Emu->DoubleSpeed << 7 | (u8)Emu->processor.SwitchArmed;
        }
        else if (adress == 0xFF4F)
        {
            //std::cout << "Vram Bank Select" << std::endl;
            return Emu->ppu.CurrentVRAMBank | ~(0x1);
            //NO_IMPL
        }
        else if (adress == 0xFF50)
        {
            //std::cout << "BootRom Enable disable" << std::endl;
            return 0;
            //NO_IMPL
        }
        else if (adress >= 0xFF51 && adress <= 0xFF55)
        {
            //std::cout << "VRAM DMA" << std::endl;
            
            return Emu->dma.VRAMDmaRead(adress);
            //NO_IMPL
        }
        else if (adress == 0xFF56)
        {
            // infared
            return 0 ;
        }
        else if (adress >= 0xFF68 && adress <= 0xFF6B)
        {

            // Color Palettes for the CGB (Small amount of ram on the gameboy color)
            // Uses this instead of BGP
            // FF68 : Background color palette specification/background palette index
            // format : 7-Auto increment 5-0 address

            // FF69 : Background color palette data/backgroud palette data
            // Each color is stored as little-endian RGB555
            // Format : 0-4 Red, 5-9 Green, 10-14 Blue
            // cannot be read or written to when ppu is reading from it

            // FF6A-FF6B : Same as previous registers, but for objects
            return 0;
            //NO_IMPL
        }
        else if (adress == 0xFF6C)
        {
            // Obj Priority mode
            // WIP
        }
        else if (adress == 0xFF70)
        {
            //std::cout << "WRAM Bank Select" << std::endl;
            return Emu->systemRam.CurrentWramBank;
            
        }

        return 0;
    }

    void IO::write(u16 adress, u8 data)
    {
        if (adress == 0xFF00)
        {
            Emu->joypad.write(data);
        }
        else if (adress <= 0xFF02)
        {

            if (adress == 0xFF01)
            {   
                //std::cout << "We have written to the serial" << std::endl;
                serial_data[0] = data;
                return;
            }
            else if (adress == 0xFF02)
            {
                //std::cout << "We have written to the serial" << std::endl;
                serial_data[1] = data;
                return;

            }
            std::cout << "Serial Transfer" << std::endl;
        }
        else if (adress <= 0xFF07)
        {
            Emu->timer.timer_write(adress, data);
        }
        else if (adress == 0xFF0F)
        {
            //std::cout << "Interrupt Flag written" << std::endl;
            Emu->processor.IF.write(data);
        }
        else if (adress >= 0xFF26 && adress <= 0xFF10 )
        {
            //std::cout << "Audio" << std::endl;
            //NO_IMPL
        }
        else if (adress >= 0xFF30 && adress <= 0xFF3F )
        {
            //std::cout << "Wave Pattern" << std::endl;
            //NO_IMPL
        }
        else if (adress >= 0xFF40 && adress <= 0xFF4B)
        {
            Emu->ppu.lcd_write(adress,data);
            //NO_IMPL
        }
        else if (adress == 0xFF4D)
        {
            Emu->processor.SwitchArmed = data & 0x1;
        }
        else if (adress == 0xFF4F)
        {
            //std::cout << "Vram Bank Select" << std::endl;
            Emu->ppu.CurrentVRAMBank = data & 0x1;
            //NO_IMPL
        }
        else if (adress == 0xFF50)
        {
            //std::cout << "BootRom Enable disable" << std::endl;
            //NO_IMPL
        }
        else if (adress >= 0xFF51 && adress <= 0xFF55)
        {
            //std::cout << "VRAM DMA" << std::endl;

            //These two registers specify the address at which the transfer will read data from. 
            //Normally, this should be either in ROM, SRAM or WRAM, thus either in range 
            //0000-7FF0 or A000-DFF0. [Note: this has yet to be tested on 
            //Echo RAM, OAM, FEXX, IO and HRAM]. Trying to specify a source address in VRAM 
            //will cause garbage to be copied.
            
            //The four lower bits of this address will be ignored and treated as 0
            Emu->dma.VRAMDmaWrite(adress,data);
            //NO_IMPL
        }
        else if (adress == 0xFF56)
        {
            // Infared
        }
        else if (adress >= 0xFF68 && adress <= 0xFF6B)
        {
            //std::cout << "BJ / OBJ Pallettes" << std::endl;
            //NO_IMPL
        }
        else if (adress == 0xFF6C)
        {
            // Obj Priority mode
            // WIP
        }
        else if (adress == 0xFF70)
        {
            //std::cout << "WRAM Bank Select" << std::endl;
            Emu->systemRam.CurrentWramBank = data;
            //NO_IMPL
        }
    }
}