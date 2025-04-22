#include "joypad.h"
#include "Emulator.h"

namespace GBEmu
{
    void Joypad::init()
    {
        JoypadInputs = 0xFF;
        selectDpad = false;
        selectButtons = false;
    }

    void Joypad::connectJoypad(Emulator *emu)
    {
        Emu = emu;
    }

    void Joypad::PressKey(u8 bit)
    {
        // clear the bit (kinda backwords 0 = pushed 1 = released)
        JoypadInputs &= ~(1 << bit);

        // depending on which bit is flipped, and the bool the cpu is looking for 
        // we request an interrupt to register IF
        if (bit <= 3 && selectDpad)
        {
            Emu->processor.IF.setBit(Joypad_Int, true);
        }
        else if (bit >= 4 && selectButtons);
        {
            Emu->processor.IF.setBit(Joypad_Int, true);
        }
        
    }

    void Joypad::ReleaseKey(u8 bit)
    {
        JoypadInputs |= (1<<bit);
    }

    u8 Joypad::read()
    {
        // returns a different nibble depending on Bool
        if (selectDpad)
        {
            return (0b0001 << 4) | JoypadInputs & 0x0F;
        }
        else if(selectButtons)
        {
            return (0b0010 << 4) | JoypadInputs  >> 4;
        }
        else
        {
            return 0xCF;
        }
        
    }
    void Joypad::write(u8 data)
    {
        if((data >> 4 & 0x1) == 0x00) // Check if dpad was selected
        {
            selectDpad = true;
            selectButtons = false;
            
        }
        else if((data >> 5 & 0x1) == 0x00) // Check if buttons were selected
        {
            selectButtons = true;
            selectDpad = false;
        }
    }
}