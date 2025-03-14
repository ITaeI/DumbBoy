#pragma once
#include "common.h"
#include "registers.h"

namespace GBEmu
{
    class EmuTimer
    {
        struct TimerRegs
        {
            Register8Bit DIV, TIMA, TMA, TAC;
        };
        
        TimerRegs timerRegs;
    
        void timer_tick();
        void timer_init();
        void timer_write(u16 address, u8 data);
        u8 timer_read(u16 adress);
    };

}
