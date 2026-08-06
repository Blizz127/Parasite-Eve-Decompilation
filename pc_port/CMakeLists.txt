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
    platform/func_80071A24_port.c
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
    game/boot/func_80036DC8_port.c
    game/boot/func_800371A4_port.c
    game/boot/func_80029388_port.c
    game/boot/func_8005BCA8_port.c
    game/boot/func_80068D28_port.c
    game/boot/func_800124F8_port.c
    game/boot/func_8001A890_port.c
    game/boot/func_80034F10_port.c
    game/boot/func_8006536C_port.c
    game/boot/func_80038D1C_port.c
    game/boot/func_8006E6A8_port.c
    game/boot/func_8006E7E8_port.c
    game/boot/func_8006E498_port.c
    game/boot/func_8006A9E4_port.c
    game/boot/func_8005B890_port.c
    game/boot/func_8005BC98_port.c
    game/boot/func_8004F808_port.c
    game/boot/func_80042B38_port.c
    game/boot/func_80051084_port.c
    game/boot/func_800527C8_port.c
    game/boot/func_800528F0_port.c
    game/boot/func_8005E588_port.c
    game/boot/func_80062568_port.c
    game/boot/func_80064964_port.c
    game/boot/func_8005DE88_port.c
    game/boot/func_80052C6C_port.c
    game/boot/func_80053D2C_port.c
    game/boot/func_80042C78_port.c
    game/boot/func_80042CC4_port.c
    game/boot/func_8006A2E8_port.c
    game/boot/func_80052790_port.c
    game/boot/func_800649D0_port.c
    game/boot/func_8005E850_port.c
    game/boot/func_8005E884_port.c
    game/boot/func_800614AC_port.c
    game/boot/func_8005BCBC_port.c
    game/boot/func_8005D6F4_port.c
    game/boot/func_8005DC4C_port.c
    game/boot/func_80052594_port.c
    game/boot/func_8005DB8C_port.c
    game/boot/func_8005DBAC_port.c
    game/boot/func_800438C0_port.c
    game/boot/func_8005CCA4_port.c
    game/boot/func_8005E968_port.c
    game/boot/func_8005F844_port.c
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
    platform/func_80071A24_port.c
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
