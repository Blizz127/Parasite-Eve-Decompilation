cmake_minimum_required(VERSION 3.16)
project(parasite_eve_port C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Phase 6A — Native Boot-to-Black Scaffold
#
# This is NOT a PS1 emulator.  It compiles translated Parasite Eve C sources
# as a native Linux executable with software-rendered framebuffer output.
#
# No SDL2, no OpenGL, no OpenAL.  Headless-first, PPM output.
# ---------------------------------------------------------------------------

# Source lists — build incrementally
set(PORT_SRCS
    src/port_main.c
    src/pe_globals.c
    platform/host_framebuffer.c
    bootstrap/stub_registry.c
    bootstrap/func_8006E9A0_port.c
)

# Include paths
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/platform
    ${CMAKE_SOURCE_DIR}/bootstrap
)

# Main executable
add_executable(parasite-eve-port ${PORT_SRCS})

# Headless framebuffer: no external deps, PPM output
target_compile_definitions(parasite-eve-port PRIVATE
    PE_PORT_HEADLESS=1
    PE_PORT_FB_WIDTH=320
    PE_PORT_FB_HEIGHT=240
)

# Tests (optional)
option(BUILD_TESTS "Build native port tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
