#pragma once
#include "common.h"
#include <array>
#include <map>
#include "registers.h"


namespace GBEmu
{
    class Emulator;
    class Instruction
    {
        public:
        std::string mnemonic;
        int cycles;
        int length;

        Instruction() : mnemonic(""), length(0), cycles(0) {}
        Instruction(const std::string& m, int l, int c) : mnemonic(m), length(l), cycles(c){}

    };

    class cpu
    {

        private:
        Emulator *Emu;
        void executeInstruction();
        void executeCBInstruction();

        public:


        struct registers
        {
            Register16Bit af, bc, de, hl, pc, sp; 
        };
        // CPU Context
        registers reg;

        bool step();
        void init();

        // Flag Setting Logic - Z = Zero Flag, N = Subtraction, H = Half Carry, C = Carry
        void setFlags(bool Z, bool N, bool H, bool C);
        u8 getFlag(int FlagValue);

        bool HalfCarry8Bit(u8 n1, u8 n2,bool subtraction);
        bool Carry8Bit(u8 n1, u8 n2,bool subtraction);

        bool HalfCarry16bit(u16 n1, u16 n2,bool subtraction);
        bool Carry16bit(u16 n1, u16 n2,bool subtraction);

        //current fetch
        u8 fetch;
        u8 opcode;
        Instruction currentInstruction;

        // CPU State
        bool isHalted;
        int current_cycles;


        // Can Use the cpu to read and write data through the bus
        u8 read_memory(u16 addr);
        void write_memory(u16 addr, u8 data);
        u16 fetch16Bit();
        u16 buildAdress(u8 high, u8 low);

        // Connects the CPU to the Emulator
        void connectCPU(Emulator* emu);

        // Interrupts
        Register8Bit IE;

        // Interrupt Flags
        // 0: VBlank
        // 1: LCD
        // 2: Timer
        // 3: Serial
        // 4: Joypad
        Register8Bit IF;
        bool enablingIME;
        bool IME;
        bool HaltBug = false;

        void HandleInterrupts();

        // CGB Double Speed
        bool SwitchArmed;

        // Debug Stuff
        std::string DBG;
        // Instructions

        // Possible reorganize these to make it more modular
        // Load 8Bit Register into another 8Bit Register ()
        // void LD_R8_R8();
        // void LD_R8_u8();
        // void LD_R8_R16();
        // void LD_R8_u16();
        // void LD_R16_u8();
        // void LD_R16_R8();
        // void LD_R16_R16();
        // void LD_R16_u16();
        // void LD_ADDR_R8();
        void LD_R8_R8(Register8Bit &RegDest, Register8Bit &RegSource);
        void LD_R8_u8(Register8Bit &RegDest);
        void LD_R8_HL(Register8Bit &RegDest);
        void LD_HL_R8(Register8Bit RegSource);
        void LD_HL_u8();
        void LD_A_BC();
        void LD_A_DE();
        void LD_BC_A();
        void LD_DE_A();
        void LD_A_u16();
        void LD_u16_A();
        void LDH_A_C();
        void LDH_C_A();
        void LDH_A_u8();
        void LDH_u8_A();
        void LD_A_HL_DEC();
        void LD_HL_DEC_A();
        void LD_A_HL_INC();
        void LD_HL_INC_A();
        void LD_R16_u16(Register16Bit& RegDest);
        void LD_u16_SP();
        void LD_SP_HL();
        void PUSH_AF();
        void PUSH_R16(Register16Bit& RegSource);
        void POP_AF();
        void POP_R16(Register16Bit& RegDest);
        void LD_HL_SP_E();
        void ADD_R8(Register8Bit& RegSource);
        void ADD_HL();
        void ADD_u8();
        void ADC_R8(Register8Bit& RegSource);
        void ADC_HL();
        void ADC_u8();
        void SUB_R8(Register8Bit& RegSource);
        void SUB_HL();
        void SUB_u8();
        void SBC_R8(Register8Bit& RegSource);
        void SBC_HL();
        void SBC_u8();
        void CP_R8(Register8Bit& RegSource);
        void CP_HL();
        void CP_u8();
        void INC_R8(Register8Bit& Reg8);
        void INC_HL();
        void DEC_R8(Register8Bit& Reg8);
        void DEC_HL();
        void AND_R8(Register8Bit& RegSource);
        void AND_HL();
        void AND_u8();
        void OR_R8(Register8Bit& RegSource);
        void OR_HL();
        void OR_u8();
        void XOR_R8(Register8Bit& RegSource);
        void XOR_HL();
        void XOR_u8();
        void CCF();
        void SCF();
        void DAA();
        void CPL();
        void INC_R16(Register16Bit& reg16);
        void DEC_R16(Register16Bit& Reg16);
        void ADD_HL_R16(Register16Bit& Reg16);
        void ADD_SP_E();
        // Check gbx80(7) for explanation //
        void RLCA(); 
        void RRCA();
        void RLA();
        void RRA(); 
        void RLC_R8(Register8Bit& RegSource);
        void RLC_HL();
        void RRC_R8(Register8Bit& RegSource); 
        void RRC_HL(); 
        void RL_R8(Register8Bit& RegSource);
        void RL_HL();
        void RR_R8(Register8Bit& RegSource); 
        void RR_HL(); 
        void SLA_R8(Register8Bit& RegSource);
        void SLA_HL();
        void SRA_R8(Register8Bit& RegSource);
        void SRA_HL();
        void SWAP_R8(Register8Bit& RegSource);
        void SWAP_HL();
        void SRL_R8(Register8Bit& RegSource);
        void SRL_HL();
        void BIT_B_R8(Register8Bit& RegSource, u8 bit);
        void BIT_B_HL(u8 bit);
        void RES_B_R8(Register8Bit& RegSource, u8 bit);
        void RES_B_HL(u8 bit);
        void SET_B_R8(Register8Bit& RegSource, u8 bit);
        void SET_B_HL(u8 bit);
        // End of gbz80(7) explanation //

        void JP_u16();
        void JP_HL();
        void JP_CC_u16(bool CC);
        void JR_E();
        void JR_CC_E(bool CC);
        void CALL_u16();
        void CALL_CC_u16(bool CC);
        void RET();
        void RET_CC(bool CC);
        void RETI();
        void RST_u8(u8 n);

        //more gbz80(7)
        void HALT(); 
        void STOP();
        void DI();
        void EI();
        void NOP();







    };


    enum Flags{
        Z = 7,
        N = 6,
        H = 5,
        C = 4
    };

    enum INT_Flags{
        VBlank_Int = 0,
        LCD_Int = 1,
        Timer_Int = 2,
        Serial_Int = 3,
        Joypad_Int = 4
    };

}

