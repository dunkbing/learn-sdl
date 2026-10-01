cmake_minimum_required(VERSION 3.25)

# Must be set before project()
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()
project(sdl3_learn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

add_executable(sdl3_learn src/main.cpp)
target_link_libraries(sdl3_learn PRIVATE SDL3::SDL3 box2d::box2d)
