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
        ChannelState.fill(0);
        DACState.fill(0);
        LengthCounter.fill(0);
    }

    u8 APU::waveFormGenerator(u8 Phase,u8 duty)
    {
        std::array<u8,8> waveform;
        switch(duty)
        {
            case 0b00:
                waveform = {0,0,0,0,0,0,0,1}; // 12.5%
            case 0b01:
                waveform = {1,0,0,0,0,0,0,1}; // 25%
            case 0b10:
                waveform = {1,0,0,0,0,1,1,1}; // 50%
            case 0b11:
                waveform = {0,1,1,1,1,1,1,0}; // 75%
        }
        return waveform[Phase];
    }

    void APU::FrameSequencer()
    {
        // Occurs Every 2 Low Frequency Clocks (DIVAPU)
        LengthCTRClock = (DIVAPU % 2 == 0) ? true : false;
        // Occurs Every 4 Low Frequency Clocks (DIVAPU)
        SweepClock = (DIVAPU % 4 == 0) ? true : false;
        // Occurs Every 8 Low Frequency Clocks (DIVAPU)
        VolEnvelopeClock = (DIVAPU % 8 == 0) ?  true : false;
    }

    void APU::Ch1Sweep()
    {

    }
    void APU::tick()
    {

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

            return Channel1.NR10.Raw;
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

            return Channel1.NR11.Raw & 0xC0;
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

            return Channel1.NR12.Raw;
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
            return Channel1.NR14.Raw & 0x40;
            break;
        case 0xFF16:
            // Channel 2 Lacks Sweep mechanics but other regs remain same as Channel 1
            return Channel2.NR21.Raw & 0xC0;
            break;
        case 0xFF17:
            return Channel2.NR22.Raw;
            break;
        case 0xFF18:
            // write only
            return 0xFF;
            break;
        case 0xFF19:
            return Channel2.NR24.Raw & 0x40;
            break;
        case 0xFF1A:
            // Channel 3 - Wave Output
            // NR30 - Channel 3 DAC Enable

            // 7: Dac on/off  - turns off the channel as well
            return Channel3.NR30.Raw;
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
            return Channel3.NR32.Raw;
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
            return Channel3.NR34.Raw & 0x40;
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
            return Channel4.NR42.Raw;
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
            return Channel4.NR43.Raw;
            break;
        case 0xFF23:
            // Control - NR44
            // - Ch4 is enabled.
            // - If the length timer expired it is reset.
            // - Envelope timer is reset.
            // - Volume is set to contents of NR42 initial volume.
            // - LFSR bits are reset.
            return Channel4.NR44.Raw & 0x40;
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
            return GlobalRegs.AudioMasterControlNR52.Raw;
        default:
            return waveRam[address - 0xFF30];
            break;
        }
    }
    void APU::write(u16 address, u8 data)
    {   
        // When APU is not turned on only NR52 is writeable
        if(address == 0xFF26)
        {
            GlobalRegs.AudioMasterControlNR52.Raw = (GlobalRegs.AudioMasterControlNR52.Raw & 0x7F) | (data & 0x80);
            return;
        }
        else if (!(GlobalRegs.AudioMasterControlNR52.OnOff))
        {
            return;
        }

        switch (address)
        {
        case 0xFF10:
            Channel1.NR10.Raw = data;
            break;
        case 0xFF11:
            // Bits 7 and 6 are Channel1's wave duty
            // Bits 5-0 are initial length timer

            Channel1.NR11.InitialTimerLength = 64 - (data & 0x3F);
            Channel1.NR11.Duty = data >> 6;
            break;
        case 0xFF12:
            Channel1.NR12.Raw = data;
            break;
        case 0xFF13:
            Channel1.NR13 = data;
            break;
        case 0xFF14:
            Channel1.NR14.Raw = data;

            if(data & (1<<7))
            {

            }
            break;
        case 0xFF16:
            // same as channel 1 but for channel 2

            break;
        case 0xFF17:
            break;
        case 0xFF18:
            break;
        case 0xFF19:
            if(data & (1<<7))
            {
                // Trigger Channel 2
            }
            break;
        case 0xFF1A:
            break;
        case 0xFF1B:
            // for wave channel Initial length timer 256-0

            break;
        case 0xFF1C:
            break;
        case 0xFF1D:
            break;
        case 0xFF1E:
            if(data & (1<<7))
            {
                // Trigger Channel 3
            }
            break;
        case 0xFF20:
            break;
        case 0xFF21:
            break;
        case 0xFF22:
            break;
        case 0xFF23:
            if(data & (1<<7))
            {
                // Trigger Channel 4
            }
            break;
        case 0xFF24:
            break;
        case 0xFF25:
            break;
        default:
            waveRam[address - 0xFF30] = data;
            break;
        }
    }
}