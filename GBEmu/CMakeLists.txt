cmake_minimum_required(VERSION 3.24)

project(GBEmu) 

set(CMAKE_CXX_STANDARD 17)

#-------------Fetching Required Packages---------------#

include(FetchContent)

# SDL3
FetchContent_Declare(
    SDL3
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.8.zip
)
FetchContent_MakeAvailable(SDL3)


#------------------Linking Packages To Project------------------#


set(EXECUTABLE_NAME ${PROJECT_NAME})  
File(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(${EXECUTABLE_NAME} SDL3::SDL3)  