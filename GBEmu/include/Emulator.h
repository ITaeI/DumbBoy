#pragma once

#include "common.h"

class Emulator
{
    private:

    struct EmulatorState
    {
        bool paused;
        bool running;
        uint64_t ticks;
    };

    static EmulatorState emu_ctx;

    public:
    //Emulator constructor
    Emulator()
    {}
    //Run the emulator
    int run(int argc, char *argv[]);
    //Get the state of the emulator
    EmulatorState getState();


};