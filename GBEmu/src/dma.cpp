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
        OamDMA_InProgress = 0;
        HBlankDMA_Enabled = 0;

        VRAMSourceAddress = 0x00;
        VRAMDestinationAddress = 0x00;
    }

    void DMA::tick()
    {
        if(!OamDMA_InProgress)
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
            OamDMA_InProgress = false;
        }
    }

    void DMA::StartOAMTransfer(u8 StartAddress)
    {
        OamDMA_InProgress = 1;
        startAdress = StartAddress;
        current_index = 0x00;
    }

    void DMA::GeneralPurposeDMA()
    {
        // For VRAM DMA General Purpose DMA
        u8 AmountOfBytesToTransfer = (TransferLength + 1) * 0x10;
        for(int i = 0; i < AmountOfBytesToTransfer; i++)
        {
            Emu->ppu.VRAM[(VRAMDestinationAddress + i - 0x8000) + (Emu->ppu.CurrentVRAMBank * 0x2000)] = Emu->systemBus.read(VRAMSourceAddress + i);
        }
        HDMA5 = 0xFF;
    }

    void DMA::HBlankDMA()
    {
        // Transfers 10 Bytes per HBlank (Block)
        // Halts when Halt Command is called

        u8 CurrentBlock = (TransferLength + 1) * 0x10;
        for(int i = 0; i<10; i++)
        {
            Emu->ppu.VRAM[(VRAMDestinationAddress + i + CurrentBlock - 0x8000) + (Emu->ppu.CurrentVRAMBank * 0x2000)] = Emu->systemBus.read(VRAMSourceAddress + i);
        }

        TransferLength = (CurrentBlock + 0x10) / 10 - 1;

        if (HDMA5 = 0xFF)
        {
            HBlankDMA_Enabled = false;
        }
    }


    u8 DMA::VRAMDmaRead(u16 address)
    {
        return HDMA5;
    }


    void DMA::VRAMDmaWrite(u16 address, u8 data)
    {
        // Note for Source and Destination Adress the 4 lowest bits are treated as 0
        switch (address)
        {
        case 0xFF51:
            HDMA1 = data;
            break;
        case 0xFF52:
            HDMA2 = data;
            break;
        case 0xFF53:
            HDMA3 = data;
            break;
        case 0xFF54:
            HDMA4 = data;
            break;
        case 0xFF55:
            HDMA5 = data;
            if(HBlankDMA_Enabled)
            {
                if(!TransferMode)
                {
                    HBlankDMA_Enabled = false;
                }
            }
            else
            {
                if (!TransferMode)
                {
                    GeneralPurposeDMA();
                }
                else
                {
                    HBlankDMA_Enabled = true;
                }

            }
            break;
        
        default:
            break;
        }
    }
}