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
        void connectCartridge(Emulator* emu);

        ~cart()
        {

            save();
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

        // Rom Banks
        u8 current_rom_bank = 1;

        // Ram Banks
        u8 ram_Banks[0x8000];
        u8 current_ram_bank = 0;
        bool ram_enabled;


        // Initialize banks depending on the ROM type
        void setupBanking();

        
        // - MBC1 Related Flags/Functions
        bool MBC1 = false;
        u8 modeFlag;

        void CalculateZeroBank();
        void CalculateHighBank();
        u8 ZeroBank;
        u8 HighBank;

        // - MBC2 Related Flags
        bool MBC2 = false;

        // - MBC3 Related Flags
        bool MBC3 = false;
        u8 currentRTCReg;
        bool isClockRegisterMapped = false;
        bool latchOccured = false;
        u8 prevInput;


        struct RTCRegs
        {
            u8 s; // Seconds Register
            u8 m; // Minutes Register
            u8 h; // hours register
            u8 DL; // Lower 8 bits of the Days Register
            u8 DH; // 0: Bit 8 of the DL Reg, 6: Timer Halt Bit, 7 Day Counter carry bit
        };

        RTCRegs RTC;
        RTCRegs RTCLatched;

        // For MBC3 there is an internal Real Time Clock that increments every 
        // 4194304 T-Cycles
        void ClockTick();

        //Load a ROM file
        bool load(char *filename);
        // Reset Rom File Data
        void freeRomData();
        //Read a byte from the ROM
        u8 read(u16 addr);
        //Write a byte to the ROM/RAM
        void write(u16 addr, u8 data);
        //Save External RAM to .sav File
        void save();
        //Reload External RAM from .sav File if it exists
        void reloadSave();

        // Bool to check if the a Cartridge was previously loaded
        bool cartridgeLoaded = false;
        // Current Rom Name Used By the GUI
        std::string CurrentRom = "";
        // Holds the Location of the Last Rom Used 
        std::string LastRomDir = "";
        // Holds the Current Directory Used By the GUI
        char CurrentDir[256] = "../../GBEmu/Roms/";
    };
}