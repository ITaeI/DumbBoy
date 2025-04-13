#pragma once
#include "common.h"
#include <array>


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

        // Channel 1 Registers
        struct
        {
            union 
            {
                u8 Raw;

                struct
                {
                    u8 SweepStep : 3;
                    u8 SweepDirection : 1;
                    u8 SweepPace : 3;
                    u8 Unused : 1;
                };
                
            }NR10;

            union 
            {
                u8 Raw;
                struct
                {
                   u8 InitialTimerLength : 6;
                   u8 Duty : 2;

                };

            }NR11;

            // Volume and Envelope Pace
            union
            {
                u8 Raw;
                struct
                {
                    u8 SweepPace : 3;
                    u8 EnvDirection : 1;
                    u8 InitialVolume : 4;

                };

            }NR12;

            // Channel 1 Lo period/frequency
            u8 NR13;

            union
            {
                u8 Raw;
                struct{
                    u8 UpperPeriod : 3;
                    u8 Unused : 3;
                    u8 LengthEnable : 1;
                    u8 Trigger : 1;
                };

            }NR14;

            
        }Channel1;

        // Channel 2 Registers
        struct
        {
            union 
            {
                u8 Raw;
                struct
                {
                   u8 InitialTimerLength : 6;
                   u8 Duty : 2;

                };

            }NR21;

            union
            {
                u8 Raw;
                struct
                {
                    u8 SweepPace : 3;
                    u8 EnvDirection : 1;
                    u8 InitialVolume : 4;

                };

            }NR22;

            // Channel 2 lo period/frequency
            u8 NR23;

            union
            {
                u8 Raw;
                struct{
                    u8 UpperPeriod : 3;
                    u8 Unused : 3;
                    u8 LengthEnable : 1;
                    u8 Trigger : 1;
                };

            }NR24;
            
        }Channel2;

        // Channel 3 Registers
        struct
        {
            union 
            {
                u8 Raw;

                struct
                {
                    u8 Unused : 7;
                    u8 DACOnOff : 1;
                };
                
            }NR30;

            // Inital Length Timer
            u8 NR31;

            union
            {
                u8 Raw;
                struct
                {
                    u8 Unused5Bits : 5;
                    u8 OutputLevel : 2;
                    u8 Unused1Bit : 1;

                };

            }NR32;

            // Channel 3 lo period/frequency
            u8 NR33;

            union
            {
                u8 Raw;
                struct{
                    u8 UpperPeriod : 3;
                    u8 Unused : 3;
                    u8 LengthEnable : 1;
                    u8 Trigger : 1;
                };

            }NR34;
            
        }Channel3;


        struct
        {

            // Inital Length Timer
            u8 NR41;

            union
            {
                u8 Raw;
                struct
                {
                    u8 SweepPace : 3;
                    u8 EnvDirection : 1;
                    u8 InitialVolume : 4;

                };

            }NR42;

            // Channel 3 lo period/frequency

            union apu
            {
                u8 Raw;
                struct 
                {
                    u8 ClockDivider : 3;
                    u8 LFSRWidth : 1;
                    // Note if value is zero use 0.5 instead
                    u8 ClockShift : 4;

                };
                
            }NR43;
            
            

            union
            {
                u8 Raw;
                struct{

                    u8 Unused : 6;
                    u8 LengthEnable : 1;
                    u8 Trigger : 1;
                };

            }NR44;

            u8 LFSR;
            
        }Channel4;

        struct{


            union{

                u8 Raw;

                struct{
                    u8 Ch1On :1;
                    u8 Ch2On :1;
                    u8 Ch3On :1;
                    u8 Ch4On :1;
                    u8 Unused: 3;
                    u8 OnOff : 1;
                };

            }AudioMasterControlNR52;


            union{

                u8 Raw;

                struct{
                    u8 Ch1Right :1;
                    u8 Ch2Right :1;
                    u8 Ch3Right :1;
                    u8 Ch4Right :1;
                    u8 Ch1Left :1;
                    u8 Ch2Left :1;
                    u8 Ch3Left :1;
                    u8 Ch4Left :1;
                };

            }SoundPanningNR51;

            union{

                u8 Raw;

                struct{
                    u8 RightVol :3;
                    u8 VinRight :1;
                    u8 LeftVol :3;
                    u8 VinLeft :1;
                };

            }NR50;

        }GlobalRegs;
        

        u8 DIVAPU;

        // On or Off
        std::array<bool,4> DACState;
        std::array<bool,4> ChannelState;

        // Each Channel has a length counter
        // Which is clocked at 256hz
        std::array<u8,4> LengthCounter;

        u8 waveFormGenerator(u8 Phase,u8 duty);
        
        void Ch1Sweep();

        void FrameSequencer();
        bool SweepClock = false;
        bool LengthCTRClock = false;
        bool VolEnvelopeClock = false;



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