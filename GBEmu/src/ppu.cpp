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
        memset(VRAM, 0, 0x2000 * sizeof(u8));
        memset(oam.raw, 0, 160 * sizeof(u8));
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
        if(!lcdRegs.LCDC.readBit(7))
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
                        lcdRegs.STAT.write((lcdRegs.STAT.read() & 0xFC) | Mode);
                        Emu->processor.IF.setBit(VBlank,true); // Set Vblank Interrupt flag
                        if(lcdRegs.STAT.readBit(4)) // if Mode 1 is a set as LCD interrupt condition
                        {
                            Emu->processor.IF.setBit(LCD,true);
                        }
                        
                    }
                    else // If Mode 2 is set as the LCD interrupt condition
                    {
                        lcdRegs.STAT.write((lcdRegs.STAT.read() & 0xFC) | Mode);
                        if(lcdRegs.STAT.readBit(5))
                        {
                            Emu->processor.IF.setBit(LCD,true);
                        }
                    }
                }
                break;

            case 1: // This is Mode 1 or the Vertical Blank (10 scanlines) waiting for next frame
                // Each scan line is 456 Dots - Mode 1 or VBLANK occurs for 10 scanlines

                if(dots >= 456)
                {
                    dots -= 456;
                    lcdRegs.LY.Increment();
                    compare_LY_LYC();

                    if(lcdRegs.LY.read() == 154)
                    {
                        Mode = 2;
                        lcdRegs.LY.write(0);
                        lcdRegs.STAT.write((lcdRegs.STAT.read() & 0xFC) | Mode);
                        if(lcdRegs.STAT.readBit(5)) // OAM interrupt
                        {
                            Emu->processor.IF.setBit(LCD,true);
                        }
                    }
                }
                break;
            case 2: // OAM Scan 80 dots - checks to see if there is any object within scan lines

                // Remember to Check LY == WY for whether window should be rendered

                if(dots >= 80)
                {
                    dots -= 80;
                    Mode = 3;
                    lcdRegs.STAT.write((lcdRegs.STAT.read() & 0xFC) | Mode);
                }
                break;
            case 3: // During Mode 3, by default the PPU outputs one pixel to the screen per dot, 
                    //from left to right; the screen is 160 pixels wide, 
                    //so the minimum Mode 3 length is 160 + 12 = 172 dots.
                if (dots > 12)
                {
                    // Draw Pixel

                }
                if(dots >= 172)
                {
                    Mode = 0;
                    lcdRegs.STAT.write((lcdRegs.STAT.read() & 0xFC) | Mode);
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

    void PPU::draw_pixel(u8 currentX)
    {
        // Check to see which tile map we are using (BG or Window)

        // Default Tile Map (0x9800)
        u16 tileMap = 0x9800;

        // These are tile coordinates
        u8 fetcherX = 0;
        u8 fetcherY = 0;

        bool CurrentTileIsWindow = false;

        // Check to see if we are on a window or background tile
        if (lcdRegs.LCDC.readBit(5)) // First Check if window is enabled
        {
            // Now check if we are within the window bounds
            if (lcdRegs.LY.read() >=  lcdRegs.WY.read() && currentX >= lcdRegs.WX.read())
            {
                CurrentTileIsWindow = true;

                // Now Check to see if we need to alter the tile map pointer
                if(lcdRegs.LCDC.readBit(6))
                {
                    tileMap = 0x9C00;
                }
            }

        }

        if (!CurrentTileIsWindow && lcdRegs.LCDC.readBit(3)) // Check to see if Background is using the second tile map
        {
            tileMap = 0x9C00;
        }

        // Now Check which Tile Data we want to read from
        u8 tileData = 0x8000;
        if (lcdRegs.LCDC.readBit(4))
        {
            tileData = 0x9000;
        }

        // Update the fetcher's X and Y according to current tile:

        //  --- https://gbdev.io/pandocs/pixel_fifo.html --- \\

        //If the current tile is a window tile, the X coordinate for the window tile is used, 
        //otherwise the following formula is used to calculate the 
        //X coordinate: ((SCX / 8) + fetcher’s X coordinate) & $1F. Because of this formula, 
        //fetcherX can be between 0 and 31.

        //If the current tile is a window tile, the Y coordinate for the window tile is used, 
        //otherwise the following formula is used to calculate the 
        //Y coordinate: (currentScanline + SCY) & 255. Because of this formula, 
        //fetcherY can be between 0 and 255.

        u8 Xpos = 0;
        u8 Ypos = 0;

        // Get XY Coords for Tile 
        Xpos = currentX + lcdRegs.SCX.read();
        Ypos = lcdRegs.LY.read() + lcdRegs.SCY.read();

        // if current tile is window convert to window coords
        if (CurrentTileIsWindow)
        {
            Xpos -= lcdRegs.WX.read();
            Ypos = lcdRegs.LY.read() - lcdRegs.WY.read();
        }

        // From these X and Y positions we can determine the Row and column of the tile in the tile map
        u8 tileRow = (Ypos /8) *32;
        u8 tileCol = (Xpos /8);

        // Depending on the tileData we use a different addressing mode

        Sint8 tileIndex = Emu->io.read(tileMap + tileRow + tileCol);
        if(tileData == 0x8000)
        {
            tileIndex = static_cast<u8>(tileIndex);
        }

        // Extract data with tile coords

        u8 tileY = (Ypos % 8) * 2; // since there is two bytes per row
        u8 tileX = Xpos % 8;

        // Now time to extract the tile data
        u8 lo = Emu->io.read(tileData+ (tileIndex *16) + tileY);
        u8 hi = Emu->io.read(tileData+ (tileIndex *16) + tileY + 1);




        // if (CurrentTileIsWindow)
        // {
        //     fetcherX = lcdRegs.WX.read();  
        //     fetcherY = lcdRegs.WY.read();
        // }
        // else // Get X Y coords for Background 
        // {
        //     fetcherX = ((lcdRegs.SCX.read() / 8) + (currentX % 8)) & 0x1F;
        //     fetcherY = (lcdRegs.LY.read() + lcdRegs.SCY.read()) & 255;
        // }







        


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
                if(Emu->dma.in_progress)
                {
                    return 0xFF;
                }
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
                lcdRegs.STAT.write((data & 0x78) | (lcdRegs.STAT.read() & 0x03)); // Bits 0-2 are read only
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
                    // still recommended to busy-wait in hram during OAM DMA

                    // During DMA the ppu cannot read OAM as well
                lcdRegs.DMA.write(data); // stores start adress
                Emu->dma.StartTransfer(data); // Starts DMA transfer
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