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
            return Emu->ppu.VRAM[addr - 0x8000];
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
            if(Emu->dma.in_progress)
            {
                return 0xFF;
            }
            return Emu->ppu.oam.raw[addr - 0xFE00];
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
            return Emu->io.read(addr);
        }
        else if (addr == 0xFFFF)
        {
            //CPU Interrupt Enable Register
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
            // VRAM
            // - $8000-$97FF
            // - each tile taking 16 bytes

            // adressing modes (determined by LCDC bit 4)
            // 0x8000 - 0x87FF : unsigned byte
            // 0x8800 - 0x97FF : signed 

            // color ID (0 -> 3):
            // first byte == low bit
            // second byte == high bit

            // Tile Maps: 32x32 tiles
            // $9800-$9BFF and $9C00-$9FFF
            // 1 Byte indexes containg the indexes of tiles to be displayed

            // BG Atributes (CGB only)
            // An additional map of 32x32 bytes is stored in vram bank 1
            // and corresponds to the same one in vram bank 0
            // attributes:
            // 7 : Priority
            // 6 : Y Flip
            // 5 : X Flip
            // 4 : empty
            // 3 : Bank no
            // 1-2 : color pallete

            // BG to object priority (CGB):
            // found in BG attributes , LCDC bit 0, OAM attributes bit 7
            // check out table in pandocs

            // BG Background
            // controlled by SCY and SCX
            // out of the 256x256 pixels only 160x144 are shown
            // SCY and SCX specify the origin of the visible 160×144 
            // pixel area within the total 256×256 pixel Background map
            
            // Window
            // WX and WY determine position on the screen
            // screen coordinates of the top left corner of the Window are (WX-7,WY)
            // Whether the Window is displayed can be toggled using LCDC bit 5

            Emu->ppu.VRAM[addr - 0x8000] = data;
        }
        else if (addr < 0xC000)
        {
            //Cartridge Ram
            Emu->cartridge.write(addr, data);
        }
        else if (addr < 0xE000)
        {
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
            // The GB PPU can show up to 40 objects/sprites at a time 8x8 or 8x16
            // only 10 can be printed per scanline
            // same format as BG but can be taken from bloxk 0 and 1 $8000-8FFF, and have unsigned numbering
            
            //$FE00-FE9F
            // Format (4bytes)
            // Byte 0 — Y position

                // Y = Object’s vertical position on the screen + 16. So for example:

            // Byte 1 — X Position
             
                // X = Object's horizontal position + 8 (hidden x<0 || x>168)

            // Byte 2 - Tile index in 8x8 mode

                // in 8x8 mode:
                // specifies object's only tile index 0x00 -> 0xFF
                // This unsigned value selects a tile from the memory area at $8000-$8FFF
                // in CGB mode can come from different banks

                // in 8 by 16 mode:
                // $8000-$8FFF still represents 8x8 tiles but we point at the first of the two tiles

            //Byte 3
                // Attributes:
                // 7: Priority: 0 = No, 1 = BG and Window colors 1–3 are drawn over this OBJ
                // 6: Y flip: 0 = Normal, 1 = Entire OBJ is vertically mirrored
                // 5: X flip: 0 = Normal, 1 = Entire OBJ is horizontally mirrored
                // 4:DMG palette [Non CGB Mode only]: 0 = OBP0, 1 = OBP1
                // 3: Bank [CGB Mode Only]: 0 = Fetch tile from VRAM bank 0, 1 = Fetch tile from VRAM bank 1
                // 2-0: CGB palette [CGB Mode Only]: Which of OBP0–7 to use
            
            // Object priorities and conflicts
                
                //Selection Priority

                    // the PPU compares LY (using LCDC bit 2 to determine their size)
                    // to the objects Y value and selects up to 10 objects to be drawn on the line
                    // the ppu scans OAM memory sequentially (0xFE00 - 0xFE9F) and selects the first 10 possible objects
                    // the ppu only checks Y value


                //Drawing Priority
                    
                    // only occurs for opaque pixels
                    // highest Object priority attribute determines last drawn

                if(Emu->dma.in_progress)
                {
                    return;
                }

                Emu->ppu.oam.raw[addr-0xFE00] = data;

        }
        else if (addr < 0xFF00)
        {
            //reserved unusable
            //std::cout << "Unusable" << std::endl;

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