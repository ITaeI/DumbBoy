#include "timer.h"
#include "Emulator.h"

namespace GBEmu
{
    void EmuTimer::connectTimer(Emulator *emu)
    {
        Emu = emu;
    }

    void EmuTimer::timer_init()
    {
        timerRegs.DIV.write(0xABCC);
        timerRegs.TIMA.write(0x00);
        timerRegs.TMA.write(0x00);
        timerRegs.TAC.write(0xF8);
    } 

    void EmuTimer::timer_tick()
    {
        // Check for falling edge
        u16 div_prev = timerRegs.DIV.read();
        timerRegs.DIV.Increment(); 
        bool falling_Edge_Check = false;



        // Check Bit 12 for falling edge (If so increment DIVAPU)
        if((div_prev & (1 << 12)) &&(!(timerRegs.DIV.read() & (1<<12))))
        {
            Emu->apu.FrameSequencer();

        }

        if(TIMAOverflowOccured)
        {
            overflowCounter++;
        }

        if (timerRegs.TAC.read() & 0b100)
        {
            switch (timerRegs.TAC.read() & 0b11)
            {
            case 0b00:
                falling_Edge_Check = (div_prev & (1 << 9)) &&(!(timerRegs.DIV.read() & (1<<9)));
                break;
            case 0b01:
                falling_Edge_Check = (div_prev & (1 << 3)) &&(!(timerRegs.DIV.read() & (1<<3)));
                break;
            case 0b10:
                falling_Edge_Check = (div_prev & (1 << 5)) &&(!(timerRegs.DIV.read() & (1<<5)));
                break;
            case 0b11:
                falling_Edge_Check = (div_prev & (1 << 7)) &&(!(timerRegs.DIV.read() & (1<<7)));
                break;
            
            default:
                break;
            }

    
            if (falling_Edge_Check)
            {

                timerRegs.TIMA.Increment();
                if(timerRegs.TIMA.read() == 0x00)
                {
                    TIMAOverflowOccured = true;
                }

            }

        }

        // For 4 T-Cycles after TIMA has overflowed the value will stay 0x00
        if(TIMAOverflowOccured)
        {
            if(overflowCounter == 4)
            {
                overflowCounter = 0;
                TIMAOverflowOccured = false;
                timerRegs.TIMA.write(timerRegs.TMA.read());
                Emu->processor.IF.setBit(Timer_Int, true); // Request Timer Interrupt
            }
        }
    }

    u8 EmuTimer::timer_read(u16 address)
    {
        switch (address)
        {
        case 0xFF04: // DIV Register
            return timerRegs.DIV.read() >> 8;
            break;
        case 0xFF05: // TIMA Register
            return timerRegs.TIMA.read();
            break;   
        case 0xFF06: // TMA Register
            return timerRegs.TMA.read();
            break;
        case 0xFF07: // TAC Register
            return timerRegs.TAC.read() & 0x07;
            break;     
        default:
            return 0;
            break;
        }
    }

    void EmuTimer::timer_write(u16 address, u8 data)
    {
        switch (address)
        {
        case 0xFF04:
            timerRegs.DIV.write(0);
            break;
        case 0xFF05:
            timerRegs.TIMA.write(data);
            break;
        case 0xFF06:
            timerRegs.TMA.write(data);
            break;
        case 0xFF07:
            timerRegs.TAC.write(timerRegs.TAC.read() & 0xFC | data & 0x07);
            break;
        
        default:
            break;
        }
    }
}