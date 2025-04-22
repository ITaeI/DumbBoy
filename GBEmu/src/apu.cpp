#include "apu.h"
#include "Emulator.h"
#include <math.h>

namespace GBEmu
{

    void APU::connectAPU(Emulator *emu)
    {
        Emu = emu;
    }


    void APU::init()
    {
        DACState.fill(0);
        DIVAPU = 0;
        for(int i = 0; i <16; i++)
        {
            waveRam[i] = InitialWaveRam[i];
        }
        Ch1.init();
        Ch2.init();
        Ch3.init();
        Ch4.init();

        GlobalRegs.NR50.Raw = 0x77;
        GlobalRegs.SoundPanningNR51.Raw = 0xF3;
        GlobalRegs.AudioMasterControlNR52.Raw = 0xF1;
    }

    u8 APU::waveFormGenerator(u8 Phase,u8 duty)
    {
        std::array<u8,8> waveform;
        switch(duty)
        {
            case 0b00:
                waveform = {0,0,0,0,0,0,0,1}; // 12.5%
                break;
            case 0b01:
                waveform = {1,0,0,0,0,0,0,1}; // 25%
                break;
            case 0b10:
                waveform = {1,0,0,0,0,1,1,1}; // 50%
                break;
            case 0b11:
                waveform = {0,1,1,1,1,1,1,0}; // 75%
                break;
        }
        return waveform[Phase];
    }

    void APU::FrameSequencer()
    {

        if(!GlobalRegs.AudioMasterControlNR52.OnOff)
        {
            return;
        }

        DIVAPU = (1+DIVAPU) % 8;

        int TempVol;
        if((LengthSequence >> DIVAPU) & 1)
        {
            // Length Count
            if(Ch1.NR14.LengthEnable )
            {
                if(--Ch1.RemainingLength == 0)
                {
                    GlobalRegs.AudioMasterControlNR52.Ch1On = 0;


                }
            }

            if(Ch2.NR24.LengthEnable)
            {
                if(--Ch2.RemainingLength == 0)
                {
                    GlobalRegs.AudioMasterControlNR52.Ch2On = 0;

                }
            } 

            if(Ch3.NR34.LengthEnable)
            {
                if(--Ch3.RemainingLength == 0)
                {
                    GlobalRegs.AudioMasterControlNR52.Ch3On = 0;

                }
            }

            if(Ch4.NR44.LengthEnable)
            {
                if(--Ch4.RemainingLength == 0)
                {
                    GlobalRegs.AudioMasterControlNR52.Ch4On = 0;

                }
            }
        }

        if ((EnvelopeSequence >> DIVAPU) & 1)
        {
            if(GlobalRegs.AudioMasterControlNR52.Ch1On && Ch1.EnvelopeEnabled)
            {   
                TempVol = Ch1.Volume;
                if(--Ch1.EnvelopeTimer == 0)
                {
                    TempVol = Ch1.NR12.EnvDirection ? TempVol + 1 : TempVol - 1;
                    if(TempVol  > 15 || TempVol  < 0)
                    {
                        Ch1.EnvelopeEnabled = false;
                    } 
                    else
                    {
                        Ch1.Volume = TempVol;
                    }

                    Ch1.EnvelopeTimer = Ch1.NR12.EnvPace;
                }
            }
            if(GlobalRegs.AudioMasterControlNR52.Ch2On && Ch2.EnvelopeEnabled)
            {
                TempVol = Ch2.Volume;
                if(--Ch2.EnvelopeTimer == 0)
                {
                    TempVol = Ch2.NR22.EnvDirection ? TempVol + 1 : TempVol - 1;
                    if(TempVol  > 15 || TempVol  < 0)
                    {
                        Ch2.EnvelopeEnabled = false;
                    }
                    else
                    {
                        Ch2.Volume = TempVol;
                    }
                    Ch2.EnvelopeTimer = Ch2.NR22.EnvPace;
                }
            }

            if(GlobalRegs.AudioMasterControlNR52.Ch4On && Ch4.EnvelopeEnabled)
            {
                TempVol = Ch4.Volume;
                if(--Ch4.EnvelopeTimer == 0)
                {
                    TempVol = Ch4.NR42.EnvDirection ? TempVol + 1 : TempVol - 1;
                    if(TempVol  > 15 || TempVol  < 0)
                    {
                        Ch4.EnvelopeEnabled = false;
                    }
                    else
                    {
                        Ch4.Volume = TempVol;
                    }
                    Ch4.EnvelopeTimer = Ch4.NR42.SweepPace;
                }
            }
        }

        if((FrequencySweepSequence >> DIVAPU) & 1)
        {
            if(Ch1.SweepEnabled)
            {
                Ch1.FrequencySweep();
            }
        }

        //DIVAPU = (1+DIVAPU) % 8;

    }

    bool APU::CheckNextFrame(u8 Sequence)
    {
        if((Sequence >> ((DIVAPU + 1)%8)) & 1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    void Channel1::FrequencySweep()
    {
        // Decrement Timer and Check if its 0
        if(SweepTimer > 0)
        {
            SweepTimer--;
        }

        if(SweepTimer == 0)
        {
            if(NR10.SweepPace > 0)
            {
                SweepTimer = NR10.SweepPace;
            }
            else
            {
                SweepTimer = 8;
            }

            if(NR10.SweepPace != 0)
            {
                u16 NewFrequency = FrequencyCalculation();

                if(NewFrequency <= 2047 && NR10.SweepStep !=0)
                {

                    // Set Shadow and Registers to New Frequency
                    Frequency = NewFrequency;
                    NR13 = NewFrequency & 0xFF;
                    NR14.UpperPeriod = (NewFrequency & 0x7) >> 8;


                    FrequencyCalculation();
                }
            }
        }
    }



    u16 Channel1::FrequencyCalculation()
    {
        u16 NewFrequency = Frequency >> NR10.SweepStep;

        if(NR10.SweepDirection)
        {
            NewFrequency = Frequency - NewFrequency;
            negateModeCalculations++;
        }
        else
        {
            NewFrequency = Frequency + NewFrequency;
        }

        if (NewFrequency > 2047)
        {
            SweepEnabled = false;
            apu->GlobalRegs.AudioMasterControlNR52.Ch1On  = 0;
        }

        return NewFrequency;
    }

    void APU::tick()
    {

        Ch1.tick();
        Ch2.tick();
        Ch3.tick();
        Ch4.tick();

        // DAC Receives (Waveform value * Volume) from both Channel 1 and 2
        // DAC Receives Sample from Channel 3 shifted depending on NR32 output level
        // DAC Receives LFSR bit0 * Volume


    }

    u8 APU::read(u16 address)
    {

        switch (address)
        {
        case 0xFF10:
            // This is the start of Channel 1

            // Channel Sweep
            // 6-4 : Pace
            // - Dictates how often sweep iterations happen (in 128hz ticks or 7.8ms)
            // - if 0 is written pace is disbled
            // 3: Direction - 0: period increases, 1: period decreases
            // 2-0: individual step
            // - on each iterationa new new period is created from the older one
            // NewPeriod = OldPeriod (+ or minus -> direction) OldPeriod/(2^step)

            // after each sweep the period in NR13 and NR14 is modified and written back

            // if period overflows (over 0x7FF) the channel turns off
            // This occurs even if sweep iterations are disabled by the pace being 0.

            return  0x80 | Ch1.NR10.Raw;
            break;
        case 0xFF11:

            // Channel Length Timer and Duty Cycle

            // 7-6: Wave Duty R/W
            // 0b00 : 12.5%
            // 0b01 : 25%
            // 0b10 : 50%
            // 0b11 : 75%
            // 5-0: Initial Length timer W only

            // the higher the value is the shorter the time before it gets cut

            return 0x3F | Ch1.NR11.Raw & 0xC0;
            break;
        case 0xFF12:
            
            // Channel Volume and Envelope - Controls digital amplitude

            // 7-4: initial Volume
            // - how loud the initial channel volume is (not updated by envelope)
            // 3: Env Dir
            // - 0: decrease volume over time
            // - 1: increas volume over time
            // 2-0: Sweep Pace
            // - the envelope ticks at 64hz
            // - envelope will change every sweep pace (+ or - , -> bit 3)
            // - 0 will disable envelope

            // setting bits 3-7 to 0 will turn off DAC and channel, and cause audio pop

            return Ch1.NR12.Raw;
            break;
        case 0xFF13:

            // Channel Period Low
            // - Stores the lower 8 bits of the Channels  11 bit period value
            // Write only
            return 0xFF;
            break;
        case 0xFF14:
            // Channel Period high and control
            // 7: trigger
            // - triggering this causes these to occur:
            //   - Ch turns on
            //   - if length timer is expired they reset
            //   - the period divider is set to contents of NR14 and NR13
            //   - Envelope Timer is reset
            //   - volume is set to contents of NR12 (intial volume)
            //   - Sweep does a bunch of things?
            // 6: Length Enable
            // 2-0: upper 3 bits of period
            return 0xBF | Ch1.NR14.Raw & 0x40;
            break;
        case 0xFF16:
            // Channel 2 Lacks Sweep mechanics but other regs remain same as Channel 1
            return 0x3F | Ch2.NR21.Raw & 0xC0;
            break;
        case 0xFF17:
            return Ch2.NR22.Raw;
            break;
        case 0xFF18:
            // write only
            return 0xFF;
            break;
        case 0xFF19:
            return 0xBF | Ch2.NR24.Raw & 0x40;
            break;
        case 0xFF1A:
            // Channel 3 - Wave Output
            // NR30 - Channel 3 DAC Enable

            // 7: Dac on/off  - turns off the channel as well
            return 0x7F | Ch3.NR30.Raw;
            break;
        case 0xFF1B:
            // Length timer Write only

            return 0xFF;
            break;
        case 0xFF1C:
            // Output Level - no envelope
            // 6-5:
            // - 0b00 Mute No Sound
            // - 0b01 (100% sound) uses wave ram samples as is
            // - 0b10 (50% sound) uses shifted wave ram samples (shifts to the right once)
            // - 0b11 (50% sound) uses shifted wave ram samples (shifts to the right twice)
            return 0x9F | Ch3.NR32.Raw;
            break;
        case 0xFF1D:
            // same as channels 1-2 NR33
            // slightly different timings
            return 0xFF;
            break;
        case 0xFF1E:
            // same as channels 1-2
            // except no envelope functionality
            // instead resets wave ram index
            return 0xBF | Ch3.NR34.Raw & 0x40;
            break;
        case 0xFF20:
            // Channel 4 - Noise
            // - used to output white noise
            // Length Timer - NR41
            // 5-0 : inital length timer

            return 0xFF; // Write only
            break;
        case 0xFF21:
            // volume and envelope - NR42
            // same as NR12
            return Ch4.NR42.Raw;
            break;
        case 0xFF22:
            // Frequency and randomness - NR43
            // 7-4: clock shift
            // 3: LFSR width
            // - 0: 15 bit
            // - 1: 7 bit
            // 2-0: Clock divider

            // The frequency the LFSR is clocked at
            // 262144/ (clock divider * (2^shift))
            // if clock divider = 0 it uses 0.5 instead
            return Ch4.NR43.Raw;
            break;
        case 0xFF23:
            // Control - NR44
            // - Ch4 is enabled.
            // - If the length timer expired it is reset.
            // - Envelope timer is reset.
            // - Volume is set to contents of NR42 initial volume.
            // - LFSR bits are reset.
            return 0xBF | Ch4.NR44.Raw & 0x40;
            break;
        case 0xFF24:
            // Master Volume and VIN Panning
            // 7: Vin Left  = 0 if no external hardware in use
            // 6-4: Left Volume - Scales Master volume
            // 3: Vin Right = 0 if no external hardware in use
            // 2-0: Right Volume - Scales Master volume
            return GlobalRegs.NR50.Raw;
            break;
        case 0xFF25:
            // Sound Panning - NR51
            // 7-4 ch 4-1 left
            // 3-0 ch 4-1 right
            return GlobalRegs.SoundPanningNR51.Raw;
            break;
        case 0xFF26:
            // Audio Master Control - NR52
            // 7: Audio Master control on/off 
            // 0-3: Ch 1-4 on?
            return 0x70 | GlobalRegs.AudioMasterControlNR52.Raw;
        default:
            break;
        }


        return 0xFF;

    }
    void APU::write(u16 address, u8 data)
    {   
        // When APU is not turned on only NR52 is writeable
        if(address == 0xFF26)
        {
            GlobalRegs.AudioMasterControlNR52.OnOff = data >> 7;
            if(!GlobalRegs.AudioMasterControlNR52.OnOff) // Turn APU OFF
            {

                Ch1.init();
                Ch2.init();
                Ch3.init();
                Ch4.init();

                GlobalRegs.NR50.Raw = 0x00;
                GlobalRegs.SoundPanningNR51.Raw = 0x00;
            }
            else // Turn APU ON
            {
                DIVAPU = 7;
                Ch1.Phase = 0;
                Ch2.Phase = 0;
                Ch3.Phase = 0;
                Ch4.Phase = 0;

            }
            return;
        }
        else if (!(GlobalRegs.AudioMasterControlNR52.OnOff))
        {
            // Even when off the Length Timers can be written to
            if(address == 0xFF11)
            {
                Ch1.NR11.Raw = data;
                Ch1.RemainingLength =  64 - (data & 0x3F);
            }
            else if(address == 0xFF16)
            {
                Ch2.NR21.Raw = data;
                Ch2.RemainingLength = 64 - (data & 0x3F);
            }
            else if(address == 0xFF1B)
            {
                Ch3.NR31 = data;
                Ch3.RemainingLength = 256 - data;
            }
            else if(address == 0xFF20)
            {
                Ch4.NR41 = data;
                Ch4.RemainingLength =  64 - (data & 0x3F);
            }
            else
            return;
        }

        switch (address)
        {
        case 0xFF10:
            Ch1.NR10.Raw = data;
            if(data == 9)
            {
                std::cout << " break";
            }
            if(!Ch1.NR10.SweepDirection && Ch1.negateModeCalculations > 0)
            {
                GlobalRegs.AudioMasterControlNR52.Ch1On = 0;
            }
            break;
        case 0xFF11:
            // Bits 7 and 6 are Ch1's wave duty
            // Bits 5-0 are initial length timer

            Ch1.NR11.Raw = data;

            Ch1.RemainingLength =  64 - (data & 0x3F);
            break;
        case 0xFF12:
            Ch1.NR12.Raw = data;
            if ((data & 0xF8) !=0)
            {
                DACState[0] = 1;
            }
            else
            {
                DACState[0] = 0;
                GlobalRegs.AudioMasterControlNR52.Ch1On = 0;
            }
            break;
        case 0xFF13:
            Ch1.NR13 = data;
            break;
        case 0xFF14:
            Ch1.LengthPrevEnabled = Ch1.NR14.LengthEnable;
            Ch1.NR14.Raw = data;

            if(Ch1.NR14.LengthEnable && ! Ch1.LengthPrevEnabled)
            {
                if(!CheckNextFrame(LengthSequence))
                {
                    if(Ch1.RemainingLength == 0)
                    {
                        
                    }
                    else if(--Ch1.RemainingLength == 0 && !Ch1.NR14.Trigger)
                    {
                        GlobalRegs.AudioMasterControlNR52.Ch1On = 0;
                    }
                }
            }

            // Channel 1 Trigger Event
            if (Ch1.NR14.Trigger)
            {
                Ch1.trigger();
            }
            break;
        case 0xFF16:
            // same as channel 1 but for channel 2
            Ch2.NR21.Raw = data;
            Ch2.RemainingLength = 64 - (data & 0x3F);
            break;
        case 0xFF17:
            Ch2.NR22.Raw = data;
            if ((data & 0xF8) !=0)
            {
                DACState[1] = 1;
            }
            else
            {
                DACState[1] = 0;
                GlobalRegs.AudioMasterControlNR52.Ch2On = 0;
            }
            break;
        case 0xFF18:
            Ch2.NR23 = data;
            break;
        case 0xFF19:
            Ch2.LengthPrevEnabled = Ch2.NR24.LengthEnable;
            Ch2.NR24.Raw = data;

            if(Ch2.NR24.LengthEnable && ! Ch2.LengthPrevEnabled)
            {
                if(!CheckNextFrame(LengthSequence))
                {
                    if(Ch2.RemainingLength == 0)
                    {
                        
                    }
                    else if(--Ch2.RemainingLength == 0 && !Ch2.NR24.Trigger)
                    {
                        GlobalRegs.AudioMasterControlNR52.Ch2On = 0;
                    }
                }
            }
            // Channel 1 Trigger Event
            if (Ch2.NR24.Trigger)
            {
                Ch2.trigger();
            }
            break;
        case 0xFF1A:
            Ch3.NR30.Raw = data;
            if(Ch3.NR30.DACOnOff)
            {
                DACState[2] = 1;
            }
            else
            {
                DACState[2] = 0;
                GlobalRegs.AudioMasterControlNR52.Ch3On = 0;
            }
            break;
        case 0xFF1B:
            // for wave channel Initial length timer 256-0
            Ch3.NR31 = data;
            Ch3.RemainingLength = 256 - data;

            break;
        case 0xFF1C:
            Ch3.NR32.Raw = data;
            break;
        case 0xFF1D:
            Ch3.NR33 = data;
            break;
        case 0xFF1E:
            Ch3.LengthPrevEnabled = Ch3.NR34.LengthEnable;
            Ch3.NR34.Raw = data;

            if(Ch3.NR34.LengthEnable && ! Ch3.LengthPrevEnabled)
            {
                if(!CheckNextFrame(LengthSequence))
                {
                    if(Ch3.RemainingLength == 0)
                    {
                        
                    }
                    else if(--Ch3.RemainingLength == 0 && !Ch3.NR34.Trigger)
                    {
                        GlobalRegs.AudioMasterControlNR52.Ch3On = 0;
                    }
                }
            }

            if(Ch3.NR34.Trigger)
            {
                Ch3.trigger();
            }
            break;
        case 0xFF20:
            Ch4.NR41 = data;
            Ch4.RemainingLength =  64 - (data & 0x3F);
            break;
        case 0xFF21:
            Ch4.NR42.Raw = data;
            if ((data & 0xF8) !=0)
            {
                DACState[3] = 1;
            }
            else
            {
                DACState[3] = 0;
                GlobalRegs.AudioMasterControlNR52.Ch4On = 0;
            }
            break;
        case 0xFF22:
            Ch4.NR43.Raw = data;
            break;
        case 0xFF23:
            Ch4.LengthPrevEnabled = Ch4.NR44.LengthEnable;
            Ch4.NR44.Raw = data;

            if(Ch4.NR44.LengthEnable && ! Ch4.LengthPrevEnabled)
            {
                if(!CheckNextFrame(LengthSequence))
                {
                    if(Ch4.RemainingLength == 0)
                    {
                        
                    }
                    else if(--Ch4.RemainingLength == 0 && !Ch4.NR44.Trigger)
                    {
                        GlobalRegs.AudioMasterControlNR52.Ch4On = 0;
                    }
                }
            }

            if(Ch4.NR44.Trigger)
            {
                Ch4.trigger();
            }
            break;
        case 0xFF24:
            GlobalRegs.NR50.Raw = data;
            break;
        case 0xFF25:
            GlobalRegs.SoundPanningNR51.Raw = data;
            break;
        default:
            break;
        }
    }


    void Channel1::init()
    {
        NR10.Raw = 0x00;
        NR11.Raw = 0x00;
        NR12.Raw = 0x00;
        NR13 = 0x00;
        NR14.Raw = 0x00;

        apu->GlobalRegs.AudioMasterControlNR52.Ch1On = 0;
        
        Frequency = 0;
        FrequencyTimer = 0;
        //RemainingLength = 0;
        EnvelopeEnabled = false;
        EnvelopeTimer = 0;
        Volume = 0;
        SweepTimer = 0;


        Phase = 0;
        DutyPhaseTimer = 0;
    }


    void Channel1::trigger()
    {
        // Enable Channel 1
        if(apu->DACState[0])
            apu->GlobalRegs.AudioMasterControlNR52.Ch1On = 1;
        // Reset Length Timer if expired
        if(RemainingLength == 0)
        {
            RemainingLength = 64;
            if(!apu->CheckNextFrame(apu->LengthSequence) && NR14.LengthEnable)
            {
                RemainingLength--;
            }
        }
        // Set Period Divider?? Same as Frequency shadow reg

        // Envelope timer is reset
        // if next frame sequence will clock the envelope the evelope timer is +1 the actual
        if(apu->CheckNextFrame(apu->EnvelopeSequence))
        {
            EnvelopeTimer = NR12.EnvPace + 1;
        }
        else
        {
            EnvelopeTimer = NR12.EnvPace;
        }

        //Volume is set
        Volume = NR12.InitialVolume;
        // Envelope is Reanabled
        EnvelopeEnabled = true;
        
        /******************Sweep Related Trigger Events****************************/
        
        // Reset how many negate mode calculations occured
        negateModeCalculations = 0;
        // Reload Frequency Shadow Reg with period/Frequency
        Frequency = NR14.UpperPeriod << 8 | NR13;
        // Reset SweepTimer equals 8 if pace is 0
        SweepTimer = NR10.SweepPace == 0 ? 8 : NR10.SweepPace;

        // Set enable flag on certain conditions
        if(NR10.SweepPace != 0 || NR10.SweepStep != 0)
        {
            SweepEnabled = true;
        }
        else
        {
            SweepEnabled = false;
        }
        
        if(NR10.SweepStep != 0)
        {
            FrequencyCalculation();
        }

        // Reload Frequency Timer
        FrequencyTimer = ((2048 - Frequency) * 4) & 0xFC;

    }

    void Channel1::tick()
    {   
        if(!apu->GlobalRegs.AudioMasterControlNR52.Ch1On)
        {
            return;
        }
        if(--FrequencyTimer == 0)
        {
            FrequencyTimer = (2048 - Frequency) *4;
            Phase = (Phase + 1) & 8;
        }
    }

    void Channel2::init()
    {
        NR21.Raw = 0x00;
        NR22.Raw = 0x0;
        NR23 = 0x00;
        NR24.Raw = 0x00;
        
        apu->GlobalRegs.AudioMasterControlNR52.Ch2On = 0;

        Frequency = 0;
        FrequencyTimer = 0;
        //RemainingLength = 0;
        EnvelopeEnabled = false;
        EnvelopeTimer = 0;
        Volume = 0;

        Phase = 0;
        DutyPhaseTimer = 0;
    }

    void Channel2::trigger()
    {
        // Enable Channel 1
        if(apu->DACState[1])
            apu->GlobalRegs.AudioMasterControlNR52.Ch2On = 1;
        // Reset Length Timer if expired
        if(RemainingLength == 0)
        {
            RemainingLength = 64;
            if(!apu->CheckNextFrame(apu->LengthSequence) && NR24.LengthEnable)
            {
                RemainingLength--;
            }
        }

        // Set Period Divider?? Same as Frequency shadow reg
        Frequency = NR24.UpperPeriod << 8 | NR23;
        FrequencyTimer = ((2048 - Frequency) * 4) & 0xFC;
        // Envelope timer is reset
        // if next frame sequence will clock the envelope the evelope timer is +1 the actual
        if(apu->CheckNextFrame(apu->EnvelopeSequence))
        {
            EnvelopeTimer = NR22.EnvPace + 1;
        }
        else
        {
            EnvelopeTimer = NR22.EnvPace;
        }
        //Volume is set
        Volume = NR22.InitialVolume;
        // Envelope is Reanabled
        EnvelopeEnabled = true;


    }

    void Channel2::tick()
    {
        if(!apu->GlobalRegs.AudioMasterControlNR52.Ch2On)
        {
            return;
        }
        if(--FrequencyTimer == 0)
        {
            FrequencyTimer = (2048 - Frequency) *4;
            Phase = (Phase + 1) & 8;
        }
    }

    void Channel3::init()
    {
        NR30.Raw = 0x00;
        NR31 = 0x00;
        NR32.Raw = 0x00;
        NR33 = 0x00;
        NR34.Raw = 0x00;

        apu->GlobalRegs.AudioMasterControlNR52.Ch3On = 0;
        
        Frequency = 0;
        FrequencyTimer = 0;
        //RemainingLength = 0;
        EnvelopeEnabled = false;
        Volume = 0;

        SampleBuffer = 0;
        WaveIndex = 0;

        Phase = 0;
        DutyPhaseTimer = 0;
    }

    void Channel3::trigger()
    {
        // Ch3 is enabled.
        if(apu->DACState[2])
            apu->GlobalRegs.AudioMasterControlNR52.Ch3On = 1;
        // If the length timer expired it is reset.
        if(RemainingLength == 0)
        {
            RemainingLength = 256;
            if(!apu->CheckNextFrame(apu->LengthSequence) && NR34.LengthEnable)
            {
                RemainingLength--;
            }
        }
        // The period divider is set to the contents of NR33 and NR34.
        Frequency = NR34.UpperPeriod << 8 | NR33;
        // Frequency Timer is set
        FrequencyTimer = (2048 - Frequency) * 2;
        // Volume is set to contents of NR32 initial volume.
        Volume = NR32.OutputLevel;
        // Wave RAM index is reset, but its not refilled.
        WaveIndex = 0;


    }

    void Channel3::tick()
    {
        if(!apu->GlobalRegs.AudioMasterControlNR52.Ch3On)
        {
            return;
        }

        if(--FrequencyTimer == 0)
        {
            FrequencyTimer = 2 * (2048 - Frequency);
            
            if(WaveIndex % 2 == 1)
            {
                SampleBuffer = apu->waveRam[WaveIndex/2] & 0x0F;
            }
            else
            {
                SampleBuffer = (apu->waveRam[WaveIndex/2] & 0xF0 ) >> 4;
            }

            WaveIndex = (WaveIndex +1) % 32;
        }
    }

    void Channel4::init()
    {
        NR41 = 0x00;
        NR42.Raw = 0x0;
        NR43.Raw = 0x0;
        NR44.Raw = 0x00;

        apu->GlobalRegs.AudioMasterControlNR52.Ch4On = 0;
        
        Frequency = 0;
        FrequencyTimer = 0;
        //RemainingLength = 0;
        EnvelopeEnabled = false;
        EnvelopeTimer = 0;
        Volume = 0;

        LFSR = 0x00;

        Phase = 0;
        DutyPhaseTimer = 0;
    }

    void Channel4::trigger()
    {
        // Enable Channel 4
        if(apu->DACState[3])
            apu->GlobalRegs.AudioMasterControlNR52.Ch4On = 1; 
        // Reset Length Timer if expired
        if(RemainingLength == 0)
        {
            RemainingLength = 64;
            if(!apu->CheckNextFrame(apu->LengthSequence)&& NR44.LengthEnable)
            {
                RemainingLength--;
            }
        }

        // Set frequency Timer
        FrequencyTimer = 4194304/int(DivisorCodes[NR43.ClockDivider] << (NR43.ClockShift + 1));
        // Envelope timer is reset 
        // if next frame sequence will clock the envelope the evelope timer is +1 the actual
        if(apu->CheckNextFrame(apu->EnvelopeSequence))
        {
            EnvelopeTimer = NR42.SweepPace + 1;
        }
        else
        {
            EnvelopeTimer = NR42.SweepPace;
        }
        //Volume is set
        Volume = NR42.InitialVolume;
        // Reset LFSR Bits
        LFSR = 0xFF;


    }

    void Channel4::tick()
    {

        if(!apu->GlobalRegs.AudioMasterControlNR52.Ch4On)
        {
            return;
        }
        if(--FrequencyTimer == 0)
        {
            // Grab the first 2 bits
            u8 Bit0 = LFSR & 1;
            u8 Bit1 = (LFSR >> 1) & 1;

            // shift LFSR to the right by 1 and place XOR of bit 0 and 1 in bit 15
            LFSR = ((Bit0 ^ Bit1) << 15) | (LFSR >> 1);

            // Depending on the Width, bit 7 also gets this value
            if(NR43.LFSRWidth)
            {
                LFSR = ((Bit0 ^ Bit1) << 7) | (LFSR & ~(1<<7));
            }
        }
    }
}