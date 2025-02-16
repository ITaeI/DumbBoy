#include "registers.h"
namespace GBEmu
{
    // *************************** 8 Bit Register *********************** //
    void Register8Bit::Increment()
    {
        Reg8++;
    }

    void Register8Bit::Decrement()
    {
        Reg8--;
    }

    void Register8Bit::write(u8 data)
    {
        Reg8 = data;
    }

    u8 Register8Bit::read()
    {
        return Reg8;
    }

    u8 Register8Bit::readBit(u8 bit_number)
    {
        return (Reg8 >> bit_number) & 0x01;
    }

    void Register8Bit::setBit(u8 bit_number, bool state)
    {   
        if (state)
            Reg8 |= (1 << bit_number);  
        else 
            Reg8 &= ~(1 << bit_number);
    }   


    // *************************** 16 Bit Register ********************** //   

    void Register16Bit::Increment()
    {
        write(read() + 1);
    }

    void Register16Bit::Decrement()
    {
        write(read() - 1);
    }

    void Register16Bit::write(u16 data)
    {
        HighByte.write((data & 0xFF00) >> 8);
        LowByte.write(data & 0x00FF);
    }

    u16 Register16Bit::read()
    {
        return (HighByte.read() << 8 | LowByte.read());
    }

    Register8Bit& Register16Bit::getHighByte()
    {
        return HighByte;
    }

    Register8Bit& Register16Bit::getLowByte()
    {
        return LowByte;
    }
}