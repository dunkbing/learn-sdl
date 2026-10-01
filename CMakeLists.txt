cmake_minimum_required(VERSION 3.25)
project(sdl3_learn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)

add_executable(sdl3_learn src/main.cpp)
target_link_libraries(sdl3_learn PRIVATE SDL3::SDL3)
