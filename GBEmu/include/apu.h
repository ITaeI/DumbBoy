#pragma once
#include "common.h"
#include <array>


namespace GBEmu
{
    class Emulator;
    class APU;
    class Channel
    {
        public: 
        // Pointer To Central Audio Processor Unit
        APU *apu;

        // Common Channel Values
        u8 Volume;
        u8 Phase;
        u8 DutyPhaseTimer;
        u16 RemainingLength;
        bool LengthPrevEnabled = false;
        u8 EnvelopeTimer;
        bool EnvelopeEnabled;
        // This is also used as the frequency shadow reg in channel 1
        // Derived from registers NRx3 and NRx4
        u16 Frequency;
        // Derived From channels Frequency (ch1-ch2)
        // In channel 4 derived from shift and clock divider (Reg NR43)
        u16 FrequencyTimer;

        Channel(APU * apu) : apu(apu) {}

        virtual void tick() = 0;
        virtual void trigger() = 0;
        virtual void init() = 0;
    };

    class Channel1 : public Channel
    {
        public:
        Channel1(APU* apu) : Channel(apu) {}
        // Frequency Sweep Variables

        u8 SweepTimer;
        bool SweepEnabled;

        void FrequencySweep();
        u16 FrequencyCalculation();

        void trigger() override;
        void tick() override;
        void init() override;

        // Channel 1 Registers
        union 
        {
            u8 Raw;

            struct
            {
                u8 SweepStep : 3;
                // 0 = Addition, 1 = Subtraction
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
               // Write only
               u8 InitialTimerLength : 6;
               // R/W
               u8 Duty : 2;

            };

        }NR11;

        // Volume and Envelope Pace
        union
        {
            u8 Raw;
            struct
            {
                // How Many 64hz Clocks it takes to Increase/Decreas Volume
                u8 EnvPace : 3;
                // 0: Decrease
                // 1: Increase
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

    };

    class Channel2 : public Channel
    {
        public:
        Channel2(APU* apu) : Channel(apu) {}

        void trigger() override;
        void tick() override;
        void init() override;

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
                // How Many 64hz Clocks it takes to Increase/Decreas Volume
                u8 EnvPace : 3;
                // 0: Decrease
                // 1: Increase
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

    };

    class Channel3 : public Channel
    {
        public:

        Channel3(APU* apu) : Channel(apu) {}

        void trigger() override;
        void tick() override;
        void init() override;

        u8 SampleBuffer;
        u8 WaveIndex;

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
    };

    class Channel4 : public Channel
    {
        public:
        Channel4(APU* apu) : Channel(apu) {}

        void trigger() override;
        void tick() override;
        void init() override;

        u16 LFSR;
        u8 DivisorCodes[8] = {8,16,32,48,64,80,96,112};

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

        union
        {
            u8 Raw;
            struct 
            {
                // Note if value is zero use 0.5 instead
                u8 ClockDivider : 3;
                // 0 = 15 bit, 1 = 7 bit
                u8 LFSRWidth : 1;
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
    };


    class APU
    {
        private:
        Emulator *Emu;
        u8 InitialWaveRam[16] = {0x84, 0x40, 0x43, 0xAA, 0x2D, 0x78, 0x92, 0x3C, 0x60, 0x59, 0x59, 0xB0, 0x34, 0xB8, 0x2E, 0xDA};
        public:
        u8 waveRam[16]; // 16 bytes
        APU() : Ch1(this), Ch2(this), Ch3(this), Ch4(this){}

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

        // Determines 1 or Zero Depending on phase an duty cycle
        u8 waveFormGenerator(u8 Phase,u8 duty);
    
        // In charge of Clocking certain Channel Components
        void FrameSequencer();
        // FrameSequences
        u8 LengthSequence = 0b01010101;
        u8 EnvelopeSequence = 0b1000000;
        u8 FrequencySweepSequence = 0b01000100;

        bool CheckNextFrame(u8 sequence);

        // Channels

        Channel1 Ch1;
        Channel2 Ch2;
        Channel3 Ch3;
        Channel4 Ch4;

        std::vector<float> samples;


    };
}