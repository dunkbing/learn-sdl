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
find_package(SDL3_image CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

add_executable(sdl3_learn src/main.cpp)
target_link_libraries(sdl3_learn PRIVATE SDL3::SDL3 SDL3_image::SDL3_image box2d::box2d)

# Convert the PNG to a C++ byte array during configuration. The resulting data
# is linked into the executable, so the game does not load the PNG from disk.
set(PLAYER_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/assets/player.png")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PLAYER_IMAGE}")
file(READ "${PLAYER_IMAGE}" PLAYER_IMAGE_HEX HEX)
string(REGEX REPLACE "([0-9A-Fa-f][0-9A-Fa-f])" "0x\\1, " PLAYER_IMAGE_BYTES "${PLAYER_IMAGE_HEX}")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/player_png.h"
"#pragma once\n\n#include <cstddef>\n\ninline constexpr unsigned char playerPng[] = { ${PLAYER_IMAGE_BYTES} };\ninline constexpr std::size_t playerPngSize = sizeof(playerPng);\n")
target_include_directories(sdl3_learn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
