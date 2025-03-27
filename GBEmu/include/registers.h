#pragma once
#include"common.h"
namespace GBEmu
{

    class Register8Bit
    {
        private:
        u8 Reg8;

        public:
        void Increment();
        void Decrement();
        void write(u8 data);
        u8 read();
        u8 readBit(u8 bit_number);
        void setBit(u8 bit_number, bool state); 
    };
    class Register16Bit
    {
        private:
        Register8Bit HighByte;
        Register8Bit LowByte;

        public:
        void Increment();
        void Decrement();
        void write(u16 adress);
        u16 read();
        Register8Bit& getHighByte();
        Register8Bit& getLowByte();

        void setBit();
    };
}