#include "timer.h"

namespace GBEmu
{
    void EmuTimer::timer_init()
    {
        timerRegs.DIV.write(0xAB);
        timerRegs.TIMA.write(0x00);
        timerRegs.TMA.write(0x00);
        timerRegs.TAC.write(0xF8);
    }

    void EmuTimer::timer_tick()
    {
        if(timerRegs.TAC.read() & 0b11)
        {
            switch (timerRegs.TAC.read() & 0b11)
            {
            case 0b00:
                /* code */
                break;
            case 0b01:
                /* code */
                break;
            case 0b10:
                /* code */
                break;
            case 0b11:
                /* code */
                break;
            
            default:
                break;
            }
        }
    }

    u8 EmuTimer::timer_read(u16 address)
    {

    }

    void EmuTimer::timer_write(u16 address, u8 data)
    {

    }
}