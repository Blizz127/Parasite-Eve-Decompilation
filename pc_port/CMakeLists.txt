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
    platform/host_window.c
    bootstrap/stub_registry.c
    bootstrap/func_8006E9A0_port.c
    bootstrap/func_8006E834_port.c
    bootstrap/func_800698D4_port.c
    bootstrap/func_8001220C_port.c
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
target_link_libraries(parasite-eve-port PRIVATE dl)

# Headless framebuffer: no external deps, PPM output
target_compile_definitions(parasite-eve-port PRIVATE
    PE_PORT_HEADLESS=1
    PE_PORT_FB_WIDTH=320
    PE_PORT_FB_HEIGHT=240
)

# Tests
option(BUILD_TESTS "Build native port tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_executable(pe-native-tests
        tests/test_native.c
        platform/host_framebuffer.c
        bootstrap/stub_registry.c
    )
    target_include_directories(pe-native-tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/bootstrap
    )
    target_compile_definitions(pe-native-tests PRIVATE
        PE_PORT_HEADLESS=1
        PE_PORT_FB_WIDTH=320
        PE_PORT_FB_HEIGHT=240
    )
    add_test(NAME native-tests COMMAND pe-native-tests)
endif()
