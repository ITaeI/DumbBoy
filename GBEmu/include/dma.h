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
        bool in_progress = 0;
        u8 startAdress;
        u8 current_index; //0-159 There are 40 OBJs in total with 4 bytes each

        void connectDMA(Emulator *emu);
        void init();
        void tick();
        void StartTransfer(u8 StartAddress);
    };
}