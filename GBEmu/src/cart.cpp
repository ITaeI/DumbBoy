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

        if (MBC1)
        {
            CalculateHighBank();
            CalculateZeroBank();
            if (addr >= 0x0000 && addr <= 0x3FFF)
            {
                if(!modeFlag)
                {
                    return cart_romdata[addr];
                }
                else
                {
                    return cart_romdata[addr + ZeroBank * 0x4000];
                }
            }
            else if (addr >= 0x4000 && addr <= 0x7FFF)
            {
                return cart_romdata[addr - 0x4000 + (HighBank * 0x4000)];
            }
            else if (addr >= 0xA000 && addr <= 0xBFFF)
            {
                if (!ram_enabled)
                {
                    return 0xFF;
                }

                if(header->ram_size >= 0x03)
                {
                    if(modeFlag)
                    {
                        return ram_Banks[(addr - 0xA000) + current_ram_bank * 0x2000];
                    }
                    else
                    {
                        return ram_Banks[addr - 0xA000];
                    }
                }
                else
                {
                    return ram_Banks[(addr - 0xA000)];
                }
            }
        }
        else if (MBC2)
        {
            u16 temp_addr;
            if (addr >= 0x4000 && addr <= 0x7FFF)
            {
                temp_addr = addr - 0x4000;
                return cart_romdata[temp_addr + (current_rom_bank * 0x4000)];
            }
            else if (addr >= 0xA000 && addr <= 0xBFFF) // relook at it for mbc1
            {
                if(!ram_enabled)
                {
                    return 0xFF;
                }
    
                temp_addr = (addr - 0xA000) & 0x1FF; // only 9 bits are wired to External Ram
                return 0xF0 | ram_Banks[temp_addr + (current_ram_bank * 0x2000)] & 0xF; // 4 high bits always 1
            }
            else
            {
                return cart_romdata[addr];
            }
        }
        else if (MBC3)
        {

        }

        return cart_romdata[addr];
    }

    // write a byte to the ROM

    void cart::write(u16 addr, u8 data)
    {
        if (MBC1)
        {
            if (addr >= 0x0000 && addr <= 0x1FFF) // set ram enabled
            {
                if(data & 0xF == 0xA)
                {
                    ram_enabled = true;
                }
                else
                {
                    ram_enabled = false;
                }
            }
            else if (addr >= 0x2000 && addr <= 0x3FFF) // set rom bank
            {
                switch(header->rom_size)
                {
                    case 0x00: // 32 KiB (no banking)
                        current_rom_bank = 1;
                        break;
                    case 0x01: // 64 KiB (4 banks)
                        current_rom_bank = data & 0x03;
                        break;
                    case 0x02: // 128 KiB (8 banks)
                        current_rom_bank = data & 0x07;
                        break;
                    case 0x03: // 256 KiB (16 banks)
                        current_rom_bank = data & 0x0F;
                        break;
                    case 0x04: // 512 KiB (32 banks)
                        current_rom_bank = data & 0x1F;
                        break;
                    case 0x05: // 1 MiB (64 banks)
                        current_rom_bank = data & 0x1F;
                        break;
                    case 0x06: // 2 MiB (128 banks)
                        current_rom_bank = data & 0x1F;
                        break;
                    default:
                        current_rom_bank = 1; // Fallback to bank 1
                        break;

                }
            }
            else if(addr >= 0x4000 && addr <= 0x5FFF)
            {
                // change the last two bits to the last two bits of written data
                current_ram_bank = (current_ram_bank & 0xFC) | (data & 0x03);
            }
            else if(addr >= 0x6000 && addr <= 0x7FFF)
            {
                modeFlag |= (data &0x1); // set the mode flag to the lowest bit of written data
            }
            else if (addr >= 0xA000 && addr <= 0xBFFF)
            {
                if(!ram_enabled)
                {
                    return;
                }

                if(header->ram_size >= 0x03)
                {
                    if(modeFlag)
                    {
                        ram_Banks[(addr - 0xA000) + current_ram_bank * 0x2000] = data;
                    }
                    else
                    {
                        ram_Banks[addr - 0xA000] = data;
                    }
                }
                else
                {
                    ram_Banks[(addr - 0xA000)] = data;
                }
            }
        }
        else if(MBC2)
        {
            if (addr >= 0x0000 && addr <= 0x3FFF) // set ram enabled
            {   
                // Check 8th bit and if lower nible == 0xA
                if(!(addr >> 8 & 1))
                {
                    if (addr & 0x0F == 0xA)
                    {
                         ram_enabled = true; 
                    }
                    else 
                    {
                        ram_enabled = false;
                    }
                }
                else // Set which rom bank we are using
                {
                    if (data != 0)
                    {
                        current_rom_bank = data & 0xF; 
                    }
                    else
                    {
                        current_rom_bank = 1;
                    }

                }

            }
            else if (addr >= 0xA000 && addr <= 0xBFFF) // Write to external RAM
            {
                if(!ram_enabled)
                {
                    return; 
                }

                ram_Banks[(addr - 0xA000) & 0x1FF] = data;
            }
        }

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

    void cart::freeRomData()
    {
        if(cart_romdata != nullptr)
        {
            delete[] cart_romdata;
            cart_romdata = nullptr; // Reset pointer to avoid dangling reference
        }
    }

    void cart::setupBanking()
    {
        // Check to see which Memory bank controller the rom is using;
        if(header->cart_type >= 0x01 && header->cart_type <= 0x03)
        {
            MBC1 = true;
        }
        else if(header->cart_type >= 0x05 && header->cart_type <= 0x06)
        {
            MBC2 = true;
        }
        else if(header->cart_type >= 0x0F && header->cart_type <= 0x13)
        {
            MBC3 = true;
        }

        // Initalize which rom bank we are pointing to (always one at the beginning)
        current_rom_bank = 1;

        // initalize ram banks (whether we are using them or not)
        memset(ram_Banks, 0, 0x8000 * sizeof(u8));
        current_ram_bank = 0;
        ram_enabled = false;

        // For MBC1
        modeFlag = 0;
    }

    void cart::CalculateZeroBank()
    {
        if(header->rom_size < 0x05)
        {
            ZeroBank = 0;
        }
        else if(header->rom_size == 0x5)
        {
            ZeroBank = (current_ram_bank & 0x1) << 5;

            // If MBC1m is used the entire ram bank number gets place in bits 4 and 5
        }
        else if(header->rom_size == 0x6)
        {
            ZeroBank = (current_ram_bank & 0x3) << 5;
        }
    }

    void cart::CalculateHighBank()
    {
        if(header->rom_size < 0x05)
        {
            HighBank = current_rom_bank;
        }
        else if(header->rom_size == 0x5)
        {
            // replace 5th bit with lowest bit of ram bank
            HighBank = (current_rom_bank & 0b11101111) | (current_ram_bank & 0x1) << 5;

        }
        else if(header->rom_size == 0x6)
        {
            // replace 5th and 6th bit with lowest two bits of ram bank number
            HighBank = (current_rom_bank & 0b11001111) | (current_ram_bank & 0x3) << 5;
        }
    }

    bool cart::load(char *filename)
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

        setupBanking();

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