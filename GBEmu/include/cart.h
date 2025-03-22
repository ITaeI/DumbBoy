#pragma once
#include "common.h"



namespace GBEmu
{

    class Emulator;

    class cart
    {
        private:
        //Get the licensee name by code
        std::string getRomTypeName(int code);
        //Get the ROM type name by code
        std::string getLicenseeName(int code);

        Emulator *Emu;

        public:

        ~cart()
        {
            std::cout << "Freeing Rom Data" << std::endl;
            freeRomData();

        };

        struct cart_header
        {
            u8 entry[4];
            u8 nintendo_logo[0x30];
            char title[16];
            u16 licensee_code;
            u8 sgb_flag;
            u8 cart_type;
            u8 rom_size;
            u8 ram_size;
            u8 dest_code;
            u8 lic_code;
            u8 version;
            u8 checksum;
            u16 global_checksum;
        };

        // Cart context
        char cart_filename[1024];
        u32 cart_rom_size;
        u8 *cart_romdata = nullptr;
        cart_header *header = nullptr;


        //Load a ROM file
        bool load(char *filename);
        // Reset Rom File Data
        void freeRomData();
        //Read a byte from the ROM
        u8 read(u16 addr);
        //Write a byte to the ROM
        void write(u16 addr, u8 data);

        void connectCartridge(Emulator* emu);

    };
}