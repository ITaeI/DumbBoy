#pragma once
#include "common.h"
#include "registers.h"
#include <array>


namespace GBEmu
{
    class Emulator;
    class PPU
    {
        private:
        Emulator *Emu;


        public:
        void connectPPU(Emulator *emu);
        void init();

        // VRAM Memory and Control Registers
        
        union OAM
        {
            u8 raw[160];
            struct OAM_ITEM
            {
                u8 Y; //Object’s vertical position on the screen + 16
                u8 X; //Object's horizontal position + 8 (hidden x<0 || x>168)
                u8 tile; // Tile location 
    
                u8 Priority : 1; // This gets passed to FIFO entry
                u8 YFlip : 1;
                u8 XFlip : 1;
                u8 Palette : 1;
                u8 Bank_No : 1;
                u8 CGB_Palette : 3;
    
    
            }o[40];

        }oam;
        
        u8 VRAM[0x2000];
        

        struct
        {
            // LCD Control
                // register format
                // 7: LCD & PPU enable: 0 = Off; 1 = On
                    // determines whether the ppu and lcd is on or off
                    // off grants full access to vram and OAM
                    // When ppu is turned back on ppu is activated immediately but screen willl be blank for first frame
                // 6: Window tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
                    // switches between tile maps
                // 5: Window enable: 0 = Off; 1 = On
                    // switches if window is shown or not
                // 4: BG & Window tile data area: 0 = 8800–97FF; 1 = 8000–8FFF
                    // Chooses which adressing mode is being used,  OBJ always 0x8000 mode
                // 3: BG tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
                    // switches between tile maps
                // 2: OBJ size: 0 = 8×8; 1 = 8×16
                    // obj size be carefull when swapping mid frame
                // 1: OBJ enable: 0 = Off; 1 = On
                    // determines whether objects are displayed or not
                // 0: BG & Window enable / priority [Different meaning in CGB Mode]: 0 = Off; 1 = On
                    // DMG: if cleared BG and Window are left blank, obj is untouched
                    // CGB: if cleared BG and Window lose priority
            //ppu never locks this and thus can be altered mid scanline!! Very useful
            Register8Bit LCDC;
            // LCD Status, Format:
                // 6: LYC int select (Read/Write): If set, selects the LYC == LY condition for the STAT interrupt.
                // 5: Mode 2 int select (Read/Write): If set, selects the Mode 2 condition for the STAT interrupt.
                // 4: Mode 1 int select (Read/Write): If set, selects the Mode 1 condition for the STAT interrupt.
                // 3: Mode 0 int select (Read/Write): If set, selects the Mode 0 condition for the STAT interrupt.
                // 2: LYC == LY (Read-only): Set when LY contains the same value as LYC; it is constantly updated.
                // 1-0: PPU mode (Read-only): Indicates the PPU’s current status. Reports 0 instead when the PPU is disabled.
            Register8Bit STAT;
            // LCD Position and Scrolling
                // SCY, SCX: Beckground Viewport y and x position
                // the values represent the top left coordinate of the 160x144 viewport
                // 0-255 values taken

                // ppu calculates the bottom right coordinates using
                // bottom = (SCY + 143) % 256
                // right = (SCX+ 159) % 256
                // if the value is larger than 255 they will wrap around 
            Register8Bit SCY; 
            Register8Bit SCX;
            // LCD Y Coordinate [Read Only]
                // LY indicates the current horizontal line, which might be about to be drawn, 
                // being drawn, or just been drawn. LY can hold any value from 0 to 153, 
                // with values from 144 to 153 indicating the VBlank period.
            Register8Bit LY; 
            Register8Bit LYC; 
            Register8Bit DMA; 
            Register8Bit BGP; 
            Register8Bit OBP0; 
            Register8Bit OBP1; 
            Register8Bit WY;
            Register8Bit WX;

        }lcdRegs;

        u8 lcd_read(u16 adress);
        void lcd_write(u16 adress, u8 data);

        // What is a "dot":
        // A “dot” = one 222 Hz (≅ 4.194 MHz) time unit. 
        // Dots remain the same regardless of whether the CPU is in Double Speed mode, 
        // so there are 4 dots per Single Speed M-cycle, and 2 per Double Speed M-cycle.
        int dots;

        // The PPU draws pixels directly to the screen
        // 154 scanlines per frame
        //during the first 144 the screen is drawn left to right top to bottom
        
        // During a frame the ppu has 4 cycles that it goes through
        // 0: Horizontal Blank (87-204 Dots) - waiting until the end of the next scanline
        // 1: Vertical Blank (4560 Dots) - Waiting for next frame
        // 2: OAM scan (80 Dots) - searching for objects on scan line
        // 3: Drawning Pixels (172-289 dots) - sending pixels to lcd
        u8 Mode;

        // When the PPU is accessing VRAM that memory is inaccessable to the cpu
        // writes are ignored and reads return 0xFF

        void tick();
        void compare_LY_LYC();

        // OAM Search - Essentially checks the current scanline for up to 10 Sprites
        void ScanOAM();
        u8 ScanlineObjects[10];


        // FIFO pixel Fetcher Functions/variables
        void fetchBGPixel(u8 currentX);
        void fetchSpritePixel(u8 currentX);
        void PushPixelToLCD(u8 currentX);
        u8 ScreenBuffer[0x5A00];

        struct FIFO_Entry
        {
            u8 color; // 0-3
            u8 pallete; // OBP1 or OBP2 (DMG) - 0-7 (CGB)
            u8 spritePriority; // only for Sprites on CGB
            u8 BackgroundPriority; // keeps bit 7 of Sprite
        };
        
        // FIFO arrays
        std::array<FIFO_Entry, 8> FIFO_BG;
        std::array<FIFO_Entry, 8> FIFO_SPR;
    };
}