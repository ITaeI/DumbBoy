#pragma once
#include "common.h"

class cart
{
    private:

    public:

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

    struct cart_context
    {
        char filename[1024];
        u32 rom_size;
        u8 *romdata;
        cart_header *header;
    };

    static cart_context cart_ctx;
    //Load a ROM file
    bool cart_load(char *filename);
    std::string getRomTypeName(int code);
    std::string getLicenseeName(int code);

};