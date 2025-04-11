#pragma once
#include "common.h"


namespace GBEmu
{
    class Emulator;
    class APU
    {
        private:
        Emulator *Emu;
        // 16 Bytes Long 2 samples Each (4 bits)
        // reads upper nibble first starting at FF30
        // When Channel 3 is started intially the lower nibble of the first byte is read first
        u8 waveRam[16]; // 16 bytes
        public:

        void connectAPU(Emulator *emu);
        void init();
        void tick();
        u8 read(u16 address);
        void write(u16 address, u8 data);

        // Audio Register NRxy scheme 
        // x: Channel number (1-4, or 5 = Global Register)
        // Note: (index number goes from 0-4 and is offset by -1)
        // y: Register ID within channel
        struct{
            u8 NRx0[5]; // in some channel specific feature if present
            u8 NRx1[5]; // controls the length timer
            u8 NRx2[5]; // controls the volume and the envelope
            u8 NRx3[5]; // controls the period 
            u8 NRx4[5]; // has the channel’s trigger and length timer enable bits, as well as any leftover bits of period
        }audioRegs;

        u8 DIVAPU;

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