#include "common.h"
#include <SDL3/SDL_main.h>


int main(int argc, char *argv[])
{
    Emulator emu;
    return emu.run(argc, argv);
}
