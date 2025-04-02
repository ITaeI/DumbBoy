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
        memset(ScreenBuffer, 0, 0x5A00 * sizeof(u8)); // used only for debugging atm
        memset(oam.raw, 0, 160 * sizeof(u8));
        memset(ScanlineObjects, 0, 10 * sizeof(u8));
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
                    ScanOAM();
                    dots -= 80;
                    Mode = 3;
                    lcdRegs.STAT.write((lcdRegs.STAT.read() & 0xFC) | Mode);
                }
                break;
            case 3: // During Mode 3, by default the PPU outputs one pixel to the screen per dot, 
                    //from left to right; the screen is 160 pixels wide, 
                    //so the minimum Mode 3 length is 160 + 12 = 172 dots.
                if (dots >= 12)
                {
                    PushPixelToLCD(dots - 12);
                }
                if(dots >= 172)
                {
                    dots -= 172;
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

    void PPU::ScanOAM()
    {
        u8 ObjCount = 0;
        u8 ObjHeight = lcdRegs.LCDC.readBit(2) ? 16 : 8;
        // OAM has 40 entries 
        for(int i = 0; i < 40; i++)
        {
            // Check to see if an obj or sprite is on the current scanline
            if (oam.o[i].X != 0 && lcdRegs.LY.read() + 16 >= oam.o->Y && lcdRegs.LY.read() + 16 >= oam.o->Y + ObjHeight)
            {
                ScanlineObjects[ObjCount] = i;
            }

            // We log the first 10 then return
            if(ObjCount == 10)
            {
                return;
            }

        }
    }

    void PPU::fetchBGPixel(u8 currentX)
    {
        u16 tileMap = 0x9800; // Default Tile Map
        u16 tileData = 0x9000; // Default Tile Data

        bool CurrentTileIsWindow = false;

        // Check to see if we are on a window or background tile
        if (lcdRegs.LCDC.readBit(5)) // First Check if window is enabled
        {
            // Now check if we are within the window bounds
            if ((lcdRegs.LY.read()  >=  lcdRegs.WY.read() && lcdRegs.LY.read()  <=  lcdRegs.WY.read() + 31) && (currentX >= lcdRegs.WX.read()-7 && currentX <= lcdRegs.WX.read() + 24))
            {
                CurrentTileIsWindow = true;

                // Now Check to see if we need to alter the tile map pointer
                if(lcdRegs.LCDC.readBit(6))
                {
                    tileMap = 0x9C00;
                }
            }

        }
        
        // Check to see if Background is using the second tile map
        if (!CurrentTileIsWindow && lcdRegs.LCDC.readBit(3))
        {
            tileMap = 0x9C00;
        }

        // Now Check which Tile Data/Adressing style we are using
        if (lcdRegs.LCDC.readBit(4))
        {
            tileData = 0x8000;
        }

        // Lets Grab Our Scroll and Window Values

        u8 scrollX = lcdRegs.SCX.read();
        u8 scrollY = lcdRegs.SCY.read();
        u8 windowX = lcdRegs.WX.read()-7;
        u8 windowY = lcdRegs.WY.read();

        // intialize our x and y positions
        u8 yPos = 0;
        u8 xPos = currentX;

        // Depending on the tile type The position is altered
        if (CurrentTileIsWindow)
        {
            xPos -= windowX;
            yPos = (lcdRegs.LY.read() - windowY);
        }
        else
        {
            xPos += scrollX;
            yPos = (scrollY + lcdRegs.LY.read() );
        }

        // Now depending on Adressing mode we use signed or unsigned tile number
        Sint16 tile_index = 0;

        if(tileData == 0x8000)
        {
            tile_index =  (u8)VRAM[tileMap + ((yPos)/8 *32) + (xPos)/8 - 0x8000];
        }
        else
        {
            tile_index =  (Sint8)VRAM[tileMap + ((yPos)/8 *32) + (xPos)/8 - 0x8000];
        }

        // We now need to grab the hi and lo byte to mix them to get the color
        // of the 8 pixels we are grabbing
        u8 lo = VRAM[tileData+ (tile_index*16) +((yPos)%8) *2 - 0x8000];
        u8 hi = VRAM[tileData + (tile_index*16) +((yPos)%8) *2 + 1 - 0x8000];

        // we only need one pixel at the moment which depends on the current X position
        u8 lo_bit = (lo >> (7-(xPos)%8)) & 1;
        u8 hi_bit = (hi >> (7-(xPos)%8)) & 1;

        u8 color_byte = (hi_bit << 1) | lo_bit;

        // We now move the final color byte to the BG FIFO

        ScreenBuffer[currentX + lcdRegs.LY.read() * 160 -1 ] = color_byte;
    }

    void PPU::fetchSpritePixel(u8 currentX)
    {
        
    }

    void PPU::PushPixelToLCD(u8 currentX)
    {
        fetchBGPixel(currentX);
        fetchSpritePixel(currentX);
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
                lcdRegs.STAT.write((data & 0xFC) | (lcdRegs.STAT.read() & 0x03)); // Bits 0-2 are read only
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