#pragma once
#include "common.h"


namespace GBEmu
{
    class Emulator;
    class APU
    {
        private:
        Emulator *Emu;
        u8 waveRam[16]; // 16 bytes
        public:

        void connectAPU(Emulator *emu);
        void init();
        u8 read(u16 address);
        void write(u16 address, u8 data);

        struct{

        }audioRegs;

        // How The APU runs:

        // The apu has 4 channels that control different aspects of sound generation
        // - These channels are controlled by the audio registers
        
        // Triggering a channel causes it to turn on

        // Volume and envelope
        // Volume can be controlled by either the master volume or individual channel volume

        // additionally an envelope can be configured for ch1,2,4

        // The parameters that can be controlled are the initial volume, 
        // the envelope’s direction (but not its slope), and its duration. 
        // Internally, all envelopes are ticked at 64 Hz, and every 1–7 of those ticks, 
        // the volume will be increased or decreased.

        // Length Timer

        // All channels can be individually be set to shut themselves down

        // When turned on the timer ticks up at 256hz (tied o DIV-APU)

        // when it reaches 64 for ch1,2,4 and 256 for ch3 it turns off

        // Frequency
        // instead of frequency the apu uses durations/periods to make sound (technically negative periods)


    };
}