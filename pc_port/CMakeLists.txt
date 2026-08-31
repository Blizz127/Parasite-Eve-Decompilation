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
    platform/pe_guest_ram.c
    platform/pe_gpu.c
    platform/pe_irq.c
    platform/pe_irq_delivery.c
    platform/pe_gte.c
    platform/pe_libetc.c
    platform/pe_libgpu.c
    platform/pe_libsnd.c
    platform/pe_spu_dma.c
    platform/pe_libcard.c
    platform/pe_libcd.c
    platform/pe_disc.c
    platform/pe_save.c
    platform/pe_stream.c
    platform/func_80071A24_port.c
    platform/func_800719E4_port.c
    platform/pe_guest_image.c
)

set(BOOTSTRAP_SRCS
    bootstrap/game_port.c
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
    game/boot/func_80068E24_port.c
    game/boot/func_80066B60_port.c
    game/boot/func_8006F39C_port.c
    game/boot/func_8006F6D4_port.c
    game/boot/func_8002FAF8_port.c
    game/boot/func_80014228_port.c
    game/boot/func_80069594_port.c
    game/boot/func_80037548_port.c
    game/boot/func_800375E0_port.c
    game/boot/func_80037870_port.c
    game/boot/func_8003EB04_port.c
    game/boot/func_8003F3C4_port.c
    game/boot/func_800124F8_port.c
    game/boot/func_800125E0_port.c
    game/boot/func_80035558_port.c
    game/boot/func_80017018_port.c
    game/boot/func_800181CC_port.c
    game/boot/func_80015DAC_port.c
    game/boot/func_80012850_port.c
    game/boot/func_80016910_port.c
    game/boot/func_8001A374_port.c
    game/boot/func_8001735C_port.c
    game/boot/func_80015240_port.c
    game/boot/func_8001A15C_port.c
    game/boot/func_800130B4_port.c
    game/boot/func_800131E8_port.c
    game/boot/func_80013C34_port.c
    game/boot/func_800143B0_port.c
    game/boot/func_8006B4F8_port.c
    game/boot/func_800371B0_port.c
    game/boot/func_8001A890_port.c
    game/boot/func_80034F10_port.c
    game/boot/func_8006536C_port.c
    game/boot/func_800653B8_port.c
    game/boot/func_80065954_port.c
    game/boot/func_800659C8_port.c
    game/boot/func_80066800_port.c
    game/boot/func_80017764_port.c
    game/boot/func_80065400_port.c
    game/boot/func_80012700_port.c
    game/boot/func_80017BB4_port.c
    game/boot/func_800177AC_port.c
    game/boot/func_80019154_port.c
    game/boot/func_8002F7D8_port.c
    game/boot/func_8002FA10_port.c
    game/boot/func_8002FAA4_port.c
    game/boot/func_8002FF78_port.c
    game/boot/func_8002FE78_port.c
    game/boot/func_8003010C_port.c
    game/boot/func_80030220_port.c
    game/boot/func_800299CC_port.c
    game/boot/func_8001D340_port.c
    game/boot/func_8006DE80_port.c
    game/boot/func_80053E6C_port.c
    game/boot/func_8002AA98_port.c
    game/boot/func_80027D14_port.c
    game/boot/func_8002CF24_port.c
    game/boot/func_800293F4_port.c
    game/boot/func_80029810_port.c
    game/boot/func_8006C4C4_port.c
    game/boot/func_8003D050_port.c
    game/boot/func_8003C5D8_port.c
    game/boot/func_8006698C_port.c
    game/boot/func_8003DFD8_port.c
    game/boot/func_800794C4_port.c
    game/boot/func_8003A088_port.c
    game/boot/func_8003B97C_port.c
    game/boot/func_8003BCE0_port.c
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
    game/boot/func_8005332C_port.c
    game/boot/func_80053968_port.c
    game/boot/func_80053B48_port.c
    game/boot/func_80051CC4_port.c
    game/boot/func_8005218C_port.c
    game/boot/func_8005B91C_port.c
    game/boot/func_800527C8_port.c
    game/boot/func_800528F0_port.c
    game/boot/func_8005E588_port.c
    game/boot/func_8005E57C_port.c
    game/boot/func_8005C1EC_port.c
    game/boot/func_80042538_port.c
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
    game/boot/func_80086728_port.c
    game/boot/func_80087090_port.c
    game/boot/func_800851A8_port.c
    game/boot/func_80085EB4_port.c
    game/boot/func_800850F4_port.c
    game/boot/func_8006AD40_port.c
    game/boot/func_800718D0_port.c
    game/boot/func_8006E1C0_port.c
    game/boot/func_8007506C_port.c
    game/boot/func_8007512C_port.c
    game/boot/func_80075358_port.c
    game/boot/func_80076664_port.c
    game/boot/func_80073CF4_port.c
    game/boot/func_800746A0_port.c
    game/boot/func_80076C34_port.c
    game/boot/func_80076EE4_port.c
    game/boot/func_80077B64_port.c
    game/boot/func_80077BA4_port.c
    game/boot/func_80077BC4_port.c
    game/boot/func_80077C44_port.c
    game/boot/func_80077C64_port.c
    game/boot/func_80077A64_port.c
    game/boot/func_80077AA4_port.c
    game/boot/func_80077B04_port.c
    game/boot/func_80077B34_port.c
    game/boot/func_80077C04_port.c
    game/boot/func_80077C84_port.c
    game/boot/func_80077CB4_port.c
    game/boot/func_8005DADC_port.c
    game/boot/func_800370DC_port.c
    game/boot/func_80037140_port.c
    game/boot/func_80030894_port.c
    game/boot/func_80190660_port.c
    game/boot/func_801909B4_port.c
    game/boot/func_801918F8_port.c
    game/boot/func_80191FB8_port.c
    game/boot/func_801924F8_port.c
    game/boot/func_80192CE8_port.c
)

set(FIELD_RUNTIME_SRCS
    src/pe_globals.c
    platform/pe_callback.c
    ${PLATFORM_SRCS}
    ${BOOTSTRAP_SRCS}
    ${GAME_SRCS}
)

# The static runtime is the reusable product boundary.  It contains the
# translated retail graph and its host-safe providers, but not the CLI entry
# point or the X11 presentation adapter.  Creating this target does not claim
# that the currently translated graph is a complete Day 1 field runtime.
add_library(pe_field_runtime STATIC ${FIELD_RUNTIME_SRCS})
target_include_directories(pe_field_runtime PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/platform
    ${CMAKE_SOURCE_DIR}/bootstrap
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_definitions(pe_field_runtime PUBLIC
    PE_PORT_HEADLESS=1
    PE_PORT_FB_WIDTH=320
    PE_PORT_FB_HEIGHT=240
)

# The command-line host is one consumer of the static runtime.  Its window
# backend remains outside the library so non-X11 consumers do not inherit it.
add_executable(parasite-eve-port
    src/port_main.c
    platform/host_window.c
)
target_link_libraries(parasite-eve-port PRIVATE pe_field_runtime dl)

target_compile_definitions(parasite-eve-port PRIVATE
    PE_PORT_HEADLESS=1
    PE_PORT_FB_WIDTH=320
    PE_PORT_FB_HEIGHT=240
)

if(PE_PORT_SANITIZERS)
    target_compile_options(pe_field_runtime PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
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
    add_executable(pe-native-tests tests/test_native.c)
    target_link_libraries(pe-native-tests PRIVATE pe_field_runtime)

    # Small external-consumer contract: this source is intentionally not part
    # of the runtime target and reaches both host substrate and translated
    # retail code only through the archive's public include/link interface.
    add_executable(pe-field-runtime-link-test
        tests/test_field_runtime_link.c
    )
    target_link_libraries(pe-field-runtime-link-test PRIVATE pe_field_runtime)

    if(PE_PORT_SANITIZERS)
        target_compile_options(pe-native-tests PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(pe-native-tests PRIVATE
            -fsanitize=address,undefined
            -static-libasan -static-libubsan
        )
        target_compile_options(pe-field-runtime-link-test PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(pe-field-runtime-link-test PRIVATE
            -fsanitize=address,undefined
            -static-libasan -static-libubsan
        )
    endif()

    add_test(NAME native-tests COMMAND pe-native-tests)
    add_test(NAME field-runtime-link COMMAND pe-field-runtime-link-test)
    set_tests_properties(native-tests field-runtime-link PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
    )
endif()
