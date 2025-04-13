#pragma once
#include "common.h"


namespace GBEmu
{
    class Emulator;
    class DMA {

        
        private : 
        Emulator *Emu;
        public:

        // DMA status
        bool OamDMA_InProgress = 0;
        u8 startAdress;
        u8 current_index; //0-159 There are 40 OBJs in total with 4 bytes each

        void connectDMA(Emulator *emu);
        void init();
        void tick();
        void StartOAMTransfer(u8 StartAddress);

        // CGB Only

        void VRAMDmaWrite(u16 address, u8 data);
        u8 VRAMDmaRead(u16 address);

        // Transfer Modes
        void GeneralPurposeDMA();
        void HBlankDMA();
        bool HBlankDMA_Enabled;

        // 1-2 Source
        union 
        {
            u16 VRAMSourceAddress;
            struct{
            u8 HDMA1;
            u8 HDMA2;
            };
        };

        // 3-4 Destination
        union 
        {
            u16 VRAMDestinationAddress;
            struct{
            u8 HDMA3;
            u8 HDMA4;
            };
        };

        // Transfer mode and Length
        union 
        {
            u8 HDMA5;
            struct 
            {
                // Transfer Mode is divided by 0x10 and subtracted by 1, reverse for actual length
                u8 TransferLength : 7;
                u8 TransferMode : 1;
            };
            
        };

    };
}