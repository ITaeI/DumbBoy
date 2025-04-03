#include "cpu.h"
#include "Emulator.h"

namespace GBEmu
{

    void cpu::connectCPU(Emulator *emu)
    {
        Emu = emu;
    }

    bool cpu::step()
    {
        
        if(!isHalted)
        {
            
            opcode = read_memory(reg.pc.read());
            if (opcode == 0xCB)
            {   
                opcode = read_memory(reg.pc.read());
                executeCBInstruction();
            }
            else
            {   
                u8 temp = Emu->systemBus.read(0xFF02);
                if(temp == 0x81)
                {
                    DBG.push_back((char)(Emu->systemBus.read(0xFF01)));
                    Emu->systemBus.write(0xFF02,0x00);
                    std::cout << "Debug Message: " << DBG << std::endl;
                }

                if(reg.pc.read() -1  == 0x7DA)
                {
                    std::cout << "breakpoint" << std::endl;
                }
    
                executeInstruction();
            }
        }
        else{

            Emu->ClockCycle(1);
            if(IF.read())
            {
                isHalted = false;
            }
            
        }

        if(IME)
        {
            HandleInterrupts();
            enablingIME = false;
        }

        if (enablingIME)
        {
            IME = true;
        }
        return 0;
    }

    void cpu::HandleInterrupts()
    {
        if(IE.readBit(VBlank) && IF.readBit(VBlank))
        {
            PUSH_R16(reg.pc);
            reg.pc.write(0x40);
            IME = false;
            IF.setBit(VBlank,0);
        }
        else if (IE.readBit(LCD) && IF.readBit(LCD))
        {
            PUSH_R16(reg.pc);
            reg.pc.write(0x48);
            IME = false;
            IF.setBit(LCD,0);
        }
        else if (IE.readBit(Timer) && IF.readBit(Timer))
        {
            PUSH_R16(reg.pc);
            reg.pc.write(0x50);
            IME = false;
            IF.setBit(Timer,0);
        }
        else if (IE.readBit(Serial) && IF.readBit(Serial))
        {
            PUSH_R16(reg.pc);
            reg.pc.write(0x58);
            IME = false;
            IF.setBit(Serial,0);
        }
        else if (IE.readBit(Joypad) && IF.readBit(Joypad))
        {
            PUSH_R16(reg.pc);
            reg.pc.write(0x60);
            IME = false;
            IF.setBit(Joypad,0);
        }
    }

    void cpu::init()
    {
        reg.af.write(0x0180);
        reg.bc.write(0x0013);
        reg.de.write(0x00D8);
        reg.hl.write(0x014D);
        reg.pc.write(0x0100);
        reg.sp.write(0xFFFE);


        isHalted = false;
        enablingIME = 0;
        IME = 0;
        current_cycles = 0;
        Emu->timer.timerRegs.DIV.write(0xABCC);
        currentInstruction = Instruction();

        // For Debugging
        DBG = "";
    }

    u8 cpu::read_memory(u16 addr)
    {   

        u8 mem = Emu->systemBus.read(addr);
        if (addr == reg.pc.read())
        {
            reg.pc.Increment();
        }
        Emu->ClockCycle(1);
        return mem;
    }


    void cpu::write_memory(u16 addr, u8 data)
    {   
        Emu->ClockCycle(1);
        Emu->systemBus.write(addr, data);
    }

    u16 cpu::fetch16Bit()
    {
        u8 lowByte = read_memory(reg.pc.read());
        u8 highByte = read_memory(reg.pc.read());
        return buildAdress(lowByte,highByte);
    }

    u16 cpu::buildAdress(u8 low, u8 high)
    {
        return (high << 8 | low);
    }


    // *************Flag Context***************** //

    void cpu::setFlags(bool z, bool n, bool h, bool c )
    {
        reg.af.getLowByte().setBit(Z,z);
        reg.af.getLowByte().setBit(N,n);
        reg.af.getLowByte().setBit(H,h);
        reg.af.getLowByte().setBit(C,c);

    }
    u8 cpu::getFlag(int FlagValue)
    {
        u8 flag = reg.af.getLowByte().readBit(FlagValue);
        return flag;
    }

    bool cpu::HalfCarry8Bit(u8 n1, u8 n2, bool subtraction)
    {   

        bool b = subtraction ? ((n1 & 0x0F) - (n2 & 0x0F)) < 0 : ((n1 & 0x0F) + (n2 & 0x0F)) > 0x0F;
        return b;
    }

    bool cpu::Carry8Bit(u8 n1, u8 n2,bool subtraction)
    {   

        bool b = subtraction ? n1 < n2: (n1 + n2) > 0xFF;
        return b;
    }

    bool cpu::HalfCarry16bit(u16 n1, u16 n2, bool subtraction)
    {

        bool b = subtraction ? ((n1 & 0x0FFF) - (n2 & 0x0FFF)) < 0 : ((n1 & 0x0FFF) + (n2 & 0x0FFF)) > 0x0FFF;
        return b;
    }
    bool cpu::Carry16bit(u16 n1, u16 n2, bool subtraction)
    {

        bool b = subtraction ? n1 < n2: (n1 + n2) > 0xFFFF;
        return b;
    }


    void cpu::executeCBInstruction()
    {
        switch (opcode)
        {
            
            case 0x00: currentInstruction = Instruction("RLC B", 2,8); RLC_R8(reg.bc.getHighByte()); break;
            case 0x01: currentInstruction = Instruction("RLC C", 2,8); RLC_R8(reg.bc.getLowByte()); break;
            case 0x02: currentInstruction = Instruction("RLC D", 2,8); RLC_R8(reg.de.getHighByte()); break;
            case 0x03: currentInstruction = Instruction("RLC E", 2,8); RLC_R8(reg.de.getLowByte()); break;
            case 0x04: currentInstruction = Instruction("RLC H", 2,8); RLC_R8(reg.hl.getHighByte()); break;
            case 0x05: currentInstruction = Instruction("RLC L", 2,8); RLC_R8(reg.hl.getLowByte()); break;
            case 0x06: currentInstruction = Instruction("RLC HL", 2,16); RLC_HL(); break;
            case 0x07: currentInstruction = Instruction("RLC A", 2,8); RLC_R8(reg.af.getHighByte()); break;
            case 0x08: currentInstruction = Instruction("RRC B", 2,8); RRC_R8(reg.bc.getHighByte()); break;
            case 0x09: currentInstruction = Instruction("RRC C", 2,8); RRC_R8(reg.bc.getLowByte()); break;
            case 0x0A: currentInstruction = Instruction("RRC D", 2,8); RRC_R8(reg.de.getHighByte()); break;
            case 0x0B: currentInstruction = Instruction("RRC E", 2,8); RRC_R8(reg.de.getLowByte()); break;
            case 0x0C: currentInstruction = Instruction("RRC H", 2,8); RRC_R8(reg.hl.getHighByte()); break;
            case 0x0D: currentInstruction = Instruction("RRC L", 2,8); RRC_R8(reg.hl.getLowByte()); break;
            case 0x0E: currentInstruction = Instruction("RRC HL", 2,16); RRC_HL(); break;
            case 0x0F: currentInstruction = Instruction("RRC A", 2,8); RRC_R8(reg.af.getHighByte()); break;
            case 0x10: currentInstruction = Instruction("RL B", 2,8); RL_R8(reg.bc.getHighByte()); break;
            case 0x11: currentInstruction = Instruction("RL C", 2,8); RL_R8(reg.bc.getLowByte()); break;
            case 0x12: currentInstruction = Instruction("RL D", 2,8); RL_R8(reg.de.getHighByte()); break;
            case 0x13: currentInstruction = Instruction("RL E", 2,8); RL_R8(reg.de.getLowByte()); break;
            case 0x14: currentInstruction = Instruction("RL H", 2,8); RL_R8(reg.hl.getHighByte()); break;
            case 0x15: currentInstruction = Instruction("RL L", 2,8); RL_R8(reg.hl.getLowByte()); break;
            case 0x16: currentInstruction = Instruction("RL HL", 2,6); RL_HL(); break;
            case 0x17: currentInstruction = Instruction("RL A", 2,8); RL_R8(reg.af.getHighByte()); break;
            case 0x18: currentInstruction = Instruction("RR B", 2,8); RR_R8(reg.bc.getHighByte()); break;
            case 0x19: currentInstruction = Instruction("RR C", 2,8); RR_R8(reg.bc.getLowByte()); break;
            case 0x1A: currentInstruction = Instruction("RR D", 2,8); RR_R8(reg.de.getHighByte());break;
            case 0x1B: currentInstruction = Instruction("RR E", 2,8); RR_R8(reg.de.getLowByte()); break;
            case 0x1C: currentInstruction = Instruction("RR H", 2,8); RR_R8(reg.hl.getHighByte()); break;
            case 0x1D: currentInstruction = Instruction("RR L", 2,8); RR_R8(reg.hl.getLowByte()); break;
            case 0x1E: currentInstruction = Instruction("RR HL", 2,16); RR_HL(); break;
            case 0x1F: currentInstruction = Instruction("RR A", 2,8); RR_R8(reg.af.getHighByte()); break;
            case 0x20: currentInstruction = Instruction("SLA B", 2,8); SLA_R8(reg.bc.getHighByte()); break;
            case 0x21: currentInstruction = Instruction("SLA C", 2,8); SLA_R8(reg.bc.getLowByte()); break;
            case 0x22: currentInstruction = Instruction("SLA D", 2,8); SLA_R8(reg.de.getHighByte()); break;
            case 0x23: currentInstruction = Instruction("SLA E", 2,8); SLA_R8(reg.de.getLowByte()); break;
            case 0x24: currentInstruction = Instruction("SLA H", 2,8); SLA_R8(reg.hl.getHighByte()); break;
            case 0x25: currentInstruction = Instruction("SLA L", 2,8); SLA_R8(reg.hl.getLowByte()); break;
            case 0x26: currentInstruction = Instruction("SLA HL", 2,16); SLA_HL(); break;
            case 0x27: currentInstruction = Instruction("SLA A", 2,8); SLA_R8(reg.af.getHighByte()); break;
            case 0x28: currentInstruction = Instruction("SRA B", 2,8); SRA_R8(reg.bc.getHighByte()); break;
            case 0x29: currentInstruction = Instruction("SRA C", 2,8); SRA_R8(reg.bc.getLowByte()); break;
            case 0x2A: currentInstruction = Instruction("SRA D", 2,8); SRA_R8(reg.de.getHighByte()); break;
            case 0x2B: currentInstruction = Instruction("SRA E", 2,8); SRA_R8(reg.de.getLowByte()); break;
            case 0x2C: currentInstruction = Instruction("SRA H", 2,8); SRA_R8(reg.hl.getHighByte()); break;
            case 0x2D: currentInstruction = Instruction("SRA L", 2,8); SRA_R8(reg.hl.getLowByte()); break;
            case 0x2E: currentInstruction = Instruction("SRA HL", 2,16); SRA_HL(); break;
            case 0x2F: currentInstruction = Instruction("SRA A", 2,8); SRA_R8(reg.af.getHighByte()); break;
            case 0x30: currentInstruction = Instruction("SWAP B", 2,8); SWAP_R8(reg.bc.getHighByte()); break;
            case 0x31: currentInstruction = Instruction("SWAP C", 2,8); SWAP_R8(reg.bc.getLowByte()); break;
            case 0x32: currentInstruction = Instruction("SWAP D", 2,8); SWAP_R8(reg.de.getHighByte()); break;
            case 0x33: currentInstruction = Instruction("SWAP E", 2,8); SWAP_R8(reg.de.getLowByte());break;
            case 0x34: currentInstruction = Instruction("SWAP H", 2,8); SWAP_R8(reg.hl.getHighByte()); break;
            case 0x35: currentInstruction = Instruction("SWAP L", 2,8); SWAP_R8(reg.hl.getLowByte()); break;
            case 0x36: currentInstruction = Instruction("SWAP HL", 2,16); SWAP_HL(); break;
            case 0x37: currentInstruction = Instruction("SWAP A", 2,8); SWAP_R8(reg.af.getHighByte()); break;
            case 0x38: currentInstruction = Instruction("SRL B", 2,8); SRL_R8(reg.bc.getHighByte()); break;
            case 0x39: currentInstruction = Instruction("SRL C", 2,8); SRL_R8(reg.bc.getLowByte()); break;
            case 0x3A: currentInstruction = Instruction("SRL D", 2,8); SRL_R8(reg.de.getHighByte()); break;
            case 0x3B: currentInstruction = Instruction("SRL E", 2,8); SRL_R8(reg.de.getLowByte()); break;
            case 0x3C: currentInstruction = Instruction("SRL H", 2,8); SRL_R8(reg.hl.getHighByte()); break;
            case 0x3D: currentInstruction = Instruction("SRL L", 2,8); SRL_R8(reg.hl.getLowByte()); break;
            case 0x3E: currentInstruction = Instruction("SRL HL", 2,16); SRL_HL(); break;
            case 0x3F: currentInstruction = Instruction("SRL A", 2,8); SRL_R8(reg.af.getHighByte()); break;
            case 0x40: currentInstruction = Instruction("BIT 0 B", 2,8); BIT_B_R8(reg.bc.getHighByte(),0); break;
            case 0x41: currentInstruction = Instruction("BIT 0 C", 2,8); BIT_B_R8(reg.bc.getLowByte(),0); break;
            case 0x42: currentInstruction = Instruction("BIT 0 D", 2,8); BIT_B_R8(reg.de.getHighByte(),0); break;
            case 0x43: currentInstruction = Instruction("BIT 0 E", 2,8); BIT_B_R8(reg.de.getLowByte(),0); break;
            case 0x44: currentInstruction = Instruction("BIT 0 H", 2,8); BIT_B_R8(reg.hl.getHighByte(),0); break;
            case 0x45: currentInstruction = Instruction("BIT 0 L", 2,8); BIT_B_R8(reg.hl.getLowByte(),0); break;
            case 0x46: currentInstruction = Instruction("BIT 0 HL", 2,12); BIT_B_HL(0); break;
            case 0x47: currentInstruction = Instruction("BIT 0 A", 2,8); BIT_B_R8(reg.af.getHighByte(),0); break;
            case 0x48: currentInstruction = Instruction("BIT 1 B", 2,8); BIT_B_R8(reg.bc.getHighByte(),1); break;
            case 0x49: currentInstruction = Instruction("BIT 1 C", 2,8); BIT_B_R8(reg.bc.getLowByte(),1); break;
            case 0x4A: currentInstruction = Instruction("BIT 1 D", 2,8); BIT_B_R8(reg.de.getHighByte(),1); break;
            case 0x4B: currentInstruction = Instruction("BIT 1 E", 2,8); BIT_B_R8(reg.de.getLowByte(),1); break;
            case 0x4C: currentInstruction = Instruction("BIT 1 H", 2,8); BIT_B_R8(reg.hl.getHighByte(),1); break;
            case 0x4D: currentInstruction = Instruction("BIT 1 L", 2,8); BIT_B_R8(reg.hl.getLowByte(),1);break;
            case 0x4E: currentInstruction = Instruction("BIT 1 HL", 2,12); BIT_B_HL(1); break;
            case 0x4F: currentInstruction = Instruction("BIT 1 A", 2,8); BIT_B_R8(reg.af.getHighByte(),1); break;
            case 0x50: currentInstruction = Instruction("BIT 2 B", 2,8); BIT_B_R8(reg.bc.getHighByte(),2); break;
            case 0x51: currentInstruction = Instruction("BIT 2 C", 2,8); BIT_B_R8(reg.bc.getLowByte(),2); break;
            case 0x52: currentInstruction = Instruction("BIT 2 D", 2,8); BIT_B_R8(reg.de.getHighByte(),2); break;
            case 0x53: currentInstruction = Instruction("BIT 2 E", 2,8); BIT_B_R8(reg.de.getLowByte(),2); break;
            case 0x54: currentInstruction = Instruction("BIT 2 H", 2,8); BIT_B_R8(reg.hl.getHighByte(),2); break;
            case 0x55: currentInstruction = Instruction("BIT 2 L", 2,8); BIT_B_R8(reg.hl.getLowByte(),2); break;
            case 0x56: currentInstruction = Instruction("BIT 2 HL", 2,12); BIT_B_HL(2); break;
            case 0x57: currentInstruction = Instruction("BIT 2 A", 2,8); BIT_B_R8(reg.af.getHighByte(),2); break;
            case 0x58: currentInstruction = Instruction("BIT 3 B", 2,8); BIT_B_R8(reg.bc.getHighByte(),3); break;
            case 0x59: currentInstruction = Instruction("BIT 3 C", 2,8); BIT_B_R8(reg.bc.getLowByte(),3); break;
            case 0x5A: currentInstruction = Instruction("BIT 3 D", 2,8); BIT_B_R8(reg.de.getHighByte(),3); break;
            case 0x5B: currentInstruction = Instruction("BIT 3 E", 2,8); BIT_B_R8(reg.de.getLowByte(),3); break;
            case 0x5C: currentInstruction = Instruction("BIT 3 H", 2,8); BIT_B_R8(reg.hl.getHighByte(),3); break;
            case 0x5D: currentInstruction = Instruction("BIT 3 L", 2,8); BIT_B_R8(reg.hl.getLowByte(),3); break;
            case 0x5E: currentInstruction = Instruction("BIT 3 HL", 2,12); BIT_B_HL(3); break;
            case 0x5F: currentInstruction = Instruction("BIT 3 A", 2,8); BIT_B_R8(reg.af.getHighByte(),3); break;
            case 0x60: currentInstruction = Instruction("BIT 4 B", 2,8); BIT_B_R8(reg.bc.getHighByte(),4); break;
            case 0x61: currentInstruction = Instruction("BIT 4 C", 2,8); BIT_B_R8(reg.bc.getLowByte(),4); break;
            case 0x62: currentInstruction = Instruction("BIT 4 D", 2,8); BIT_B_R8(reg.de.getHighByte(),4); break;
            case 0x63: currentInstruction = Instruction("BIT 4 E", 2,8); BIT_B_R8(reg.de.getLowByte(),4); break;
            case 0x64: currentInstruction = Instruction("BIT 4 H", 2,8); BIT_B_R8(reg.hl.getHighByte(),4); break;
            case 0x65: currentInstruction = Instruction("BIT 4 L", 2,8); BIT_B_R8(reg.hl.getLowByte(),4); break;
            case 0x66: currentInstruction = Instruction("BIT 4 HL", 2,12); BIT_B_HL(4); break;
            case 0x67: currentInstruction = Instruction("BIT 4 A", 2,8); BIT_B_R8(reg.af.getHighByte(),4); break;
            case 0x68: currentInstruction = Instruction("BIT 5 B", 2,8); BIT_B_R8(reg.bc.getHighByte(),5); break;
            case 0x69: currentInstruction = Instruction("BIT 5 C", 2,8); BIT_B_R8(reg.bc.getLowByte(),5); break;
            case 0x6A: currentInstruction = Instruction("BIT 5 D", 2,8); BIT_B_R8(reg.de.getHighByte(),5); break;
            case 0x6B: currentInstruction = Instruction("BIT 5 E", 2,8); BIT_B_R8(reg.de.getLowByte(),5); break;
            case 0x6C: currentInstruction = Instruction("BIT 5 H", 2,8); BIT_B_R8(reg.hl.getHighByte(),5); break;
            case 0x6D: currentInstruction = Instruction("BIT 5 L", 2,8); BIT_B_R8(reg.hl.getLowByte(),5); break;
            case 0x6E: currentInstruction = Instruction("BIT 5 HL", 2,12); BIT_B_HL(5); break;
            case 0x6F: currentInstruction = Instruction("BIT 5 A", 2,8); BIT_B_R8(reg.af.getHighByte(),5); break;
            case 0x70: currentInstruction = Instruction("BIT 6 B", 2,8); BIT_B_R8(reg.bc.getHighByte(),6); break;
            case 0x71: currentInstruction = Instruction("BIT 6 C", 2,8); BIT_B_R8(reg.bc.getLowByte(),6); break;
            case 0x72: currentInstruction = Instruction("BIT 6 D", 2,8); BIT_B_R8(reg.de.getHighByte(),6); break;
            case 0x73: currentInstruction = Instruction("BIT 6 E", 2,8); BIT_B_R8(reg.de.getLowByte(),6); break;
            case 0x74: currentInstruction = Instruction("BIT 6 H", 2,8); BIT_B_R8(reg.hl.getHighByte(),6); break;
            case 0x75: currentInstruction = Instruction("BIT 6 L", 2,8); BIT_B_R8(reg.hl.getLowByte(),6); break;
            case 0x76: currentInstruction = Instruction("BIT 6 HL", 2,12); BIT_B_HL(6); break;
            case 0x77: currentInstruction = Instruction("BIT 6 A", 2,8); BIT_B_R8(reg.af.getHighByte(),6); break;
            case 0x78: currentInstruction = Instruction("BIT 7 B", 2,8); BIT_B_R8(reg.bc.getHighByte(),7); break;
            case 0x79: currentInstruction = Instruction("BIT 7 C", 2,8); BIT_B_R8(reg.bc.getLowByte(),7); break;
            case 0x7A: currentInstruction = Instruction("BIT 7 D", 2,8); BIT_B_R8(reg.de.getHighByte(),7); break;
            case 0x7B: currentInstruction = Instruction("BIT 7 E", 2,8); BIT_B_R8(reg.de.getLowByte(),7); break;
            case 0x7C: currentInstruction = Instruction("BIT 7 H", 2,8); BIT_B_R8(reg.hl.getHighByte(),7); break;
            case 0x7D: currentInstruction = Instruction("BIT 7 L", 2,8); BIT_B_R8(reg.hl.getLowByte(),7); break;
            case 0x7E: currentInstruction = Instruction("BIT 7 HL", 2,12); BIT_B_HL(7); break;
            case 0x7F: currentInstruction = Instruction("BIT 7 A", 2,8); BIT_B_R8(reg.af.getHighByte(),7); break;
            case 0x80: currentInstruction = Instruction("RES 0 B", 2,8); RES_B_R8(reg.bc.getHighByte(), 0); break;
            case 0x81: currentInstruction = Instruction("RES 0 C", 2,8); RES_B_R8(reg.bc.getLowByte(), 0); break;
            case 0x82: currentInstruction = Instruction("RES 0 D", 2,8); RES_B_R8(reg.de.getHighByte(), 0); break;
            case 0x83: currentInstruction = Instruction("RES 0 E", 2,8); RES_B_R8(reg.de.getLowByte(), 0); break;
            case 0x84: currentInstruction = Instruction("RES 0 H", 2,8); RES_B_R8(reg.hl.getHighByte(), 0); break;
            case 0x85: currentInstruction = Instruction("RES 0 L", 2,8); RES_B_R8(reg.hl.getLowByte(), 0); break;
            case 0x86: currentInstruction = Instruction("RES 0 HL", 2,16); RES_B_HL(0); break;
            case 0x87: currentInstruction = Instruction("RES 0 A", 2,8); RES_B_R8(reg.af.getHighByte(), 0); break;
            case 0x88: currentInstruction = Instruction("RES 1 B", 2,8); RES_B_R8(reg.bc.getHighByte(), 1); break;
            case 0x89: currentInstruction = Instruction("RES 1 C", 2,8); RES_B_R8(reg.bc.getLowByte(), 1); break;
            case 0x8A: currentInstruction = Instruction("RES 1 D", 2,8); RES_B_R8(reg.de.getHighByte(), 1); break;
            case 0x8B: currentInstruction = Instruction("RES 1 E", 2,8); RES_B_R8(reg.de.getLowByte(), 1); break;
            case 0x8C: currentInstruction = Instruction("RES 1 H", 2,8); RES_B_R8(reg.hl.getHighByte(), 1); break;
            case 0x8D: currentInstruction = Instruction("RES 1 L", 2,8); RES_B_R8(reg.hl.getLowByte(), 1); break;
            case 0x8E: currentInstruction = Instruction("RES 1 HL", 2,16); RES_B_HL(1); break;
            case 0x8F: currentInstruction = Instruction("RES 1 A", 2,8); RES_B_R8(reg.af.getHighByte(), 1); break;
            case 0x90: currentInstruction = Instruction("RES 2 B", 2,8); RES_B_R8(reg.bc.getHighByte(), 2); break;
            case 0x91: currentInstruction = Instruction("RES 2 C", 2,8); RES_B_R8(reg.bc.getLowByte(), 2); break;
            case 0x92: currentInstruction = Instruction("RES 2 D", 2,8); RES_B_R8(reg.de.getHighByte(), 2); break;
            case 0x93: currentInstruction = Instruction("RES 2 E", 2,8); RES_B_R8(reg.de.getLowByte(), 2); break;
            case 0x94: currentInstruction = Instruction("RES 2 H", 2,8); RES_B_R8(reg.hl.getHighByte(), 2); break;
            case 0x95: currentInstruction = Instruction("RES 2 L", 2,8); RES_B_R8(reg.hl.getLowByte(), 2); break;
            case 0x96: currentInstruction = Instruction("RES 2 HL", 2,16); RES_B_HL(2); break;
            case 0x97: currentInstruction = Instruction("RES 2 A", 2,8); RES_B_R8(reg.af.getHighByte(), 2); break;
            case 0x98: currentInstruction = Instruction("RES 3 B", 2,8); RES_B_R8(reg.bc.getHighByte(), 3); break;
            case 0x99: currentInstruction = Instruction("RES 3 C", 2,8); RES_B_R8(reg.bc.getLowByte(), 3); break;
            case 0x9A: currentInstruction = Instruction("RES 3 D", 2,8); RES_B_R8(reg.de.getHighByte(), 3); break;
            case 0x9B: currentInstruction = Instruction("RES 3 E", 2,8); RES_B_R8(reg.de.getLowByte(), 3); break;
            case 0x9C: currentInstruction = Instruction("RES 3 H", 2,8); RES_B_R8(reg.hl.getHighByte(), 3); break;
            case 0x9D: currentInstruction = Instruction("RES 3 L", 2,8); RES_B_R8(reg.hl.getLowByte(), 3); break;
            case 0x9E: currentInstruction = Instruction("RES 3 HL", 2,16); RES_B_HL(3); break;
            case 0x9F: currentInstruction = Instruction("RES 3 A", 2,8); RES_B_R8(reg.af.getHighByte(), 3); break;
            case 0xA0: currentInstruction = Instruction("RES 4 B", 2,8); RES_B_R8(reg.bc.getHighByte(), 4); break;
            case 0xA1: currentInstruction = Instruction("RES 4 C", 2,8); RES_B_R8(reg.bc.getLowByte(), 4); break;
            case 0xA2: currentInstruction = Instruction("RES 4 D", 2,8); RES_B_R8(reg.de.getHighByte(), 4); break;
            case 0xA3: currentInstruction = Instruction("RES 4 E", 2,8); RES_B_R8(reg.de.getLowByte(), 4); break;
            case 0xA4: currentInstruction = Instruction("RES 4 H", 2,8); RES_B_R8(reg.hl.getHighByte(), 4); break;
            case 0xA5: currentInstruction = Instruction("RES 4 L", 2,8); RES_B_R8(reg.hl.getLowByte(), 4); break;
            case 0xA6: currentInstruction = Instruction("RES 4 HL", 2,16); RES_B_HL(4); break;
            case 0xA7: currentInstruction = Instruction("RES 4 A", 2,8); RES_B_R8(reg.af.getHighByte(), 4); break;
            case 0xA8: currentInstruction = Instruction("RES 5 B", 2,8); RES_B_R8(reg.bc.getHighByte(), 5); break;
            case 0xA9: currentInstruction = Instruction("RES 5 C", 2,8); RES_B_R8(reg.bc.getLowByte(), 5); break;
            case 0xAA: currentInstruction = Instruction("RES 5 D", 2,8); RES_B_R8(reg.de.getHighByte(), 5); break;
            case 0xAB: currentInstruction = Instruction("RES 5 E", 2,8); RES_B_R8(reg.de.getLowByte(), 5); break;
            case 0xAC: currentInstruction = Instruction("RES 5 H", 2,8); RES_B_R8(reg.hl.getHighByte(), 5); break;
            case 0xAD: currentInstruction = Instruction("RES 5 L", 2,8); RES_B_R8(reg.hl.getLowByte(), 5); break;
            case 0xAE: currentInstruction = Instruction("RES 5 HL", 2,16); RES_B_HL(5); break;
            case 0xAF: currentInstruction = Instruction("RES 5 A", 2,8); RES_B_R8(reg.af.getHighByte(), 5); break;
            case 0xB0: currentInstruction = Instruction("RES 6 B", 2,8); RES_B_R8(reg.bc.getHighByte(), 6); break;
            case 0xB1: currentInstruction = Instruction("RES 6 C", 2,8); RES_B_R8(reg.bc.getLowByte(), 6); break;
            case 0xB2: currentInstruction = Instruction("RES 6 D", 2,8); RES_B_R8(reg.de.getHighByte(), 6); break;
            case 0xB3: currentInstruction = Instruction("RES 6 E", 2,8); RES_B_R8(reg.de.getLowByte(), 6); break;
            case 0xB4: currentInstruction = Instruction("RES 6 H", 2,8); RES_B_R8(reg.hl.getHighByte(), 6); break;
            case 0xB5: currentInstruction = Instruction("RES 6 L", 2,8); RES_B_R8(reg.hl.getLowByte(), 6); break;
            case 0xB6: currentInstruction = Instruction("RES 6 HL", 2,16); RES_B_HL(6); break;
            case 0xB7: currentInstruction = Instruction("RES 6 A", 2,8); RES_B_R8(reg.af.getHighByte(), 6); break;
            case 0xB8: currentInstruction = Instruction("RES 7 B", 2,8); RES_B_R8(reg.bc.getHighByte(), 7); break;
            case 0xB9: currentInstruction = Instruction("RES 7 C", 2,8); RES_B_R8(reg.bc.getLowByte(), 7); break;
            case 0xBA: currentInstruction = Instruction("RES 7 D", 2,8); RES_B_R8(reg.de.getHighByte(), 7); break;
            case 0xBB: currentInstruction = Instruction("RES 7 E", 2,8); RES_B_R8(reg.de.getLowByte(), 7); break;
            case 0xBC: currentInstruction = Instruction("RES 7 H", 2,8); RES_B_R8(reg.hl.getHighByte(), 7); break;
            case 0xBD: currentInstruction = Instruction("RES 7 L", 2,8); RES_B_R8(reg.hl.getLowByte(), 7); break;
            case 0xBE: currentInstruction = Instruction("RES 7 HL", 2,16); RES_B_HL(7); break;
            case 0xBF: currentInstruction = Instruction("RES 7 A", 2,8); RES_B_R8(reg.af.getHighByte(), 7); break;
            case 0xC0: currentInstruction = Instruction("SET 0 B", 2,8); SET_B_R8(reg.bc.getHighByte(), 0); break;
            case 0xC1: currentInstruction = Instruction("SET 0 C", 2,8); SET_B_R8(reg.bc.getLowByte(), 0); break;
            case 0xC2: currentInstruction = Instruction("SET 0 D", 2,8); SET_B_R8(reg.de.getHighByte(), 0); break;
            case 0xC3: currentInstruction = Instruction("SET 0 E", 2,8); SET_B_R8(reg.de.getLowByte(), 0); break;
            case 0xC4: currentInstruction = Instruction("SET 0 H", 2,8); SET_B_R8(reg.hl.getHighByte(), 0); break;
            case 0xC5: currentInstruction = Instruction("SET 0 L", 2,8); SET_B_R8(reg.hl.getLowByte(), 0); break;
            case 0xC6: currentInstruction = Instruction("SET 0 HL", 2,16); SET_B_HL(0); break;
            case 0xC7: currentInstruction = Instruction("SET 0 A", 2,8); SET_B_R8(reg.af.getHighByte(), 0); break;
            case 0xC8: currentInstruction = Instruction("SET 1 B", 2,8); SET_B_R8(reg.bc.getHighByte(), 1); break;
            case 0xC9: currentInstruction = Instruction("SET 1 C", 2,8); SET_B_R8(reg.bc.getLowByte(), 1); break;
            case 0xCA: currentInstruction = Instruction("SET 1 D", 2,8); SET_B_R8(reg.de.getHighByte(), 1); break;
            case 0xCB: currentInstruction = Instruction("SET 1 E", 2,8); SET_B_R8(reg.de.getLowByte(), 1); break;
            case 0xCC: currentInstruction = Instruction("SET 1 H", 2,8); SET_B_R8(reg.hl.getHighByte(), 1); break;
            case 0xCD: currentInstruction = Instruction("SET 1 L", 2,8); SET_B_R8(reg.hl.getLowByte(), 1); break;
            case 0xCE: currentInstruction = Instruction("SET 1 HL", 2,16); SET_B_HL(1); break;
            case 0xCF: currentInstruction = Instruction("SET 1 A", 2,8); SET_B_R8(reg.af.getHighByte(), 1); break;
            case 0xD0: currentInstruction = Instruction("SET 2 B", 2,8); SET_B_R8(reg.bc.getHighByte(), 2); break;
            case 0xD1: currentInstruction = Instruction("SET 2 C", 2,8); SET_B_R8(reg.bc.getLowByte(), 2); break;
            case 0xD2: currentInstruction = Instruction("SET 2 D", 2,8); SET_B_R8(reg.de.getHighByte(), 2); break;
            case 0xD3: currentInstruction = Instruction("SET 2 E", 2,8); SET_B_R8(reg.de.getLowByte(), 2); break;
            case 0xD4: currentInstruction = Instruction("SET 2 H", 2,8); SET_B_R8(reg.hl.getHighByte(), 2); break;
            case 0xD5: currentInstruction = Instruction("SET 2 L", 2,8); SET_B_R8(reg.hl.getLowByte(), 2); break;
            case 0xD6: currentInstruction = Instruction("SET 2 HL", 2,16); SET_B_HL(2); break;
            case 0xD7: currentInstruction = Instruction("SET 2 A", 2,8); SET_B_R8(reg.af.getHighByte(), 2); break;
            case 0xD8: currentInstruction = Instruction("SET 3 B", 2,8); SET_B_R8(reg.bc.getHighByte(), 3); break;
            case 0xD9: currentInstruction = Instruction("SET 3 C", 2,8); SET_B_R8(reg.bc.getLowByte(), 3); break;
            case 0xDA: currentInstruction = Instruction("SET 3 D", 2,8); SET_B_R8(reg.de.getHighByte(), 3); break;
            case 0xDB: currentInstruction = Instruction("SET 3 E", 2,8); SET_B_R8(reg.de.getLowByte(), 3); break;
            case 0xDC: currentInstruction = Instruction("SET 3 H", 2,8); SET_B_R8(reg.hl.getHighByte(), 3); break;
            case 0xDD: currentInstruction = Instruction("SET 3 L", 2,8); SET_B_R8(reg.hl.getLowByte(), 3); break;
            case 0xDE: currentInstruction = Instruction("SET 3 HL", 2,16); SET_B_HL(3); break;
            case 0xDF: currentInstruction = Instruction("SET 3 A", 2,8); SET_B_R8(reg.af.getHighByte(), 3); break;
            case 0xE0: currentInstruction = Instruction("SET 4 B", 2,8); SET_B_R8(reg.bc.getHighByte(), 4); break;
            case 0xE1: currentInstruction = Instruction("SET 4 C", 2,8); SET_B_R8(reg.bc.getLowByte(), 4); break;
            case 0xE2: currentInstruction = Instruction("SET 4 D", 2,8); SET_B_R8(reg.de.getHighByte(), 4); break;
            case 0xE3: currentInstruction = Instruction("SET 4 E", 2,8); SET_B_R8(reg.de.getLowByte(), 4); break;
            case 0xE4: currentInstruction = Instruction("SET 4 H", 2,8); SET_B_R8(reg.hl.getHighByte(), 4); break;
            case 0xE5: currentInstruction = Instruction("SET 4 L", 2,8); SET_B_R8(reg.hl.getLowByte(), 4); break;
            case 0xE6: currentInstruction = Instruction("SET 4 HL", 2,16); SET_B_HL(4); break;
            case 0xE7: currentInstruction = Instruction("SET 4 A", 2,8); SET_B_R8(reg.af.getHighByte(), 4); break;
            case 0xE8: currentInstruction = Instruction("SET 5 B", 2,8); SET_B_R8(reg.bc.getHighByte(), 5); break;
            case 0xE9: currentInstruction = Instruction("SET 5 C", 2,8); SET_B_R8(reg.bc.getLowByte(), 5); break;
            case 0xEA: currentInstruction = Instruction("SET 5 D", 2,8); SET_B_R8(reg.de.getHighByte(), 5); break;
            case 0xEB: currentInstruction = Instruction("SET 5 E", 2,8); SET_B_R8(reg.de.getLowByte(), 5); break;
            case 0xEC: currentInstruction = Instruction("SET 5 H", 2,8); SET_B_R8(reg.hl.getHighByte(), 5); break;
            case 0xED: currentInstruction = Instruction("SET 5 L", 2,8); SET_B_R8(reg.hl.getLowByte(), 5); break;
            case 0xEE: currentInstruction = Instruction("SET 5 HL", 2,16); SET_B_HL(5); break;
            case 0xEF: currentInstruction = Instruction("SET 5 A", 2,8); SET_B_R8(reg.af.getHighByte(), 5); break;
            case 0xF0: currentInstruction = Instruction("SET 6 B", 2,8); SET_B_R8(reg.bc.getHighByte(), 6); break;
            case 0xF1: currentInstruction = Instruction("SET 6 C", 2,8); SET_B_R8(reg.bc.getLowByte(), 6); break;
            case 0xF2: currentInstruction = Instruction("SET 6 D", 2,8); SET_B_R8(reg.de.getHighByte(), 6); break;
            case 0xF3: currentInstruction = Instruction("SET 6 E", 2,8); SET_B_R8(reg.de.getLowByte(), 6); break;
            case 0xF4: currentInstruction = Instruction("SET 6 H", 2,8); SET_B_R8(reg.hl.getHighByte(), 6); break;
            case 0xF5: currentInstruction = Instruction("SET 6 L", 2,8); SET_B_R8(reg.hl.getLowByte(), 6); break;
            case 0xF6: currentInstruction = Instruction("SET 6 HL", 2,16); SET_B_HL(6); break;
            case 0xF7: currentInstruction = Instruction("SET 6 A", 2,8); SET_B_R8(reg.af.getHighByte(), 6); break;
            case 0xF8: currentInstruction = Instruction("SET 7 B", 2,8); SET_B_R8(reg.bc.getHighByte(), 7); break;
            case 0xF9: currentInstruction = Instruction("SET 7 C", 2,8); SET_B_R8(reg.bc.getLowByte(), 7); break;
            case 0xFA: currentInstruction = Instruction("SET 7 D", 2,8); SET_B_R8(reg.de.getHighByte(), 7); break;
            case 0xFB: currentInstruction = Instruction("SET 7 E", 2,8); SET_B_R8(reg.de.getLowByte(), 7); break;
            case 0xFC: currentInstruction = Instruction("SET 7 H", 2,8); SET_B_R8(reg.hl.getHighByte(), 7); break;
            case 0xFD: currentInstruction = Instruction("SET 7 L", 2,8); SET_B_R8(reg.hl.getLowByte(), 7); break;
            case 0xFE: currentInstruction = Instruction("SET 7 HL", 2,16); SET_B_HL(7); break;
            case 0xFF: currentInstruction = Instruction("SET 7 A", 2,8); SET_B_R8(reg.af.getHighByte(), 7); break;
            default: /* Handle unknown opcode */ break;
        }
    }
    void cpu::executeInstruction()
    {
        switch (opcode)
        {
            case 0x00: currentInstruction = Instruction("NOP",1,4); NOP(); break;
            case 0x01: currentInstruction = Instruction("LD BC, U16", 3, 12); LD_R16_u16(reg.bc); break;
            case 0x02: currentInstruction = Instruction("LD BC, A",1,8); LD_BC_A(); break;
            case 0x03: currentInstruction = Instruction("INC BC",1,8); INC_R16(reg.bc); break;
            case 0x04: currentInstruction = Instruction("INC B",1,4); INC_R8(reg.bc.getHighByte()); break;
            case 0x05: currentInstruction = Instruction("DEC B",1,4); DEC_R8(reg.bc.getHighByte()); break;
            case 0x06: currentInstruction = Instruction("LD B, u8",2,8); LD_R8_u8(reg.bc.getHighByte()); break;
            case 0x07: currentInstruction = Instruction("RLCA", 1, 4); RLCA(); break;
            case 0x08: currentInstruction = Instruction("LD (u16), SP", 3, 20); LD_u16_SP(); break;
            case 0x09: currentInstruction = Instruction("ADD HL, BC", 1, 8); ADD_HL_R16(reg.bc); break;
            case 0x0A: currentInstruction = Instruction("LD A, (BC)", 1, 8); LD_A_BC(); break;
            case 0x0B: currentInstruction = Instruction("DEC BC", 1, 8); DEC_R16(reg.bc); break;
            case 0x0C: currentInstruction = Instruction("INC C", 1, 4); INC_R8(reg.bc.getLowByte()); break;
            case 0x0D: currentInstruction = Instruction("DEC C", 1, 4); DEC_R8(reg.bc.getLowByte()); break;
            case 0x0E: currentInstruction = Instruction("LD C, u8", 2, 8); LD_R8_u8(reg.bc.getLowByte()); break;
            case 0x0F: currentInstruction = Instruction("RRCA", 1, 4); RRCA(); break;
            case 0x10: currentInstruction = Instruction("STOP", 2, 4); STOP(); break;
            case 0x11: currentInstruction = Instruction("LD DE, u16", 3, 12); LD_R16_u16(reg.de); break;
            case 0x12: currentInstruction = Instruction("LD (DE), A", 1, 8); LD_DE_A(); break;
            case 0x13: currentInstruction = Instruction("INC DE", 1, 8); INC_R16(reg.de); break;
            case 0x14: currentInstruction = Instruction("INC D", 1, 4); INC_R8(reg.de.getHighByte()); break;
            case 0x15: currentInstruction = Instruction("DEC D", 1, 4); DEC_R8(reg.de.getHighByte()); break;
            case 0x16: currentInstruction = Instruction("LD D, u8", 2, 8); LD_R8_u8(reg.de.getHighByte()); break;
            case 0x17: currentInstruction = Instruction("RLA", 1, 4); RLA(); break;
            case 0x18: currentInstruction = Instruction("JR e", 2, 12); JR_E(); break;
            case 0x19: currentInstruction = Instruction("ADD HL, DE", 1, 8); ADD_HL_R16(reg.de); break;
            case 0x1A: currentInstruction = Instruction("LD A, (DE)", 1, 8); LD_A_DE(); break;
            case 0x1B: currentInstruction = Instruction("DEC DE", 1, 8); DEC_R16(reg.de); break;
            case 0x1C: currentInstruction = Instruction("INC E", 1, 4); INC_R8(reg.de.getLowByte()); break;
            case 0x1D: currentInstruction = Instruction("DEC E", 1, 4); DEC_R8(reg.de.getLowByte()); break;
            case 0x1E: currentInstruction = Instruction("LD E, u8", 2, 8); LD_R8_u8(reg.de.getLowByte()); break;
            case 0x1F: currentInstruction = Instruction("RRA", 1, 4); RRA(); break;
            case 0x20: currentInstruction = Instruction("JR NZ, e", 2, 8); JR_CC_E(!getFlag(Z)); break;
            case 0x21: currentInstruction = Instruction("LD HL, u16", 3, 12); LD_R16_u16(reg.hl); break;
            case 0x22: currentInstruction = Instruction("LD (HL+), A", 1, 8); LD_HL_INC_A(); break;
            case 0x23: currentInstruction = Instruction("INC HL", 1, 8); INC_R16(reg.hl); break;
            case 0x24: currentInstruction = Instruction("INC H", 1, 4); INC_R8(reg.hl.getHighByte()); break;
            case 0x25: currentInstruction = Instruction("DEC H", 1, 4); DEC_R8(reg.hl.getHighByte()); break;
            case 0x26: currentInstruction = Instruction("LD H, u8", 2, 8); LD_R8_u8(reg.hl.getHighByte()); break;
            case 0x27: currentInstruction = Instruction("DAA", 1, 4); DAA(); break;
            case 0x28: currentInstruction = Instruction("JR Z, e", 2, 8); JR_CC_E(getFlag(Z)); break;
            case 0x29: currentInstruction = Instruction("ADD HL, HL", 1, 8); ADD_HL_R16(reg.hl); break;
            case 0x2A: currentInstruction = Instruction("LD A, (HL+)", 1, 8); LD_A_HL_INC(); break;
            case 0x2B: currentInstruction = Instruction("DEC HL", 1, 8); DEC_R16(reg.hl); break;
            case 0x2C: currentInstruction = Instruction("INC L", 1, 4); INC_R8(reg.hl.getLowByte()); break;
            case 0x2D: currentInstruction = Instruction("DEC L", 1, 4); DEC_R8(reg.hl.getLowByte()); break;
            case 0x2E: currentInstruction = Instruction("LD L, u8", 2, 8); LD_R8_u8(reg.hl.getLowByte()); break;
            case 0x2F: currentInstruction = Instruction("CPL", 1, 4); CPL(); break;
            case 0x30: currentInstruction = Instruction("JR NC, e", 2, 8); JR_CC_E(!getFlag(C)); break;
            case 0x31: currentInstruction = Instruction("LD SP, u16", 3, 12); LD_R16_u16(reg.sp); break;
            case 0x32: currentInstruction = Instruction("LD (HL-), A", 1, 8); LD_HL_DEC_A(); break;
            case 0x33: currentInstruction = Instruction("INC SP", 1, 8); INC_R16(reg.sp); break;
            case 0x34: currentInstruction = Instruction("INC (HL)", 1, 12); INC_HL(); break;
            case 0x35: currentInstruction = Instruction("DEC (HL)", 1, 12); DEC_HL(); break;
            case 0x36: currentInstruction = Instruction("LD (HL), u8", 2, 12); LD_HL_u8(); break;
            case 0x37: currentInstruction = Instruction("SCF", 1, 4); SCF(); break;
            case 0x38: currentInstruction = Instruction("JR C, e", 2, 8); JR_CC_E(getFlag(C)); break;
            case 0x39: currentInstruction = Instruction("ADD HL, SP", 1, 8); ADD_HL_R16(reg.sp); break;
            case 0x3A: currentInstruction = Instruction("LD A, (HL-)", 1, 8); LD_A_HL_DEC(); break;
            case 0x3B: currentInstruction = Instruction("DEC SP", 1, 8); DEC_R16(reg.sp); break;
            case 0x3C: currentInstruction = Instruction("INC A", 1, 4); INC_R8(reg.af.getHighByte()); break;
            case 0x3D: currentInstruction = Instruction("DEC A", 1, 4); DEC_R8(reg.af.getHighByte()); break;
            case 0x3E: currentInstruction = Instruction("LD A, u8", 2, 8); LD_R8_u8(reg.af.getHighByte()); break;
            case 0x3F: currentInstruction = Instruction("CCF", 1, 4); CCF(); break;
            case 0x40: currentInstruction = Instruction("LD B, B", 1, 4); LD_R8_R8(reg.bc.getHighByte(), reg.bc.getHighByte()); break;
            case 0x41: currentInstruction = Instruction("LD B, C", 1, 4); LD_R8_R8(reg.bc.getHighByte(), reg.bc.getLowByte()); break;
            case 0x42: currentInstruction = Instruction("LD B, D", 1, 4); LD_R8_R8(reg.bc.getHighByte(), reg.de.getHighByte()); break;
            case 0x43: currentInstruction = Instruction("LD B, E", 1, 4); LD_R8_R8(reg.bc.getHighByte(), reg.de.getLowByte()); break;
            case 0x44: currentInstruction = Instruction("LD B, H", 1, 4); LD_R8_R8(reg.bc.getHighByte(), reg.hl.getHighByte()); break;
            case 0x45: currentInstruction = Instruction("LD B, L", 1, 4); LD_R8_R8(reg.bc.getHighByte(), reg.hl.getLowByte()); break;
            case 0x46: currentInstruction = Instruction("LD B, (HL)", 1, 8); LD_R8_HL(reg.bc.getHighByte()); break;
            case 0x47: currentInstruction = Instruction("LD B, A", 1, 4); LD_R8_R8(reg.bc.getHighByte(), reg.af.getHighByte()); break;
            case 0x48: currentInstruction = Instruction("LD C, B", 1, 4); LD_R8_R8(reg.bc.getLowByte(), reg.bc.getHighByte()); break;
            case 0x49: currentInstruction = Instruction("LD C, C", 1, 4); LD_R8_R8(reg.bc.getLowByte(), reg.bc.getLowByte()); break;
            case 0x4A: currentInstruction = Instruction("LD C, D", 1, 4); LD_R8_R8(reg.bc.getLowByte(), reg.de.getHighByte()); break;
            case 0x4B: currentInstruction = Instruction("LD C, E", 1, 4); LD_R8_R8(reg.bc.getLowByte(), reg.de.getLowByte()); break;
            case 0x4C: currentInstruction = Instruction("LD C, H", 1, 4); LD_R8_R8(reg.bc.getLowByte(), reg.hl.getHighByte()); break;
            case 0x4D: currentInstruction = Instruction("LD C, L", 1, 4); LD_R8_R8(reg.bc.getLowByte(), reg.hl.getLowByte()); break;
            case 0x4E: currentInstruction = Instruction("LD C, (HL)", 1, 8); LD_R8_HL(reg.bc.getLowByte()); break;
            case 0x4F: currentInstruction = Instruction("LD C, A", 1, 4); LD_R8_R8(reg.bc.getLowByte(), reg.af.getHighByte()); break;
            case 0x50: currentInstruction = Instruction("LD D, B", 1, 4); LD_R8_R8(reg.de.getHighByte(), reg.bc.getHighByte()); break;
            case 0x51: currentInstruction = Instruction("LD D, C", 1, 4); LD_R8_R8(reg.de.getHighByte(), reg.bc.getLowByte()); break;
            case 0x52: currentInstruction = Instruction("LD D, D", 1, 4); LD_R8_R8(reg.de.getHighByte(), reg.de.getHighByte()); break;
            case 0x53: currentInstruction = Instruction("LD D, E", 1, 4); LD_R8_R8(reg.de.getHighByte(), reg.de.getLowByte()); break;
            case 0x54: currentInstruction = Instruction("LD D, H", 1, 4); LD_R8_R8(reg.de.getHighByte(), reg.hl.getHighByte()); break;
            case 0x55: currentInstruction = Instruction("LD D, L", 1, 4); LD_R8_R8(reg.de.getHighByte(), reg.hl.getLowByte()); break;
            case 0x56: currentInstruction = Instruction("LD D, (HL)", 1, 8); LD_R8_HL(reg.de.getHighByte()); break;
            case 0x57: currentInstruction = Instruction("LD D, A", 1, 4); LD_R8_R8(reg.de.getHighByte(), reg.af.getHighByte()); break;
            case 0x58: currentInstruction = Instruction("LD E, B", 1, 4); LD_R8_R8(reg.de.getLowByte(), reg.bc.getHighByte()); break;
            case 0x59: currentInstruction = Instruction("LD E, C", 1, 4); LD_R8_R8(reg.de.getLowByte(), reg.bc.getLowByte()); break;
            case 0x5A: currentInstruction = Instruction("LD E, D", 1, 4); LD_R8_R8(reg.de.getLowByte(), reg.de.getHighByte()); break;
            case 0x5B: currentInstruction = Instruction("LD E, E", 1, 4); LD_R8_R8(reg.de.getLowByte(), reg.de.getLowByte()); break;
            case 0x5C: currentInstruction = Instruction("LD E, H", 1, 4); LD_R8_R8(reg.de.getLowByte(), reg.hl.getHighByte()); break;
            case 0x5D: currentInstruction = Instruction("LD E, L", 1, 4); LD_R8_R8(reg.de.getLowByte(), reg.hl.getLowByte()); break;
            case 0x5E: currentInstruction = Instruction("LD E, (HL)", 1, 8); LD_R8_HL(reg.de.getLowByte()); break;
            case 0x5F: currentInstruction = Instruction("LD E, A", 1, 4); LD_R8_R8(reg.de.getLowByte(), reg.af.getHighByte()); break;
            case 0x60: currentInstruction = Instruction("LD H, B", 1, 4); LD_R8_R8(reg.hl.getHighByte(), reg.bc.getHighByte()); break;
            case 0x61: currentInstruction = Instruction("LD H, C", 1, 4); LD_R8_R8(reg.hl.getHighByte(), reg.bc.getLowByte()); break;
            case 0x62: currentInstruction = Instruction("LD H, D", 1, 4); LD_R8_R8(reg.hl.getHighByte(), reg.de.getHighByte()); break;
            case 0x63: currentInstruction = Instruction("LD H, E", 1, 4); LD_R8_R8(reg.hl.getHighByte(), reg.de.getLowByte()); break;
            case 0x64: currentInstruction = Instruction("LD H, H", 1, 4); LD_R8_R8(reg.hl.getHighByte(), reg.hl.getHighByte()); break;
            case 0x65: currentInstruction = Instruction("LD H, L", 1, 4); LD_R8_R8(reg.hl.getHighByte(), reg.hl.getLowByte()); break;
            case 0x66: currentInstruction = Instruction("LD H, (HL)", 1, 8); LD_R8_HL(reg.hl.getHighByte()); break;
            case 0x67: currentInstruction = Instruction("LD H, A", 1, 4); LD_R8_R8(reg.hl.getHighByte(), reg.af.getHighByte()); break;
            case 0x68: currentInstruction = Instruction("LD L, B", 1, 4); LD_R8_R8(reg.hl.getLowByte(), reg.bc.getHighByte()); break;
            case 0x69: currentInstruction = Instruction("LD L, C", 1, 4); LD_R8_R8(reg.hl.getLowByte(), reg.bc.getLowByte()); break;
            case 0x6A: currentInstruction = Instruction("LD L, D", 1, 4); LD_R8_R8(reg.hl.getLowByte(), reg.de.getHighByte()); break;
            case 0x6B: currentInstruction = Instruction("LD L, E", 1, 4); LD_R8_R8(reg.hl.getLowByte(), reg.de.getLowByte()); break;
            case 0x6C: currentInstruction = Instruction("LD L, H", 1, 4); LD_R8_R8(reg.hl.getLowByte(), reg.hl.getHighByte()); break;
            case 0x6D: currentInstruction = Instruction("LD L, L", 1, 4); LD_R8_R8(reg.hl.getLowByte(), reg.hl.getLowByte()); break;
            case 0x6E: currentInstruction = Instruction("LD L, (HL)", 1, 8); LD_R8_HL(reg.hl.getLowByte()); break;
            case 0x6F: currentInstruction = Instruction("LD L, A", 1, 4); LD_R8_R8(reg.hl.getLowByte(), reg.af.getHighByte()); break;
            case 0x70: currentInstruction = Instruction("LD (HL), B", 1, 8); LD_HL_R8(reg.bc.getHighByte()); break;
            case 0x71: currentInstruction = Instruction("LD (HL), C", 1, 8); LD_HL_R8(reg.bc.getLowByte()); break;
            case 0x72: currentInstruction = Instruction("LD (HL), D", 1, 8); LD_HL_R8(reg.de.getHighByte()); break;
            case 0x73: currentInstruction = Instruction("LD (HL), E", 1, 8); LD_HL_R8(reg.de.getLowByte()); break;
            case 0x74: currentInstruction = Instruction("LD (HL), H", 1, 8); LD_HL_R8(reg.hl.getHighByte()); break;
            case 0x75: currentInstruction = Instruction("LD (HL), L", 1, 8); LD_HL_R8(reg.hl.getLowByte()); break;
            case 0x76: currentInstruction = Instruction("HALT", 1, 4); HALT(); break;
            case 0x77: currentInstruction = Instruction("LD (HL), A", 1, 8); LD_HL_R8(reg.af.getHighByte()); break;
            case 0x78: currentInstruction = Instruction("LD A, B", 1, 4); LD_R8_R8(reg.af.getHighByte(), reg.bc.getHighByte()); break;
            case 0x79: currentInstruction = Instruction("LD A, C", 1, 4); LD_R8_R8(reg.af.getHighByte(), reg.bc.getLowByte()); break;
            case 0x7A: currentInstruction = Instruction("LD A, D", 1, 4); LD_R8_R8(reg.af.getHighByte(), reg.de.getHighByte()); break;
            case 0x7B: currentInstruction = Instruction("LD A, E", 1, 4); LD_R8_R8(reg.af.getHighByte(), reg.de.getLowByte()); break;
            case 0x7C: currentInstruction = Instruction("LD A, H", 1, 4); LD_R8_R8(reg.af.getHighByte(), reg.hl.getHighByte()); break;
            case 0x7D: currentInstruction = Instruction("LD A, L", 1, 4); LD_R8_R8(reg.af.getHighByte(), reg.hl.getLowByte()); break;
            case 0x7E: currentInstruction = Instruction("LD A, (HL)", 1, 8); LD_R8_HL(reg.af.getHighByte()); break;
            case 0x7F: currentInstruction = Instruction("LD A, A", 1, 4); LD_R8_R8(reg.af.getHighByte(), reg.af.getHighByte()); break;
            case 0x80: currentInstruction = Instruction("ADD A, B", 1, 4); ADD_R8(reg.bc.getHighByte()); break;
            case 0x81: currentInstruction = Instruction("ADD A, C", 1, 4); ADD_R8(reg.bc.getLowByte()); break;
            case 0x82: currentInstruction = Instruction("ADD A, D", 1, 4); ADD_R8(reg.de.getHighByte()); break;
            case 0x83: currentInstruction = Instruction("ADD A, E", 1, 4); ADD_R8(reg.de.getLowByte()); break;
            case 0x84: currentInstruction = Instruction("ADD A, H", 1, 4); ADD_R8(reg.hl.getHighByte()); break;
            case 0x85: currentInstruction = Instruction("ADD A, L", 1, 4); ADD_R8(reg.hl.getLowByte()); break;
            case 0x86: currentInstruction = Instruction("ADD A, (HL)", 1, 8); ADD_HL(); break;
            case 0x87: currentInstruction = Instruction("ADD A, A", 1, 4); ADD_R8(reg.af.getHighByte()); break;
            case 0x88: currentInstruction = Instruction("ADC A, B", 1, 4); ADC_R8(reg.bc.getHighByte()); break;
            case 0x89: currentInstruction = Instruction("ADC A, C", 1, 4); ADC_R8(reg.bc.getLowByte()); break;
            case 0x8A: currentInstruction = Instruction("ADC A, D", 1, 4); ADC_R8(reg.de.getHighByte()); break;
            case 0x8B: currentInstruction = Instruction("ADC A, E", 1, 4); ADC_R8(reg.de.getLowByte()); break;
            case 0x8C: currentInstruction = Instruction("ADC A, H", 1, 4); ADC_R8(reg.hl.getHighByte()); break;
            case 0x8D: currentInstruction = Instruction("ADC A, L", 1, 4); ADC_R8(reg.hl.getLowByte()); break;
            case 0x8E: currentInstruction = Instruction("ADC A, (HL)", 1, 8); ADC_HL(); break;
            case 0x8F: currentInstruction = Instruction("ADC A, A", 1, 4); ADC_R8(reg.af.getHighByte()); break;
            case 0x90: currentInstruction = Instruction("SUB B", 1, 4); SUB_R8(reg.bc.getHighByte()); break;
            case 0x91: currentInstruction = Instruction("SUB C", 1, 4); SUB_R8(reg.bc.getLowByte()); break;
            case 0x92: currentInstruction = Instruction("SUB D", 1, 4); SUB_R8(reg.de.getHighByte()); break;
            case 0x93: currentInstruction = Instruction("SUB E", 1, 4); SUB_R8(reg.de.getLowByte()); break;
            case 0x94: currentInstruction = Instruction("SUB H", 1, 4); SUB_R8(reg.hl.getHighByte()); break;
            case 0x95: currentInstruction = Instruction("SUB L", 1, 4); SUB_R8(reg.hl.getLowByte()); break;
            case 0x96: currentInstruction = Instruction("SUB (HL)", 1, 8); SUB_HL(); break;
            case 0x97: currentInstruction = Instruction("SUB A", 1, 4); SUB_R8(reg.af.getHighByte()); break;
            case 0x98: currentInstruction = Instruction("SBC A, B", 1, 4); SBC_R8(reg.bc.getHighByte()); break;
            case 0x99: currentInstruction = Instruction("SBC A, C", 1, 4); SBC_R8(reg.bc.getLowByte()); break;
            case 0x9A: currentInstruction = Instruction("SBC A, D", 1, 4); SBC_R8(reg.de.getHighByte()); break;
            case 0x9B: currentInstruction = Instruction("SBC A, E", 1, 4); SBC_R8(reg.de.getLowByte()); break;
            case 0x9C: currentInstruction = Instruction("SBC A, H", 1, 4); SBC_R8(reg.hl.getHighByte()); break;
            case 0x9D: currentInstruction = Instruction("SBC A, L", 1, 4); SBC_R8(reg.hl.getLowByte()); break;
            case 0x9E: currentInstruction = Instruction("SBC A, (HL)", 1, 8); SBC_HL(); break;
            case 0x9F: currentInstruction = Instruction("SBC A, A", 1, 4); SBC_R8(reg.af.getHighByte()); break;
            case 0xA0: currentInstruction = Instruction("AND B", 1, 4); AND_R8(reg.bc.getHighByte()); break;
            case 0xA1: currentInstruction = Instruction("AND C", 1, 4); AND_R8(reg.bc.getLowByte()); break;
            case 0xA2: currentInstruction = Instruction("AND D", 1, 4); AND_R8(reg.de.getHighByte()); break;
            case 0xA3: currentInstruction = Instruction("AND E", 1, 4); AND_R8(reg.de.getLowByte()); break;
            case 0xA4: currentInstruction = Instruction("AND H", 1, 4); AND_R8(reg.hl.getHighByte()); break;
            case 0xA5: currentInstruction = Instruction("AND L", 1, 4); AND_R8(reg.hl.getLowByte()); break;
            case 0xA6: currentInstruction = Instruction("AND (HL)", 1, 8); AND_HL(); break;
            case 0xA7: currentInstruction = Instruction("AND A", 1, 4); AND_R8(reg.af.getHighByte()); break;
            case 0xA8: currentInstruction = Instruction("XOR B", 1, 4); XOR_R8(reg.bc.getHighByte()); break;
            case 0xA9: currentInstruction = Instruction("XOR C", 1, 4); XOR_R8(reg.bc.getLowByte()); break;
            case 0xAA: currentInstruction = Instruction("XOR D", 1, 4); XOR_R8(reg.de.getHighByte()); break;
            case 0xAB: currentInstruction = Instruction("XOR E", 1, 4); XOR_R8(reg.de.getLowByte()); break;
            case 0xAC: currentInstruction = Instruction("XOR H", 1, 4); XOR_R8(reg.hl.getHighByte()); break;
            case 0xAD: currentInstruction = Instruction("XOR L", 1, 4); XOR_R8(reg.hl.getLowByte()); break;
            case 0xAE: currentInstruction = Instruction("XOR (HL)", 1, 8); XOR_HL(); break;
            case 0xAF: currentInstruction = Instruction("XOR A", 1, 4); XOR_R8(reg.af.getHighByte()); break;
            case 0xB0: currentInstruction = Instruction("OR B", 1, 4); OR_R8(reg.bc.getHighByte()); break;
            case 0xB1: currentInstruction = Instruction("OR C", 1, 4); OR_R8(reg.bc.getLowByte()); break;
            case 0xB2: currentInstruction = Instruction("OR D", 1, 4); OR_R8(reg.de.getHighByte()); break;
            case 0xB3: currentInstruction = Instruction("OR E", 1, 4); OR_R8(reg.de.getLowByte()); break;
            case 0xB4: currentInstruction = Instruction("OR H", 1, 4); OR_R8(reg.hl.getHighByte()); break;
            case 0xB5: currentInstruction = Instruction("OR L", 1, 4); OR_R8(reg.hl.getLowByte()); break;
            case 0xB6: currentInstruction = Instruction("OR (HL)", 1, 8); OR_HL(); break;
            case 0xB7: currentInstruction = Instruction("OR A", 1, 4); OR_R8(reg.af.getHighByte()); break;
            case 0xB8: currentInstruction = Instruction("CP B", 1, 4); CP_R8(reg.bc.getHighByte()); break;
            case 0xB9: currentInstruction = Instruction("CP C", 1, 4); CP_R8(reg.bc.getLowByte()); break;
            case 0xBA: currentInstruction = Instruction("CP D", 1, 4); CP_R8(reg.de.getHighByte()); break;
            case 0xBB: currentInstruction = Instruction("CP E", 1, 4); CP_R8(reg.de.getLowByte()); break;
            case 0xBC: currentInstruction = Instruction("CP H", 1, 4); CP_R8(reg.hl.getHighByte()); break;
            case 0xBD: currentInstruction = Instruction("CP L", 1, 4); CP_R8(reg.hl.getLowByte()); break;
            case 0xBE: currentInstruction = Instruction("CP (HL)", 1, 8); CP_HL(); break;
            case 0xBF: currentInstruction = Instruction("CP A", 1, 4); CP_R8(reg.af.getHighByte()); break;
            case 0xC0: currentInstruction = Instruction("RET NZ", 1, 8); RET_CC(!getFlag(Z)); break;
            case 0xC1: currentInstruction = Instruction("POP BC", 1, 12); POP_R16(reg.bc); break;
            case 0xC2: currentInstruction = Instruction("JP NZ, u16", 3, 12); JP_CC_u16(!getFlag(Z)); break;
            case 0xC3: currentInstruction = Instruction("JP u16", 3, 16); JP_u16(); break;
            case 0xC4: currentInstruction = Instruction("CALL NZ, u16", 3, 12); CALL_CC_u16(!getFlag(Z)); break;
            case 0xC5: currentInstruction = Instruction("PUSH BC", 1, 16); PUSH_R16(reg.bc); break;
            case 0xC6: currentInstruction = Instruction("ADD A, u8", 2, 8); ADD_u8(); break;
            case 0xC7: currentInstruction = Instruction("RST 00H", 1, 16); RST_u8(0x00); break;
            case 0xC8: currentInstruction = Instruction("RET Z", 1, 8); RET_CC(getFlag(Z)); break;
            case 0xC9: currentInstruction = Instruction("RET", 1, 16); RET(); break;
            case 0xCA: currentInstruction = Instruction("JP Z, u16", 3, 12); JP_CC_u16(getFlag(Z)); break;
            case 0xCB: /*  */ break;
            case 0xCC: currentInstruction = Instruction("CALL Z, u16", 3, 12); CALL_CC_u16(getFlag(Z)); break;
            case 0xCD: currentInstruction = Instruction("CALL u16", 3, 24); CALL_u16(); break;
            case 0xCE: currentInstruction = Instruction("ADC A, u8", 2, 8); ADC_u8(); break;
            case 0xCF: currentInstruction = Instruction("RST 08H", 1, 16); RST_u8(0x08); break;
            case 0xD0: currentInstruction = Instruction("RET NC", 1, 8); RET_CC(!getFlag(C)); break;
            case 0xD1: currentInstruction = Instruction("POP DE", 1, 12); POP_R16(reg.de); break;
            case 0xD2: currentInstruction = Instruction("JP NC, u16", 3, 12); JP_CC_u16(!getFlag(C)); break;
            case 0xD3: /* Implement instruction */ break;
            case 0xD4: currentInstruction = Instruction("CALL NC, u16", 3, 12); CALL_CC_u16(!getFlag(C)); break;
            case 0xD5: currentInstruction = Instruction("PUSH DE", 1, 16); PUSH_R16(reg.de); break;
            case 0xD6: currentInstruction = Instruction("SUB u8", 2, 8); SUB_u8(); break;
            case 0xD7: currentInstruction = Instruction("RST 10H", 1, 16); RST_u8(0x10); break;
            case 0xD8: currentInstruction = Instruction("RET C", 1, 8); RET_CC(getFlag(C)); break;
            case 0xD9: currentInstruction = Instruction("RETI", 1, 16); RETI(); break;
            case 0xDA: currentInstruction = Instruction("JP C, u16", 3, 12); JP_CC_u16(getFlag(C)); break;
            case 0xDB: /* Implement instruction */ break;
            case 0xDC: currentInstruction = Instruction("CALL C, u16", 3, 12); CALL_CC_u16(getFlag(C)); break;
            case 0xDD: /* Implement instruction */ break;
            case 0xDE: currentInstruction = Instruction("SBC A, u8", 2, 8); SBC_u8(); break;
            case 0xDF: currentInstruction = Instruction("RST 18H", 1, 16); RST_u8(0x18); break;
            case 0xE0: currentInstruction = Instruction("LDH (u8), A", 2, 12); LDH_u8_A(); break;
            case 0xE1: currentInstruction = Instruction("POP HL", 1, 12); POP_R16(reg.hl); break;
            case 0xE2: currentInstruction = Instruction("LDH (C), A", 1, 8); LDH_C_A(); break;
            case 0xE3: /* Implement instruction */ break;
            case 0xE4: /* Implement instruction */ break;
            case 0xE5: currentInstruction = Instruction("PUSH HL", 1, 16); PUSH_R16(reg.hl); break;
            case 0xE6: currentInstruction = Instruction("AND u8", 2, 8); AND_u8(); break;
            case 0xE7: currentInstruction = Instruction("RST 20H", 1, 16); RST_u8(0x20); break;
            case 0xE8: currentInstruction = Instruction("ADD SP, e", 2, 16); ADD_SP_E(); break;
            case 0xE9: currentInstruction = Instruction("JP (HL)", 1, 4); JP_HL(); break;
            case 0xEA: currentInstruction = Instruction("LD (u16), A", 3, 16); LD_u16_A(); break;
            case 0xEB: /* Implement instruction */ break;
            case 0xEC: /* Implement instruction */ break;
            case 0xED: /* Implement instruction */ break;
            case 0xEE: currentInstruction = Instruction("XOR u8", 2, 8); XOR_u8(); break;
            case 0xEF: currentInstruction = Instruction("RST 28H", 1, 16); RST_u8(0x28); break;
            case 0xF0: currentInstruction = Instruction("LDH A, (u8)", 2, 12); LDH_A_u8(); break;
            case 0xF1: currentInstruction = Instruction("POP AF", 1, 12); POP_AF(); break;
            case 0xF2: currentInstruction = Instruction("LDH A, (C)", 1, 12); LDH_A_C(); break;
            case 0xF3: currentInstruction = Instruction("DI", 1, 4); DI(); break;
            case 0xF4: /* Implement instruction */ break;
            case 0xF5: currentInstruction = Instruction("PUSH AF", 1, 16); PUSH_AF(); break;
            case 0xF6: currentInstruction = Instruction("OR u8", 2, 8); OR_u8(); break;
            case 0xF7: currentInstruction = Instruction("RST 30H", 1, 16); RST_u8(0x30); break;
            case 0xF8: currentInstruction = Instruction("LD HL, SP+e", 2, 12); LD_HL_SP_E(); break;
            case 0xF9: currentInstruction = Instruction("LD SP, HL", 1, 8); LD_SP_HL(); break;
            case 0xFA: currentInstruction = Instruction("LD A, (u16)", 3, 16); LD_A_u16(); break;
            case 0xFB: currentInstruction = Instruction("EI", 1, 4); EI(); break;
            case 0xFC: /* Implement instruction */ break;
            case 0xFD: /* Implement instruction */ break;
            case 0xFE: currentInstruction = Instruction("CP u8", 2, 8); CP_u8(); break;
            case 0xFF: currentInstruction = Instruction("RST 38H", 1, 16); RST_u8(0x38); break;
            default: isHalted = true; break;
        }

    };
    

    // Load Data from one 8 Bit register to another
    void cpu::LD_R8_R8(Register8Bit &RegDest, Register8Bit &RegSource)
    {
        RegDest.write(RegSource.read());
    }

    // load a 8 bit value into a 8 bit register
    void cpu::LD_R8_u8(Register8Bit &RegDest)
    {
        u8 n = read_memory(reg.pc.read());
        RegDest.write(n);
    }

    // load a 8 bit value from adress HL into a 8 bit register
    void cpu::LD_R8_HL(Register8Bit &RegDest)
    {
        RegDest.write(read_memory( reg.hl.read() ));
    }

    // load data from an 8 bit register to value at adress HL
    void cpu::LD_HL_R8(Register8Bit RegSource)
    {
        write_memory(reg.hl.read() , RegSource.read());
    }

    // load a u8 value to the adress specified in HL
    void cpu::LD_HL_u8()
    {
        u8 n = read_memory(reg.pc.read());
        write_memory(reg.hl.read(), n);
    }

    void cpu::LD_A_BC()
    {
        u8 n = read_memory(reg.bc.read());
        reg.af.getHighByte().write(n);
    }

    void cpu::LD_A_DE()
    {
        u8 n = read_memory(reg.de.read());
        reg.af.getHighByte().write(n);        
    }

    void cpu::LD_BC_A()
    {
        write_memory(reg.bc.read(), reg.af.getHighByte().read());
    }

    void cpu::LD_DE_A()
    {
        write_memory(reg.de.read(), reg.af.getHighByte().read());       
    }

    void cpu::LD_A_u16()
    {
        u8 n = read_memory(fetch16Bit());
        reg.af.getHighByte().write(n);
    }

    void cpu::LD_u16_A()
    {
        u16 addr = fetch16Bit();
        write_memory(addr, reg.af.getHighByte().read());
    }

    void cpu::LDH_A_C()
    {
        u8 n = read_memory(buildAdress(reg.bc.getLowByte().read(), 0xFF));
        reg.af.getHighByte().write(n);
    }
    
    void cpu::LDH_C_A()
    {
        write_memory(buildAdress(reg.bc.getLowByte().read(), 0xFF), reg.af.getHighByte().read());
    }

    void cpu::LDH_A_u8()
    {
        u8 n = read_memory(reg.pc.read());
        u8 A = read_memory(buildAdress(n,0xFF));
        reg.af.getHighByte().write(A);

    }

    void cpu::LDH_u8_A()
    {
        u8 n = read_memory(reg.pc.read());
        write_memory(buildAdress(n,0xFF), reg.af.getHighByte().read());
    }
    // go back to 36
    void cpu::LD_A_HL_DEC()
    {
        u8 n = read_memory(reg.hl.read());
        reg.hl.Decrement();
        reg.af.getHighByte().write(n);
    }

    void cpu::LD_HL_DEC_A()
    {
        write_memory(reg.hl.read(), reg.af.getHighByte().read());
        reg.hl.Decrement();
    }

    void cpu::LD_A_HL_INC()
    {
        u8 n = read_memory(reg.hl.read());
        reg.hl.Increment();
        reg.af.getHighByte().write(n);
    }

    void cpu::LD_HL_INC_A()
    {
        write_memory(reg.hl.read(), reg.af.getHighByte().read());
        reg.hl.Increment();
    }

    void cpu::LD_R16_u16(Register16Bit& RegDest)
    {
        u16 nn = fetch16Bit();
        RegDest.write(nn);
    }

    void cpu::LD_u16_SP()
    {
        u16 nn = fetch16Bit();
        write_memory(nn,reg.sp.getLowByte().read());
        write_memory(nn + 1,reg.sp.getHighByte().read());
    }

    void cpu::LD_SP_HL()
    {
        // requires an extra cycle 
        reg.sp.write(reg.hl.read());
        Emu->ClockCycle(1);
    }

    void cpu::PUSH_AF()
    {
        reg.sp.Decrement();
        // Requires Extra Cycle
        Emu->ClockCycle(1);
        write_memory(reg.sp.read(), reg.af.getHighByte().read());
        reg.sp.Decrement();
        write_memory(reg.sp.read(), reg.af.getLowByte().read() & 0xF0); 
    }

    void cpu::PUSH_R16(Register16Bit& RegSource)
    {
        reg.sp.Decrement();
        // requires an extra cycle
        Emu->ClockCycle(1);
        write_memory(reg.sp.read(), RegSource.getHighByte().read());
        reg.sp.Decrement();
        write_memory(reg.sp.read(), RegSource.getLowByte().read());
    }

    void cpu::POP_AF()
    {
        reg.af.getLowByte().write(read_memory(reg.sp.read()) & 0xF0);
        reg.sp.Increment();
        reg.af.getHighByte().write(read_memory(reg.sp.read()));
        reg.sp.Increment(); 
    }

    void cpu::POP_R16(Register16Bit& RegDest)
    {
        RegDest.getLowByte().write(read_memory(reg.sp.read()));
        reg.sp.Increment();
        RegDest.getHighByte().write(read_memory(reg.sp.read()));
        reg.sp.Increment();

    }

    void cpu::LD_HL_SP_E()
    {
        Sint8 e = read_memory(reg.pc.read());
        u16 result = reg.sp.read() + e;

        // Set Flags
        setFlags(
        0,
        0, 
        HalfCarry8Bit(reg.sp.read()&0xFF, e, false), 
        Carry8Bit(reg.sp.read()&0xFF, e, false)
        );
        //Requires an extra cycle
        Emu->ClockCycle(1);

        reg.hl.write(result);
    }

    void cpu::ADD_R8(Register8Bit& RegSource)
    {
        u8 result = reg.af.getHighByte().read() + RegSource.read();
        setFlags(
        result == 0,
        0, 
        HalfCarry8Bit(reg.af.getHighByte().read(), RegSource.read(), false), 
        Carry8Bit(reg.af.getHighByte().read(), RegSource.read(), false)
        );
        reg.af.getHighByte().write(result);
    }

    void cpu::ADD_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = reg.af.getHighByte().read() + n;

        setFlags(
        result == 0,
        0, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, false), 
        Carry8Bit(reg.af.getHighByte().read(), n, false)
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::ADD_u8()
    {
        u8 n = read_memory(reg.pc.read());

        u8 result = reg.af.getHighByte().read() + n;

        setFlags(
        result == 0,
        0, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, false), 
        Carry8Bit(reg.af.getHighByte().read(), n, false)
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::ADC_R8(Register8Bit& RegSource)
    {
        u8 result = reg.af.getHighByte().read() + RegSource.read() + getFlag(C);
        setFlags(
        result == 0,
        0, 
        HalfCarry8Bit(reg.af.getHighByte().read(), RegSource.read(), false) || HalfCarry8Bit(reg.af.getHighByte().read() + RegSource.read(), getFlag(C), false),  
        Carry8Bit(reg.af.getHighByte().read(), RegSource.read(), false) ||  Carry8Bit(reg.af.getHighByte().read() + RegSource.read(), getFlag(C), false)
        );
        reg.af.getHighByte().write(result);
    }

    void cpu::ADC_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = reg.af.getHighByte().read() + n + getFlag(C);

        setFlags(
        result == 0,
        0, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, false) || HalfCarry8Bit(reg.af.getHighByte().read() + n, getFlag(C), false), 
        Carry8Bit(reg.af.getHighByte().read(), n, false) || Carry8Bit(reg.af.getHighByte().read() + n, getFlag(C), false )
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::ADC_u8()
    {
        u8 n = read_memory(reg.pc.read());

        u8 result = reg.af.getHighByte().read() + n + getFlag(C);

        setFlags(
        result == 0,
        0, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n , false) || HalfCarry8Bit(reg.af.getHighByte().read() + n, getFlag(C), false), 
        Carry8Bit(reg.af.getHighByte().read(), n , false) || Carry8Bit(reg.af.getHighByte().read() + n, getFlag(C), false)
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::SUB_R8(Register8Bit& RegSource)
    {
        u8 result = reg.af.getHighByte().read() - RegSource.read();
        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), RegSource.read(), true), 
        Carry8Bit(reg.af.getHighByte().read(), RegSource.read(), true)
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::SUB_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = reg.af.getHighByte().read() - n;

        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, true), 
        Carry8Bit(reg.af.getHighByte().read(), n, true)
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::SUB_u8()
    {
        u8 n = read_memory(reg.pc.read());

        u8 result = reg.af.getHighByte().read() - n;

        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, true), 
        Carry8Bit(reg.af.getHighByte().read(), n, true)
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::SBC_R8(Register8Bit& RegSource)
    {
        u8 result = reg.af.getHighByte().read() - RegSource.read() - getFlag(C);
        
        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), RegSource.read(), true) ||  HalfCarry8Bit(reg.af.getHighByte().read() - RegSource.read(), getFlag(C), true),
        Carry8Bit(reg.af.getHighByte().read(), RegSource.read(), true) || Carry8Bit(reg.af.getHighByte().read() - RegSource.read(), getFlag(C), true)
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::SBC_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = reg.af.getHighByte().read() - n - getFlag(C);

        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, true) || HalfCarry8Bit(reg.af.getHighByte().read() - n, getFlag(C), true), 
        Carry8Bit(reg.af.getHighByte().read(), n, true) || Carry8Bit(reg.af.getHighByte().read() - n, getFlag(C), true)
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::SBC_u8()
    {
        u8 n = read_memory(reg.pc.read());

        u8 result = reg.af.getHighByte().read() - n - getFlag(C);

        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, true) || HalfCarry8Bit(reg.af.getHighByte().read() - n, getFlag(C), true), 
        Carry8Bit(reg.af.getHighByte().read(), n, true) || Carry8Bit(reg.af.getHighByte().read() - n, getFlag(C), true)
        );

        reg.af.getHighByte().write(result);
    }

    // same as SUBR8 but does not update register A
    void cpu::CP_R8(Register8Bit& RegSource)
    {
        u8 result = reg.af.getHighByte().read() - RegSource.read();
        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), RegSource.read(), true), 
        Carry8Bit(reg.af.getHighByte().read(), RegSource.read(), true)
        );
    }

    void cpu::CP_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = reg.af.getHighByte().read() - n;

        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, true), 
        Carry8Bit(reg.af.getHighByte().read(), n, true)
        );
    }

    void cpu::CP_u8()
    {
        u8 n = read_memory(reg.pc.read());

        u8 result = reg.af.getHighByte().read() - n;

        setFlags(
        result == 0,
        1, 
        HalfCarry8Bit(reg.af.getHighByte().read(), n, true), 
        Carry8Bit(reg.af.getHighByte().read(), n, true)
        );
    }

    void cpu::INC_R8(Register8Bit& Reg8)
    {
        setFlags((u8)(Reg8.read()+1) == 0, 0, HalfCarry8Bit(Reg8.read(),1,false), getFlag(C));
        Reg8.Increment();
    }

    void cpu::INC_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = n + 1;
        setFlags(
            result == 0,
            0,
            HalfCarry8Bit(n,1,false),
            getFlag(C)
        );
        write_memory(reg.hl.read(), result);
    }

    void cpu::DEC_R8(Register8Bit& Reg8)
    {
        setFlags((u8)(Reg8.read()-1) == 0, 1, HalfCarry8Bit(Reg8.read(),1,true), getFlag(C));
        Reg8.Decrement();
    }

    void cpu::DEC_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = n - 1;
        setFlags(
            result == 0,
            1,
            HalfCarry8Bit(n,1,true),
            getFlag(C)
        );
        write_memory(reg.hl.read(), result);
    }

    void cpu::AND_R8(Register8Bit& RegSource)
    {
        u8 result = reg.af.getHighByte().read() & RegSource.read();

        setFlags(
        result == 0,
        0,
        1,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::AND_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = reg.af.getHighByte().read() & n;

        setFlags(
        result == 0,
        0,
        1,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::AND_u8()
    {
        u8 n = read_memory(reg.pc.read());
        u8 result = reg.af.getHighByte().read() & n;

        setFlags(
        result == 0,
        0,
        1,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::OR_R8(Register8Bit& RegSource)
    {
        u8 result = reg.af.getHighByte().read() | RegSource.read();
        
        setFlags(
        result == 0,
        0,
        0,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::OR_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = reg.af.getHighByte().read() | n;

        setFlags(
        result == 0,
        0,
        0,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::OR_u8()
    {
        u8 n = read_memory(reg.pc.read());
        u8 result = reg.af.getHighByte().read() | n;

        setFlags(
        result == 0,
        0,
        0,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::XOR_R8(Register8Bit& RegSource)
    {
        u8 result = reg.af.getHighByte().read() ^ RegSource.read();
        
        setFlags(
        result == 0,
        0,
        0,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::XOR_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 result = reg.af.getHighByte().read() ^ n;

        setFlags(
        result == 0,
        0,
        0,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::XOR_u8()
    {
        u8 n = read_memory(reg.pc.read());
        u8 result = reg.af.getHighByte().read() ^ n;

        setFlags(
        result == 0,
        0,
        0,
        0
        );

        reg.af.getHighByte().write(result);
    }

    void cpu::CCF()
    {
        setFlags(
            getFlag(Z),
            0,
            0,
            getFlag(C) ^ 1
        );
    }

    void cpu::SCF()
    {
        setFlags(
            getFlag(Z),
            0,
            0,
            1
        );
    }

    void cpu::DAA()
    {
        /* https://ehaskins.com/2018-01-30%20Z80%20DAA/ */
        uint8_t v = reg.af.getHighByte().read();
        uint8_t n = getFlag(N);
        uint8_t h = getFlag(H);
        uint8_t c = getFlag(C);
    
        uint8_t correction = 0;
    
        if (h || (!n && (v & 0xf) > 9)) {
            correction |= 0x6;
        }
    
        if (c || (!n && v > 0x99)) {
            correction |= 0x60;
            setFlags(getFlag(Z), n, h, 1);
        }
    
        v += n ? -correction : correction;
    
        reg.af.getHighByte().write(v);
        setFlags(v == 0, n, 0, getFlag(C));
          
    }

    void cpu::CPL()
    {
        u8 NotA = ~(reg.af.getHighByte().read());
        reg.af.getHighByte().write(NotA);

        setFlags(
            getFlag(Z),
            1,
            1,
            getFlag(C)
        );
    }

    void cpu::INC_R16(Register16Bit& Reg16)
    {
        Reg16.Increment();
        //Requires an extra cycle
        Emu->ClockCycle(1);
    }

    void cpu::DEC_R16(Register16Bit& Reg16)
    {
        Reg16.Decrement();
        //Requires an extra cycle
        Emu->ClockCycle(1);
    }

    void cpu::ADD_HL_R16(Register16Bit& Reg16)
    {
        u16 result = reg.hl.read() + Reg16.read();

        setFlags(
            getFlag(Z),
            0,
            HalfCarry16bit(reg.hl.read(), Reg16.read(),false),
            Carry16bit(reg.hl.read(), Reg16.read(),false)
        );

        reg.hl.write(result);
        //Requires an extra cycle
        Emu->ClockCycle(1);
    }

    void cpu::ADD_SP_E()
    {
        Sint8 e = read_memory(reg.pc.read());
        u16 result = reg.sp.read() + e;

        setFlags(
            0,
            0,
            HalfCarry8Bit(reg.sp.read() & 0xFF, e & 0xFF, false),
            Carry8Bit(reg.sp.read() & 0xFF, e & 0xFF, false)
        );
        //Requires an extra 2 cycles
        Emu->ClockCycle(2);

        reg.sp.write(result);
    }
 
    void cpu::RLCA()
    {
        u8 b7 = reg.af.getHighByte().readBit(7);
        u8 left_rotated_byte = reg.af.getHighByte().read() << 1 | b7;
        setFlags
        (
            0,
            0,
            0,
            b7
        );
        reg.af.getHighByte().write(left_rotated_byte);
    }

    void cpu::RR_R8(Register8Bit& RegSource)
    {
        u8 b0 = RegSource.readBit(0);
        u8 right_rotated_byte =  (getFlag(C) << 7) | (RegSource.read() >> 1);
        setFlags
        (
            right_rotated_byte == 0,
            0,
            0,
            b0
        );
        RegSource.write(right_rotated_byte);
    }

    void cpu:: RR_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 b0 = n&0x1;
        u8 right_rotated_byte =  (getFlag(C) << 7) | (n >> 1);
        setFlags
        (
            right_rotated_byte == 0,
            0,
            0,
            b0
        );
        write_memory(reg.hl.read(),right_rotated_byte);
    }

    void cpu::RRA()
    {
        u8 b0 = reg.af.getHighByte().readBit(0);
        u8 right_rotated_byte =  (getFlag(C) << 7) | (reg.af.getHighByte().read() >> 1);
        setFlags
        (
            0,
            0,
            0,
            b0
        );
        reg.af.getHighByte().write(right_rotated_byte); 
    }

    void cpu::RRC_R8(Register8Bit& RegSource)
    {
        u8 b0 = RegSource.readBit(0);
        u8 right_rotated_byte =  (b0 << 7) | (RegSource.read() >> 1);
        setFlags
        (
            right_rotated_byte == 0,
            0,
            0,
            b0
        );
        RegSource.write(right_rotated_byte);
    }

    void cpu:: RRC_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 b0 = n&0x1;
        u8 right_rotated_byte =  (b0 << 7) | (n >> 1);
        setFlags
        (
            right_rotated_byte == 0,
            0,
            0,
            b0
        );
        write_memory(reg.hl.read(),right_rotated_byte);
    }

    void cpu::RRCA()
    {
        u8 b0 = reg.af.getHighByte().readBit(0);
        u8 right_rotated_byte =  (b0 << 7) | (reg.af.getHighByte().read() >> 1);
        setFlags
        (
            0,
            0,
            0,
            b0
        );
        reg.af.getHighByte().write(right_rotated_byte);
    }

    void cpu::RL_R8(Register8Bit& RegSource)
    {
        u8 b7 = RegSource.readBit(7);
        u8 left_rotated_byte = RegSource.read() << 1 | getFlag(C);
        setFlags
        (
            left_rotated_byte == 0,
            0,
            0,
            b7
        );
        RegSource.write(left_rotated_byte);
    }

    void cpu::RL_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 b7 = (n & 0x80) >> 7;
        u8 left_rotated_byte = (n << 1) | getFlag(C);
        setFlags
        (
            left_rotated_byte == 0,
            0,
            0,
            b7
        );
        write_memory(reg.hl.read(), left_rotated_byte);
    }

    void cpu::RLA()
    {
        u8 b7 = reg.af.getHighByte().readBit(7);
        u8 left_rotated_byte = (reg.af.getHighByte().read() << 1) | getFlag(C);
        setFlags
        (
            0,
            0,
            0,
            b7
        );
        reg.af.getHighByte().write(left_rotated_byte);
    }

    void cpu::RLC_R8(Register8Bit& RegSource)
    {
        u8 b7 = RegSource.readBit(7);
        u8 left_rotated_byte = RegSource.read() << 1 | b7;
        setFlags
        (
            left_rotated_byte == 0,
            0,
            0,
            b7
        );
        RegSource.write(left_rotated_byte);
    }

    void cpu::RLC_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 b7 = (n >> 7) & 0x1;
        u8 left_rotated_byte = (n << 1) | b7;
        setFlags
        (
            left_rotated_byte == 0,
            0,
            0,
            b7
        );
        write_memory(reg.hl.read(), left_rotated_byte);
        
    }

    void cpu::SLA_R8(Register8Bit& RegSource)
    {
        u8 b7 = RegSource.readBit(7);
        u8 left_rotated_byte = (RegSource.read() << 1) & 0xFE;
        setFlags
        (
            left_rotated_byte == 0,
            0,
            0,
            b7
        );
        RegSource.write(left_rotated_byte);
    }

    void cpu::SLA_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 b7 = (n >> 7) & 0x1;
        u8 left_rotated_byte = (n << 1);
        setFlags
        (
            left_rotated_byte == 0,
            0,
            0,
            b7
        );
        write_memory(reg.hl.read(), left_rotated_byte);

    }

    void cpu::SRA_R8(Register8Bit& RegSource)
    {
        u8 b7 = RegSource.readBit(7) << 7;
        u8 b0 = RegSource.readBit(0);
        u8 right_rotated_byte = b7 | RegSource.read() >> 1;
        setFlags
        (
            right_rotated_byte == 0,
            0,
            0,
            b0
        );
        RegSource.write(right_rotated_byte);
    }

    void cpu::SRA_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 b7 = (n >> 7) & 0x1;
        u8 b0 = n & 0x01;
        u8 right_rotated_byte = (b7  << 7) | (n >> 1);

        setFlags
        (
            right_rotated_byte == 0,
            0,
            0,
            b0
        );
        write_memory(reg.hl.read(), right_rotated_byte);

    }


    void cpu::SRL_R8(Register8Bit& RegSource)
    {
        u8 b0 = RegSource.readBit(0);
        u8 right_rotated_byte = RegSource.read() >> 1;
        setFlags
        (
            right_rotated_byte == 0,
            0,
            0,
            b0
        );
        RegSource.write(right_rotated_byte);
    }

    void cpu::SRL_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 b0 = n&0x01;
        u8 right_rotated_byte = n >> 1;
        setFlags
        (
            right_rotated_byte == 0,
            0,
            0,
            b0
        );
        write_memory(reg.hl.read(), right_rotated_byte);

    }

    void cpu::SWAP_R8(Register8Bit& RegSource)
    {
        u8 temp = RegSource.read();
        RegSource.write(((temp&0x0F) << 4) | ((temp&0xF0) >> 4)); 
        setFlags(
            RegSource.read() == 0,
            0,
            0,
            0
        );
    }

    void cpu::SWAP_HL()
    {
        u8 n = read_memory(reg.hl.read());
        u8 swap = ((n&0x0F) << 4) | ((n&0xF0) >> 4);
        write_memory(reg.hl.read(), swap);
        setFlags(
            (swap == 0),
            0,
            0,
            0
        );
    }

    void cpu::BIT_B_R8(Register8Bit& RegSource, u8 bit)
    {
        setFlags(
            (RegSource.readBit(bit) == 0),
            0,
            1,
            getFlag(C)
        );
    }

    void cpu::BIT_B_HL(u8 bit)
    {
        u8 n = read_memory(reg.hl.read());
        setFlags
        (
            ((n >> bit)&0x01) == 0,
            0,
            1,
            getFlag(C)
        );
    }

    void cpu::RES_B_R8(Register8Bit& RegSource, u8 bit)
    {
        RegSource.setBit(bit,0);
    }

    void cpu::RES_B_HL(u8 bit)
    {
        u8 n = read_memory(reg.hl.read());
        n &= ~(1 << bit);
        write_memory(reg.hl.read(), n);
    }

    void cpu::SET_B_R8(Register8Bit& RegSource, u8 bit)
    {
        RegSource.setBit(bit,1);
    }

    void cpu::SET_B_HL(u8 bit)
    {
        u8 n = read_memory(reg.hl.read());
        n |= (1 << bit);
        write_memory(reg.hl.read(), n);
    }

    void cpu::JP_u16()
    {
        u16 adress = fetch16Bit();

        reg.pc.write(adress);
        //Requires an extra cycle
        Emu->ClockCycle(1);
    }

    void cpu::JP_HL()
    {
        reg.pc.write(reg.hl.read());
    }

    void cpu::JP_CC_u16(bool CC)
    {
        u16 adress = fetch16Bit();

        if (CC)
        {
            reg.pc.write(adress);
            //Requires an extra cycle
            Emu->ClockCycle(1);
        }
    }

    void cpu::JR_E()
    {
        Sint8 e = read_memory(reg.pc.read());
        reg.pc.write(reg.pc.read() + e);
        //Requires an extra cycle
        Emu->ClockCycle(1);
    }

    void cpu::JR_CC_E(bool CC)
    {
        Sint8 e = read_memory(reg.pc.read());

        if(CC)
        {
            reg.pc.write(reg.pc.read() + e);
            //Requires an extra cycle
            Emu->ClockCycle(1); 
        }
    }

    void cpu::CALL_u16()
    {
        u16 adress = fetch16Bit();
        reg.sp.Decrement();
        write_memory(reg.sp.read(), reg.pc.getHighByte().read());
        reg.sp.Decrement();
        write_memory(reg.sp.read(), reg.pc.getLowByte().read());
        reg.pc.write(adress);
        //Requires an extra cycle
        Emu->ClockCycle(1);

    }

    void cpu::CALL_CC_u16(bool CC)
    {
        u16 adress = fetch16Bit();
        if (CC)
        {
            reg.sp.Decrement();
            write_memory(reg.sp.read(), reg.pc.getHighByte().read());
            reg.sp.Decrement();
            write_memory(reg.sp.read(), reg.pc.getLowByte().read());
            reg.pc.write(adress);
            //Requires an extra cycle
            Emu->ClockCycle(1);
        }

    }

    void cpu::RET()
    {
        u8 lsb = read_memory(reg.sp.read());
        reg.sp.Increment();
        u8 msb = read_memory(reg.sp.read());
        reg.sp.Increment();

        reg.pc.write(buildAdress(lsb,msb));
        //Requires an extra cycle
        Emu->ClockCycle(1);
    }

    void cpu::RET_CC(bool CC)
    {
        if(CC)
        {

            u8 lsb = read_memory(reg.sp.read());
            reg.sp.Increment();
            u8 msb = read_memory(reg.sp.read());
            reg.sp.Increment();
            reg.pc.write(buildAdress(lsb,msb)); 
            //Requires an extra cycle
            Emu->ClockCycle(1);
        }
        //Requires an extra cycle
        Emu->ClockCycle(1);
    }

    void cpu::RETI()
    {
        u8 lsb = read_memory(reg.sp.read());
        reg.sp.Increment();
        u8 msb = read_memory(reg.sp.read());
        reg.sp.Increment();
        reg.pc.write(buildAdress(lsb,msb)); 
        //Requires an extra cycle
        Emu->ClockCycle(1);
        enablingIME = true;
    }

    void cpu::RST_u8(u8 n)
    {
        reg.sp.Decrement();
        write_memory(reg.sp.read(), reg.pc.getHighByte().read());
        reg.sp.Decrement();
        write_memory(reg.sp.read(), reg.pc.getLowByte().read());
        reg.pc.write(buildAdress(n,0x00));
        //Requires an extra cycle
        Emu->ClockCycle(1);
    }

    //Look more into this
    void cpu::HALT()
    {
        isHalted = true;
    }

    void cpu::STOP()
    {
        // Nothing
    }

    void cpu::DI()
    {
        IME = 0;
    }

    void cpu::EI()
    {
        enablingIME = true;
    }

    void cpu::NOP()
    {
        //nothing
    }
} 