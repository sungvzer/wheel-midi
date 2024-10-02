cmake_minimum_required(VERSION 3.30)
project(MidiPedal VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
# include from ./include
add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-unknown-warning-option)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE src/main.cpp )
add_executable(MidiPedal ${SOURCE})

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

target_link_libraries(MidiPedal rtmidi udev SDL2::SDL2)
