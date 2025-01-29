#include "common.h"
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include "cart.h"
#include "Emulator.h"

namespace GBEmu
{

    void cart::connectCartridge(Emulator *emu)
    {
        Emu = emu;
    }
    // read a byte from the ROM
    u8 cart::read(u16 addr)
    {
        return cart_romdata[addr];
    }

    // write a byte to the ROM

    u8 cart::write(u16 addr, u8 data)
    {
        return 0x00;
    }


    std::map<int, std::string> licenseeLookup = {
        {0x00, "None"},
        {0x01, "Nintendo Research & Development 1"},
        {0x08, "Capcom"},
        {0x13, "EA (Electronic Arts)"},
        {0x18, "Hudson Soft"},
        {0x19, "B-AI"},
        {0x20, "KSS"},
        {0x22, "Planning Office WADA"},
        {0x24, "PCM Complete"},
        {0x25, "San-X"},
        {0x28, "Kemco"},
        {0x29, "SETA Corporation"},
        {0x30, "Viacom"},
        {0x31, "Nintendo"},
        {0x32, "Bandai"},
        {0x33, "Ocean Software/Acclaim Entertainment"},
        {0x34, "Konami"},
        {0x35, "HectorSoft"},
        {0x37, "Taito"},
        {0x38, "Hudson Soft"},
        {0x39, "Banpresto"},
        {0x41, "Ubi Soft"},
        {0x42, "Atlus"}
    };

    std::map<int, std::string> romTypeLookup = {
        {0x00, "ROM ONLY"},
        {0x01, "MBC1"},
        {0x02, "MBC1+RAM"},
        {0x03, "MBC1+RAM+BATTERY"},
        {0x05, "MBC2"},
        {0x06, "MBC2+BATTERY"},
        {0x08, "ROM+RAM"},
        {0x09, "ROM+RAM+BATTERY"},
        {0x0B, "MMM01"},
        {0x0C, "MMM01+RAM"},
        {0x0D, "MMM01+RAM+BATTERY"},
        {0x0F, "MBC3+TIMER+BATTERY"},
        {0x10, "MBC3+TIMER+RAM+BATTERY"},
        {0x11, "MBC3"},
        {0x12, "MBC3+RAM"},
        {0x13, "MBC3+RAM+BATTERY"}
    };

    // Function to get the licensee name by code
    std::string cart::getLicenseeName(int code) {
        auto it = licenseeLookup.find(code);
        if (it != licenseeLookup.end()) {
            return it->second;
        } else {
            return "Unknown";
        }
    }

    std::string cart::getRomTypeName(int code) {
        auto it = romTypeLookup.find(code);
        if (it != romTypeLookup.end()) {
            return it->second;
        } else {
            return "Unknown";
        }
    }

    bool cart::cart_load(char *filename)
    {
        // Copy the filename to the cart context
        strcpy(cart_filename, filename);

        //Adde the location context to the name of the rom file
        std::string filepath = "../../GBEmu/Roms/" + std::string(filename);

        // Open the file and seek to the end
        std::ifstream file(filepath, std::ios::binary| std::ios::ate);
        if (!file.is_open())
        {
            return false;
        }

        std::cout << "Loading ROM: " << filename << std::endl;

        // Get the file size and seek back to the beginning
        //since we are at the end of the file we can get the size from the pointer location
        cart_rom_size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Allocate memory for the ROM
        cart_romdata = new u8[cart_rom_size];
        file.read(reinterpret_cast<char*>(cart_romdata), cart_rom_size);
        file.close();

        // Get the header location
        // the header is always at 0x100 in the ROM data
        header = reinterpret_cast<cart_header*>(cart_romdata + 0x100);
        header->title[15] = 0;

        // Print the header information
        std::cout << "Title: " << header->title << std::endl;
        std::cout << "Licensee: " << getLicenseeName(header->lic_code) << std::endl;
        std::cout << "ROM Type: " << getRomTypeName(header->cart_type) << std::endl;
        std::cout << "ROM Size: " << (32*1024*(1 << header->rom_size)) << " bytes" << std::endl;
        std::cout << "RAM Size: " << (int)(header->ram_size) << " bytes" << std::endl;
        std::cout << "Version: " << (int)header->version << std::endl;

        //Build Checksum
        u8 checksum = 0;
        for (u16 address = 0x0134; address <= 0x014C; address++) {
            checksum = checksum - cart_romdata[address] - 1;
        }

        // Print the checksum and check if it is correct
        if ((checksum & 0xFF) == header->checksum) {
            std::cout << "Checksum: OK" << std::endl;
            std::cout << "checksumActual: " << (int)header->checksum << std::endl;
            std::cout << "checksumCalc: " << (int)(checksum & 0xFF) << std::endl;
            return 0;
        } else {
            std::cout << "Checksum: FAIL" << std::endl;
            std::cout << "checksumActual: " << (int)header->checksum << std::endl;
            std::cout << "checksumCalc: " << (int)(checksum & 0xFF) << std::endl;
            return 1;
        }
        
    }

}