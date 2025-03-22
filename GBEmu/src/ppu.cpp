#include "ppu.h"
#include "Emulator.h"


namespace GBEmu
{
    void PPU::connectPPU(Emulator *emu)
    {
        Emu = emu;
    }
    void PPU::init()
    {
        // Reset For new use
        VRAM[0x2000] = {0};
        oam.raw[160] = {0};
        Mode = 2;
        dots = 0;

        lcdRegs.LCDC.write(0x91);
        lcdRegs.STAT.write(0x85);
        lcdRegs.SCY.write(0x00);
        lcdRegs.SCX.write(0x00);
        lcdRegs.LYC.write(0x00);
        lcdRegs.LY.write(0x00);
        lcdRegs.DMA.write(0xFF);
        lcdRegs.BGP.write(0xFC);
        lcdRegs.WX.write(0x00);
        lcdRegs.WY.write(0x00);
    }

    void PPU::tick()
    {
        // A single frame is 70224 Dots
        dots += 1; // Dots occur once every 4 M cycles  

        // Check if PPU is enabled
        if(lcdRegs.LCDC.readBit(7))
        {
            if (dots >= 70224)
            {
                dots-= 70224;
            }
            return;
        }

        switch (Mode)
        {
            case 0:
                // Mode 0 : This is the horizontal blank and will wait here until next scanline

                if(dots >= 204)
                {
                    dots -= 204;
                    Mode = 2;

                    lcdRegs.LY.Increment();
                    compare_LY_LYC();

                    if (lcdRegs.LY.read() == 144) // if VBlank mode is next
                    {
                        Mode = 1;
                        lcdRegs.STAT.write(lcdRegs.STAT.read() | Mode);
                        Emu->processor.IF.setBit(VBlank,true); // Set Vblank Interrupt flag
                        if(lcdRegs.STAT.readBit(4)) // if Mode 1 is a set as LCD interrupt condition
                        {
                            Emu->processor.IF.setBit(LCD,true);
                        }
                        
                    }
                    else // If Mode 2 is set as the LCD interrupt condition
                    {
                        lcdRegs.STAT.write(lcdRegs.STAT.read() | Mode);
                        if(lcdRegs.STAT.readBit(5))
                        {
                            Emu->processor.IF.setBit(LCD,true);
                        }
                    }
                }
                break;

            case 1: // This is Mode 1 or the Vertical Blank (10 scanlines) waiting for next frame
                // Each scan line is 456 Dots - Mode 1 or VBLANK occurs for 10 scanlines

                // Update STAT register Mode
                lcdRegs.STAT.write(lcdRegs.STAT.read() | Mode);

                if(dots >= 456)
                {
                    dots -= 456;
                    lcdRegs.LY.Increment();
                    compare_LY_LYC();

                    if(lcdRegs.LY.read() == 154)
                    {
                        Mode = 2;
                        lcdRegs.LY.write(0);
                        lcdRegs.STAT.write(lcdRegs.STAT.read() | Mode);
                        if(lcdRegs.STAT.readBit(5)) // OAM interrupt
                        {
                            Emu->processor.IF.setBit(LCD,true);
                        }
                    }
                }
                break;
            case 2: // OAM Scan 80 dots - checks to see if there is any object within scan lines

                if(dots >= 80)
                {
                    dots -= 80;
                    Mode = 3;
                    lcdRegs.STAT.write(lcdRegs.STAT.read() | Mode);
                }
                break;
            case 3: // During Mode 3, by default the PPU outputs one pixel to the screen per dot, 
                    //from left to right; the screen is 160 pixels wide, 
                    //so the minimum Mode 3 length is 160 + 121 = 172 dots.
                if(dots >= 172)
                {
                    Mode = 0;
                    lcdRegs.STAT.write(lcdRegs.STAT.read() | Mode);
                    if(lcdRegs.STAT.readBit(3))
                    {
                        Emu->processor.IF.setBit(LCD,true);
                    }
                }
                break;
            
            default:
                break;
        }
    }

    void PPU::compare_LY_LYC()
    {
        bool LY_LYC_compare = lcdRegs.LY.read() == lcdRegs.LYC.read();
        lcdRegs.STAT.setBit(2,LY_LYC_compare);
        if (LY_LYC_compare)
        {
            Emu->processor.IF.setBit(LCD,true);
        } 
    } 

    u8 PPU::lcd_read(u16 adress)
    {
        switch (adress)
        {
            case 0xFF40:
                return lcdRegs.LCDC.read();
            case 0xFF41:
                return lcdRegs.STAT.read();
            case 0xFF42:

                return lcdRegs.SCY.read();
            case 0xFF43:
                return lcdRegs.SCX.read();
            case 0xFF44:

                return lcdRegs.LY.read();
            case 0xFF45:
                // LCY: LY compare:
                // The Game Boy constantly compares the value of the LYC and LY registers. 
                // When both values are identical, the “LYC=LY” flag in the STAT register is set, 
                // and (if enabled) a STAT interrupt is requested.

                return lcdRegs.LYC.read();
            case 0xFF46:
                return lcdRegs.DMA.read();
            case 0xFF47:
                // Background palette
                //DMG:
                // assigned colors to the color IDs
                // 7-6 ID3, 5-4 ID2, 3-2 ID1, 1-0 ID0
                //CGB: uses CGB palette memory instead
                return lcdRegs.BGP.read();
            case 0xFF48:
                // Same as BGP however last two bits are ignored as ID0 is transparent for objs
                return lcdRegs.OBP0.read();
            case 0xFF49:
                return lcdRegs.OBP1.read();
            case 0xFF4A:
                // WY, WX are window position on the screen
                // they are the top left pixel of the window
                // range WX 0..166(0,166 dont use because hardware bugs) WY 0..143
                // placing at WX 7 and WY 0 will be at top left of screen

                // Mid Frame Behavior
                // The scroll registers are re-read on each tile fetch, 
                // except for the low 3 bits of SCX, which are only read at the beginning of the 
                // scanline (for the initial shifting of pixels).
                return lcdRegs.WY.read();
            case 0xFF4B:
                return lcdRegs.WX.read();
            default:
                std::cout << "Unknown LCD register" << std::endl;
                return 0;
        }
    }

    void PPU::lcd_write(u16 adress, u8 data)
    {
        switch (adress)
        {
            case 0xFF40:
                std::cout << "LCD Control" << std::endl;
                lcdRegs.LCDC.write(data);
                break;
            case 0xFF41:
                std::cout << "LCD Status" << std::endl;
                lcdRegs.STAT.write(data);
                break;
            case 0xFF42:
                std::cout << "Scroll Y" << std::endl;
                lcdRegs.SCY.write(data);
                break;
            case 0xFF43:
                std::cout << "Scroll X" << std::endl;
                lcdRegs.SCX.write(data);
                break;
            case 0xFF44:
                std::cout << "LY" << std::endl;

                lcdRegs.LY.write(data);
                break;
            case 0xFF45:
                std::cout << "LY Compare" << std::endl;
                lcdRegs.LYC.write(data);
                break;
            case 0xFF46:
                std::cout << "DMA Transfer" << std::endl;

                // DMA
                    // The value written to this register will kickoff direct memory access
                    // the value given will read from 0x(value)00 - 0x(value)9F
                    // to OAM 0xFE00 - 0xFE9F
                    // Speculation is that we will use the same bus and this will only happen once?

                    // DMG: During this procedure the cpu can only access hram
                    // CGB: The Cartridge and Wram are on separate buses
                    // still recommended to budy-wait in hram during OAM DMA

                    // During DMA the ppu cannot read OAM as well
                lcdRegs.DMA.write(data);
                break;
            case 0xFF47:
                std::cout << "BG Palette Data" << std::endl;
                lcdRegs.BGP.write(data);
                break;
            case 0xFF48:
                std::cout << "OBJ Palette 0 Data" << std::endl;
                lcdRegs.OBP0.write(data);
                break;
            case 0xFF49:
                std::cout << "OBJ Palette 1 Data" << std::endl;
                lcdRegs.OBP1.write(data);
                break;
            case 0xFF4A:
                std::cout << "Window Y Position" << std::endl;
                lcdRegs.WY.write(data);
                break;
            case 0xFF4B:
                std::cout << "Window X Position" << std::endl;
                lcdRegs.WX.write(data);
                break;
            default:
                std::cout << "Unknown LCD register" << std::endl;
                break;
        }
    }
}