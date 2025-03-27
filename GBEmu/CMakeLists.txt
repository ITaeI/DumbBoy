cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)

# --- Setup Project --- #
project(GBEmu) 
set(EXECUTABLE_NAME ${PROJECT_NAME})  


# --- Set up imgui Directories --- #
set(Third_Party_DIR ThirdParty)
set(IMGUI_DIR ${Third_Party_DIR}/imgui)
set(IMGUI_BACKENDS ${IMGUI_DIR}/backends)

# --- Tell CMAKE where to find SDL Package --- #
add_subdirectory(${Third_Party_DIR}/SDL)

# --- Build Imgui Library --- #

add_library(imgui)

target_include_directories(
    imgui
    PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_BACKENDS} 
)

target_sources(
    imgui
    PUBLIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    ${IMGUI_BACKENDS}/imgui_impl_sdl3.cpp
    ${IMGUI_BACKENDS}/imgui_impl_sdlrenderer3.cpp
)



# --- Setup GB Sources and Header directories ---#

file(GLOB GB_SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${GB_SOURCES}) 
set(EXECUTABLE_NAME ${PROJECT_NAME})
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include) 

# --- Link Libraries together --- #
target_link_libraries(imgui PUBLIC SDL3::SDL3)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE imgui)

