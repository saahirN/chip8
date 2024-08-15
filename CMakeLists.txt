cmake_minimum_required(VERSION 3.10)

project(Chip8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")
add_executable(Chip8 ${SOURCES})

target_link_libraries(Chip8 ${SDL2_LIBRARIES})
