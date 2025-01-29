#include "common.h"
#include "Emulator.h"
#include <SDL3/SDL_main.h>


int main(int argc, char *argv[])
{
    GBEmu::Emulator emu;
    return emu.run();
}
