// Third Party Libraries
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

// Standard Libraries
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// Standard Variable Sizes

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


#define NO_IMPL { fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5); }