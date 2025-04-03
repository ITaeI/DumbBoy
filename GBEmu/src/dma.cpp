#include "dma.h"
#include "Emulator.h"

namespace GBEmu
{
    void DMA::connectDMA(Emulator *emu)
    {
        Emu = emu;
    }

    void DMA::init()
    {
        in_progress = 0;
    }

    void DMA::tick()
    {
        if(!in_progress)
        {
            return;
        }



        // Write Object to PPU OAM - in CGB Mode VRAM Can also be written to
        u16 DmaAdress = (startAdress << 8 | current_index);
        Emu->ppu.oam.raw[current_index] = Emu->systemBus.read( (startAdress << 8) | current_index);
        current_index++;

        // DMA takes 160 M cycles to complete
        if(current_index > 0x9F)
        {
            current_index = 0x00;
            in_progress = false;
        }
    }

    void DMA::StartTransfer(u8 StartAddress)
    {
        in_progress = 1;
        startAdress = StartAddress;
        current_index = 0x00;
    }
}