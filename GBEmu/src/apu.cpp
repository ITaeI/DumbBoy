#include "apu.h"


namespace GBEmu
{

    u8 APU::read(u16 address)
    {
        switch (address)
        {
        case 0xFF10:
            break;
        case 0xFF11:
            break;
        case 0xFF12:
            break;
        case 0xFF13:
            break;
        case 0xFF14:
            break;
        case 0xFF1A:
            break;
        case 0xFF1B:
            break;
        case 0xFF1C:
            break;
        case 0xFF1D:
            break;
        case 0xFF1E:
            break;
        case 0xFF20:
            break;
        case 0xFF21:
            break;
        case 0xFF22:
            break;
        case 0xFF23:
            break;
        case 0xFF24:
            break;
        case 0xFF25:
            break;
        case 0xFF26:
            // Audio Master Control - NR52
        default:
            return waveRam[address - 0xFF30];
            break;
        }
    }
    void APU::write(u16 address, u8 data)
    {
        switch (address)
        {
        case 0xFF10:
            break;
        case 0xFF11:
            break;
        case 0xFF12:
            break;
        case 0xFF13:
            break;
        case 0xFF14:
            break;
        case 0xFF1A:
            break;
        case 0xFF1B:
            break;
        case 0xFF1C:
            break;
        case 0xFF1D:
            break;
        case 0xFF1E:
            break;
        case 0xFF20:
            break;
        case 0xFF21:
            break;
        case 0xFF22:
            break;
        case 0xFF23:
            break;
        case 0xFF24:
            break;
        case 0xFF25:
            break;
        case 0xFF26:
            // Audio Master Control - NR52
        default:
            waveRam[address - 0xFF30] = data;
            break;
        }
    }
}