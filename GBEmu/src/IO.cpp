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
            std::cout << "Joypad Input" << std::endl;
            return 0;
            //NO_IMPL
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
        else if (adress <= 0xFF26 && adress >= 0xFF10 )
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
            //std::cout << "LCD Control, Status, Position, Scrolling, and Palettes" << std::endl;
            switch (adress)
            {
                case 0xFF40:
                    return TempRegs.LCDC.read();
                case 0xFF41:
                    return TempRegs.STAT.read();
                case 0xFF42:
                    return TempRegs.SCY.read();
                case 0xFF43:
                    return TempRegs.SCX.read();
                case 0xFF44:
                    return TempRegs.LY.read();
                case 0xFF45:
                    return TempRegs.LYC.read();
                case 0xFF46:
                    return TempRegs.DMA.read();
                case 0xFF47:
                    return TempRegs.BGP.read();
                case 0xFF48:
                    return TempRegs.OBP0.read();
                case 0xFF49:
                    return TempRegs.OBP1.read();
                case 0xFF4A:
                    return TempRegs.WY.read();
                case 0xFF4B:
                    return TempRegs.WX.read();
                default:
                    std::cout << "Unknown LCD register" << std::endl;
                    return 0;
            }
            //NO_IMPL
        }
        else if (adress == 0xFF4F)
        {
            //std::cout << "Vram Bank Select" << std::endl;
            return 0;
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
            return 0;
            //NO_IMPL
        }
        else if (adress >= 0xFF68 && adress <= 0xFF6B)
        {
            //std::cout << "BJ / OBJ Pallettes" << std::endl;
            return 0;
            //NO_IMPL
        }
        else if (adress == 0xFF70)
        {
            //std::cout << "WRAM Bank Select" << std::endl;
            return 0;
            
        }
        return 0 ;
    }

    void IO::write(u16 adress, u8 data)
    {
        if (adress == 0xFF00)
        {
            //std::cout << "Joypad Input" << std::endl;
            //NO_IMPL
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
        else if (adress <= 0xFF26 && adress >= 0xFF10 )
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
            //std::cout << "LCD Control, Status, Position, Scrolling, and Palettes" << std::endl;
            switch (adress)
            {
                case 0xFF40:
                    std::cout << "LCD Control" << std::endl;
                    TempRegs.LCDC.write(data);
                    break;
                case 0xFF41:
                    std::cout << "LCD Status" << std::endl;
                    TempRegs.STAT.write(data);
                    break;
                case 0xFF42:
                    std::cout << "Scroll Y" << std::endl;
                    TempRegs.SCY.write(data);
                    break;
                case 0xFF43:
                    std::cout << "Scroll X" << std::endl;
                    TempRegs.SCX.write(data);
                    break;
                case 0xFF44:
                    std::cout << "LY" << std::endl;
                    TempRegs.LY.write(data);
                    break;
                case 0xFF45:
                    std::cout << "LY Compare" << std::endl;
                    TempRegs.LYC.write(data);
                    break;
                case 0xFF46:
                    std::cout << "DMA Transfer" << std::endl;
                    TempRegs.DMA.write(data);
                    break;
                case 0xFF47:
                    std::cout << "BG Palette Data" << std::endl;
                    TempRegs.BGP.write(data);
                    break;
                case 0xFF48:
                    std::cout << "OBJ Palette 0 Data" << std::endl;
                    TempRegs.OBP0.write(data);
                    break;
                case 0xFF49:
                    std::cout << "OBJ Palette 1 Data" << std::endl;
                    TempRegs.OBP1.write(data);
                    break;
                case 0xFF4A:
                    std::cout << "Window Y Position" << std::endl;
                    TempRegs.WY.write(data);
                    break;
                case 0xFF4B:
                    std::cout << "Window X Position" << std::endl;
                    TempRegs.WX.write(data);
                    break;
                default:
                    std::cout << "Unknown LCD register" << std::endl;
                    break;
            }
            //NO_IMPL
        }
        else if (adress == 0xFF4F)
        {
            //std::cout << "Vram Bank Select" << std::endl;
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
            //NO_IMPL
        }
        else if (adress >= 0xFF68 && adress <= 0xFF6B)
        {
            //std::cout << "BJ / OBJ Pallettes" << std::endl;
            //NO_IMPL
        }
        else if (adress == 0xFF70)
        {
            //std::cout << "WRAM Bank Select" << std::endl;
            //NO_IMPL
        }
    }
}