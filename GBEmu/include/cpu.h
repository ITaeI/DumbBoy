#pragma once
#include "common.h"
#include <array>
#include <map>


namespace GBEmu
{
    class Emulator;

    class cpu
    {

        private:
        Emulator *Emu;
        std::array<u8,8192> ram;

        static std::map<u16, int> instructionSet;
        static std::map<u16, int> CBPrefixinstructionSet;

        struct registers
        {
            u8 a, b, c, d, e, h, l;
            u16 pc, sp;
        };


        // CPU Context
        registers reg;
        public:

        //current fetch
        u16 fetch_data;
        u16 mem_dest;
        u8 opcode;

        bool isHalted;
        bool stepping;


        u8 read(u16 addr);
        void write(u16 addr, u8 data);

        void connectCPU(Emulator* emu);


    };
}

