#pragma once
#include "common.h"

namespace GBEmu
{
    class Emulator;
    class Joypad
    {
        private:
        Emulator *Emu;
        public:
        void connectJoypad(Emulator *emu);

        void init();
        // 7: Start
        // 6: Select
        // 5: B
        // 4: A
        // 3: Down
        // 2: Up
        // 1: Left
        // 0: Right
        u8 JoypadInputs;

        // Selction Bools Which determine which nibble the cpu is looking at
        bool selectButtons;
        bool selectDpad;

        // updates Joypad bits depending on key pressed
        void PressKey(u8 bit);
        // updates Joypad bits depending on key released
        void ReleaseKey(u8 bit);

        u8 read();
        void write(u8 data);

    };
} 