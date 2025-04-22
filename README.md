# **GameBoy Emulator**


## Table of Contents

- [Overview](#overview)
- [Screenshots](#screenshots)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Resources](#resources)


## Overview

A fun side project on emulating an original Gameboy (and possibly Gameboy color) using C++. It uses the SDL3 library for rendering as well as Dear ImGui for UI, (Specifically the docking branch, as its pretty cool)

[SDL/SDL3](https://github.com/libsdl-org/SDL)
[ImGui](https://github.com/ocornut/imgui/tree/docking)

## Screenshots

![FullScreen](DumbBoy/Screenshots/FullScreen.jpg)
![Background](DumbBoy/Screenshots/Background.jpg)
![Tiles](DumbBoy/Screenshots/Tiles.jpg)

## Features

- Accurate CPU, PPU, timer emulation
- Debug Windows to inspect Tiles and registers
- Simple Folder Scanning (For gb and gbc roms)
- MBC1, MBC2, MBC3 are available
- Currently Windows Only
- Audio (WIP)

## Installation

### Install [Cmake]https://cmake.org/download/ Minimum 3.24

    git clone --recursive-submodules https://github.com/ITaeI/DumbBoy.git ./DumbBoy

### Using Visual Studio Code

- Visual Studio Code has a two extensions (Use Both)
    - Cmake : twxs
    - Cmake Tools : Microsoft

#### Quick Startup

On the search bar on the top of the screen:

- >Cmake Quick Start

### Using Visual Studio

- A Workload is available that includes Cmake
    - Tools -> Get Tools and Features ->  Workloads -> Desktop development with C++
    - Tools -> Get Tools and Features -> Individual Components -> C++ Cmake tools for Windows


### Build Via CommandLine

    cd ./DumbBoy
    mkdir build
    cd build
    cmake ..
    cmake --build . --conifg Release

     
## Usage

### Keyboard Bindings

    |:---------:|
    |Up    |   W|
    |:---------:|                            
    |Left  |   A|
    |:---------:|
    |Down  |   S|
    |:---------:|
    |Right |   D|
    |:---------:|
    |A     |   L|
    |:---------:|
    |B     |   K|
    |:---------:|
    |Start |   O|
    |:---------:|
    |Select|   P|
    |:---------:|

### Rom Select

When Running the Executable:
- File -> Select Rom -> Input Rom Directory -> Select File -> Load Rom
- .sav Files will be created within the same directory as Rom

## Resources

- Pandocs: https://gbdev.io/pandocs/
- Opcode Reference: https://rgbds.gbdev.io/docs/v0.9.1/gbz80.7
- Opcode Tables: https://gbdev.io/gb-opcodes/optables/
- General GB Emulator Guide: https://hacktix.github.io/GBEDG/
- Complete Gameboy Technical Reference: https://gekkio.fi/files/gb-docs/gbctr.pdf

