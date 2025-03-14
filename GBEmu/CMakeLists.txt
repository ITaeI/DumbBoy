cmake_minimum_required(VERSION 3.16)

project(GBEmu) 

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(EXECUTABLE_NAME ${PROJECT_NAME}) 

add_executable(${EXECUTABLE_NAME})   
 
file(GLOB SOURCES "src/*.cpp")
target_sources(${EXECUTABLE_NAME} PRIVATE ${SOURCES})  
 
add_subdirectory(ThirdParty/SDL)   
  
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL3::SDL3)  
