cmake_minimum_required(VERSION 3.16)
project(parasite_eve_port C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Phase 6D-S — Native Boot-to-Black Scaffold with host-safe guest memory
# ---------------------------------------------------------------------------

# Sanitizer option
option(PE_PORT_SANITIZERS "Build with AddressSanitizer and UBSan" OFF)

# Source lists
set(PLATFORM_SRCS
    platform/host_framebuffer.c
    platform/host_window.c
    platform/pe_guest_ram.c
    platform/pe_gte.c
    platform/pe_libetc.c
    platform/pe_libgpu.c
    platform/pe_libsnd.c
    platform/pe_libcard.c
    platform/pe_libcd.c
    platform/pe_disc.c
    platform/pe_save.c
    platform/pe_stream.c
    platform/pe_guest_image.c
)

set(BOOTSTRAP_SRCS
    bootstrap/stub_registry.c
    bootstrap/pe_bootstrap.c
    bootstrap/func_800725DC_port.c
    bootstrap/func_8006E9A0_port.c
    bootstrap/func_8006E834_port.c
    bootstrap/func_800698D4_port.c
    bootstrap/func_8001220C_port.c
)

set(GAME_SRCS
    game/boot/func_8003E610_port.c
    game/boot/func_8003E680_port.c
    game/boot/func_8003E754_port.c
    game/boot/func_8003E944_port.c
    game/boot/func_8003E974_port.c
    game/boot/func_8003EAC8_port.c
    game/boot/func_80070D10_port.c
    game/boot/func_80070D6C_port.c
    game/boot/func_80070DD0_port.c
    game/boot/func_8006A5BC_port.c
    game/boot/func_8006A64C_port.c
    game/boot/func_8006A674_port.c
    game/boot/func_8006A8D4_port.c
)

set(PORT_SRCS
    src/port_main.c
    src/pe_globals.c
    platform/pe_callback.c
    ${PLATFORM_SRCS}
    ${BOOTSTRAP_SRCS}
    ${GAME_SRCS}
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

target_compile_definitions(parasite-eve-port PRIVATE
    PE_PORT_HEADLESS=1
    PE_PORT_FB_WIDTH=320
    PE_PORT_FB_HEIGHT=240
)

if(PE_PORT_SANITIZERS)
    target_compile_options(parasite-eve-port PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    # Static runtimes: the system shared libasan can mismatch the compiler
    target_link_options(parasite-eve-port PRIVATE
        -fsanitize=address,undefined
        -static-libasan -static-libubsan
    )
endif()

# Tests
option(BUILD_TESTS "Build native port tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_executable(pe-native-tests
        tests/test_native.c
        platform/host_framebuffer.c
        platform/pe_guest_ram.c
        platform/pe_callback.c
        platform/pe_gte.c
        platform/pe_libetc.c
        platform/pe_libgpu.c
        platform/pe_libsnd.c
        platform/pe_libcard.c
        platform/pe_libcd.c
        platform/pe_disc.c
        platform/pe_save.c
        platform/pe_stream.c
        platform/pe_guest_image.c
        bootstrap/stub_registry.c
        bootstrap/pe_bootstrap.c
        src/pe_globals.c
        ${GAME_SRCS}
        bootstrap/func_800725DC_port.c
        bootstrap/func_800698D4_port.c
        bootstrap/func_8006E834_port.c
        bootstrap/func_8006E9A0_port.c
    )
    target_include_directories(pe-native-tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/bootstrap
        ${CMAKE_SOURCE_DIR}/src
    )
    target_compile_definitions(pe-native-tests PRIVATE
        PE_PORT_HEADLESS=1
        PE_PORT_FB_WIDTH=320
        PE_PORT_FB_HEIGHT=240
    )
    target_link_libraries(pe-native-tests PRIVATE dl)

    if(PE_PORT_SANITIZERS)
        target_compile_options(pe-native-tests PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(pe-native-tests PRIVATE
            -fsanitize=address,undefined
            -static-libasan -static-libubsan
        )
    endif()

    add_test(NAME native-tests COMMAND pe-native-tests)
endif()
