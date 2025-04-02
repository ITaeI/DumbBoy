#pragma once
#include "common.h"
#include "registers.h"

namespace GBEmu
{
    class Emulator;
    class EmuTimer
    {
        private:
        Emulator *Emu;

        // TIMA Overflow Logic
        bool TIMAOverflowOccured = false;
        int overflowCounter = 0;

        public:
        struct TimerRegs
        {
            Register8Bit TIMA, TMA, TAC;
            Register16Bit DIV;
        };
        
        TimerRegs timerRegs;

        void connectTimer(Emulator *emu);
    
        void timer_tick();
        void timer_init();
        void timer_write(u16 address, u8 data);
        u8 timer_read(u16 adress);
    };

}
