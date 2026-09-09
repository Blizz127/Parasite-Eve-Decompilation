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
    platform/pe_dispenv.c
    platform/pe_timer1.c
    platform/pe_vsync.c
    platform/host_vram.c
    platform/pe_guest_ram.c
    platform/pe_gpu.c
    platform/pe_irq.c
    platform/pe_irq_delivery.c
    platform/pe_gte.c
    platform/pe_mdec.c
    platform/pe_libetc.c
    platform/pe_libgpu.c
    platform/pe_libsnd.c
    platform/pe_spu_dma.c
    platform/pe_spu_synth.c
    platform/pe_host_audio.c
    platform/pe_libcard.c
    platform/pe_cdreg.c
    platform/pe_libcd.c
    platform/pe_disc.c
    platform/pe_save.c
    platform/pe_stream.c
    platform/pe_stream_commands.c
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
    bootstrap/func_8006ECEC_port.c
    bootstrap/func_800698D4_port.c
    bootstrap/func_8001220C_port.c
)

set(GAME_SRCS
    game/boot/func_8003E610_port.c
    game/boot/func_8003E680_port.c
    game/boot/func_80071A84_port.c
    game/boot/func_80072334_port.c
    game/boot/func_80041108_port.c
    game/boot/func_800405A4_port.c
    game/boot/func_80015AF0_port.c
    game/boot/func_80019DB8_port.c
    game/boot/func_8003E91C_port.c
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
    game/boot/func_800D413C_port.c
    game/boot/func_800D751C_port.c
    game/boot/func_800C2414_port.c
    game/boot/func_800C3B04_port.c
    game/boot/func_800C9C20_port.c
    game/boot/func_800CEE20_port.c
    game/boot/func_8018F330_port.c
    game/boot/m0013i_effect_port.c
    game/boot/m0023i_effect_port.c
    game/boot/mirror_effect_port.c
    game/boot/field_item_pickup_port.c
    game/boot/actor_contact_port.c
    game/boot/func_800CEB8C_port.c
    game/boot/func_800C71E4_port.c
    game/boot/func_80062D2C_port.c
    game/boot/func_8004DD64_port.c
    game/boot/func_8004DF74_port.c
    game/boot/func_8005C498_port.c
    game/boot/func_800438EC_port.c
    game/boot/func_80043B0C_port.c
    game/boot/func_800512AC_port.c
    game/boot/func_8005DCEC_port.c
    game/boot/func_80055760_port.c
    game/boot/func_80044924_port.c
    game/boot/func_8004F910_port.c
    game/boot/func_800452C0_port.c
    game/boot/func_8005112C_port.c
    game/boot/func_80026824_port.c
    game/boot/func_80044B0C_port.c
    game/boot/func_8005E12C_port.c
    game/boot/func_8005EED4_port.c
    game/boot/func_80061C34_port.c
    game/boot/func_800638D8_port.c
    game/boot/func_80063E0C_port.c
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
    game/boot/func_80191854_port.c
    game/boot/func_8018F55C_port.c
    game/boot/func_80195994_port.c
    game/boot/func_80196498_port.c
    game/boot/func_80192030_port.c
    game/boot/func_8018F344_port.c
    game/boot/func_8018F05C_port.c
    game/boot/func_80190D3C_port.c
    game/boot/func_80192800_port.c
    game/boot/transition_ordering_table_port.c
    game/boot/pe_controller_sdk_port.c
    game/boot/func_80197BA0_port.c
    game/boot/func_8019A318_port.c
    game/boot/func_801995BC_port.c
    game/boot/func_8019B1D0_port.c
    game/boot/func_80079384_port.c
    game/boot/func_8018F92C_port.c
    game/boot/func_80193478_port.c
    game/boot/func_801942FC_port.c
    game/boot/func_801904B0_port.c
    game/boot/movie_overlay_port.c
    game/boot/cd_stream_port.c
    game/boot/func_80191DE8_port.c
    game/boot/func_80190998_port.c
    game/boot/func_80012850_port.c
    game/boot/func_80016910_port.c
    game/boot/func_8001A374_port.c
    game/boot/func_8001735C_port.c
    game/boot/func_80015240_port.c
    game/boot/func_80014BA0_port.c
    game/boot/func_8001A15C_port.c
    game/boot/func_800130B4_port.c
    game/boot/func_800131E8_port.c
    game/boot/func_80013C34_port.c
    game/boot/func_800143B0_port.c
    game/boot/func_8006B4F8_port.c
    game/boot/func_800371B0_port.c
    game/boot/func_8001A890_port.c
    game/boot/func_8001AE40_port.c
    game/boot/func_80034F10_port.c
    game/boot/func_8006536C_port.c
    game/boot/func_800653B8_port.c
    game/boot/func_80065954_port.c
    game/boot/func_800659F8_port.c
    game/boot/func_80013988_port.c
    game/boot/func_80013E84_port.c
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
    game/boot/func_80033A40_port.c
    game/boot/func_8001D340_port.c
    game/boot/func_8001F9C4_port.c
    game/boot/func_80023E14_port.c
    game/boot/func_8006DE80_port.c
    game/boot/func_80053E6C_port.c
    game/boot/func_8002AA98_port.c
    game/boot/func_80027D14_port.c
    game/boot/func_8002DC58_port.c
    game/boot/func_8002CF24_port.c
    game/boot/func_800293F4_port.c
    game/boot/func_8006CDA4_port.c
    game/boot/func_8006D2B8_port.c
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
    game/boot/func_80051980_port.c
    game/boot/func_8002156C_port.c
    game/boot/func_80020F18_port.c
    game/boot/func_80026FF8_port.c
    game/boot/func_80025EE8_port.c
    game/boot/func_80021F38_port.c
    game/boot/func_80022210_port.c
    game/boot/func_800D4C24_port.c
    game/boot/func_80022394_port.c
    game/boot/func_80024250_port.c
    game/boot/func_80046574_port.c
    game/boot/func_8004542C_port.c
    game/boot/func_80046DFC_port.c
    game/boot/func_8005B500_port.c
    game/boot/func_80046ABC_port.c
    game/boot/func_800515F8_port.c
    game/boot/func_800236E8_port.c
    game/boot/func_80028574_port.c
    game/boot/func_800314E4_port.c
    game/boot/func_8007041C_port.c
    game/boot/func_800C22F8_port.c
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
    game/boot/func_80042798_port.c
    game/boot/func_800752AC_port.c
    game/boot/func_80075358_port.c
    game/boot/func_800753B4_port.c
    game/boot/func_80070E54_port.c
    game/boot/func_80076664_port.c
    game/boot/func_800768A0_port.c
    game/boot/func_80073CF4_port.c
    game/boot/func_800746A0_port.c
    game/boot/func_80076C34_port.c
    game/boot/func_80076EE4_port.c
    game/boot/func_80077B64_port.c
    game/boot/func_80077BA4_port.c
    game/boot/func_80077BC4_port.c
    game/boot/func_80077BE4_port.c
    game/boot/func_80077C44_port.c
    game/boot/func_80077C64_port.c
    game/boot/func_80077A64_port.c
    game/boot/func_80077AA4_port.c
    game/boot/func_80077B04_port.c
    game/boot/func_80077B34_port.c
    game/boot/func_80077C04_port.c
    game/boot/func_80077C84_port.c
    game/boot/func_80077CB4_port.c
    game/boot/func_8007A214_port.c
    game/boot/func_8005DADC_port.c
    game/boot/func_800370DC_port.c
    game/boot/func_80037140_port.c
    game/boot/func_80030894_port.c
    game/boot/func_8010BE3C_port.c
    game/boot/func_80190660_port.c
    game/boot/func_801909B4_port.c
    game/boot/func_801918F8_port.c
    game/boot/func_80191FB8_port.c
    game/boot/func_801924F8_port.c
    game/boot/func_80081314_port.c
    game/boot/func_80192CE8_port.c
    game/boot/func_800870F0_port.c
    game/boot/func_80191B64_port.c
    game/boot/func_8010BD4C_port.c
    game/boot/func_8010C89C_port.c
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
# Window backend is per-OS: X11/dlopen on POSIX, Win32 GDI on Windows.
# Both implement the same platform/host_window.h interface.
if(WIN32)
    set(HOST_WINDOW_SRC platform/host_window_win32.c)
else()
    set(HOST_WINDOW_SRC platform/host_window.c)
endif()

add_executable(parasite-eve-port
    src/port_main.c
    ${HOST_WINDOW_SRC}
)
target_link_libraries(parasite-eve-port PRIVATE pe_field_runtime)
if(NOT WIN32)
    # POSIX X11 backend resolves dlopen/dlsym from libdl.
    target_link_libraries(parasite-eve-port PRIVATE dl)
endif()

target_compile_definitions(parasite-eve-port PRIVATE
    PE_PORT_HEADLESS=1
    PE_PORT_FB_WIDTH=320
    PE_PORT_FB_HEIGHT=240
)

# GCC/Clang-style sanitizer flags; MSVC uses /fsanitize=address instead,
# so these only apply to non-MSVC toolchains.
if(PE_PORT_SANITIZERS AND NOT MSVC)
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
    if(WIN32)
        add_executable(pe-win32-input-tests tests/test_win32_input.c platform/host_window_win32.c)
        target_include_directories(pe-win32-input-tests PRIVATE platform)
        target_link_libraries(pe-win32-input-tests PRIVATE user32 gdi32)
        add_test(NAME win32-input COMMAND pe-win32-input-tests)
        set_tests_properties(win32-input PROPERTIES TIMEOUT 30)
    endif()
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

    if(UNIX AND NOT APPLE)
        add_executable(pe-disc-loader-tests tests/test_disc_loader_loop.c)
        target_link_libraries(pe-disc-loader-tests PRIVATE pe_field_runtime)
        foreach(provider 80087198 80087414 8006E6D4 8006E7E8 800871AC 80087090 800875FC 80087428 800870E0)
            target_link_options(pe-disc-loader-tests PRIVATE "-Wl,--wrap=func_${provider}")
        endforeach()
        if(PE_PORT_SANITIZERS)
            target_compile_options(pe-disc-loader-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(pe-disc-loader-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
        endif()
        add_test(NAME disc-loader-control-flow COMMAND pe-disc-loader-tests)
        set_tests_properties(disc-loader-control-flow PROPERTIES TIMEOUT 30)
    endif()

    if(UNIX AND NOT APPLE)
        add_executable(pe-music-bank-tests tests/test_music_bank.c)
        target_link_libraries(pe-music-bank-tests PRIVATE pe_field_runtime)
        target_link_options(pe-music-bank-tests PRIVATE
            "-Wl,--wrap=func_8006CDA4" "-Wl,--wrap=func_80086FF8")
        if(PE_PORT_SANITIZERS)
            target_compile_options(pe-music-bank-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(pe-music-bank-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
        endif()
        add_test(NAME music-bank-control-flow COMMAND pe-music-bank-tests)
        set_tests_properties(music-bank-control-flow PROPERTIES TIMEOUT 30)

        add_executable(pe-transition-exit-tests tests/test_transition_exit.c)
        target_link_libraries(pe-transition-exit-tests PRIVATE pe_field_runtime)
        foreach(provider HostFB_SetDispMask HostFB_VSync func_80074A44 func_80074F44 func_80074DC0 func_800755F0 func_80086C5C func_8008CBA8 func_80086FF8 func_80087024)
            target_link_options(pe-transition-exit-tests PRIVATE "-Wl,--wrap=${provider}")
        endforeach()
        if(PE_PORT_SANITIZERS)
            target_compile_options(pe-transition-exit-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(pe-transition-exit-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
        endif()
        add_test(NAME transition-exit-control-flow COMMAND pe-transition-exit-tests)
        set_tests_properties(transition-exit-control-flow PROPERTIES TIMEOUT 30)

        add_executable(pe-transition-constructor-tests tests/test_transition_constructor.c)
        target_link_libraries(pe-transition-constructor-tests PRIVATE pe_field_runtime)
        foreach(provider func_80191854 func_800371B0 func_80078E34 func_80078E64 func_80078FC4 func_80077E64 func_80078FE4)
            target_link_options(pe-transition-constructor-tests PRIVATE "-Wl,--wrap=${provider}")
        endforeach()
        if(PE_PORT_SANITIZERS)
            target_compile_options(pe-transition-constructor-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(pe-transition-constructor-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
        endif()
        add_test(NAME transition-constructor-control-flow COMMAND pe-transition-constructor-tests)
        set_tests_properties(transition-constructor-control-flow PROPERTIES TIMEOUT 30)

        add_executable(pe-transition-init-tests tests/test_transition_init.c)
        target_link_libraries(pe-transition-init-tests PRIVATE pe_field_runtime)
        foreach(provider func_8019BD78 func_8005BCB0 func_800371A4 func_80191C94)
            target_link_options(pe-transition-init-tests PRIVATE "-Wl,--wrap=${provider}")
        endforeach()
        if(PE_PORT_SANITIZERS)
            target_compile_options(pe-transition-init-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(pe-transition-init-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
        endif()
        add_test(NAME transition-init-control-flow COMMAND pe-transition-init-tests)
        set_tests_properties(transition-init-control-flow PROPERTIES TIMEOUT 30)

        add_executable(pe-transition-loader-tests tests/test_transition_loader.c)
        target_link_libraries(pe-transition-loader-tests PRIVATE pe_field_runtime)
        foreach(provider HostFB_VSync HostFB_SetDispMask func_8006E6A8 func_8006E7E8 func_8006CDA4 func_8006E6D4 func_800811E4 func_800718D0 func_80074DC0 func_80072714 func_800726C4 func_80072724)
            target_link_options(pe-transition-loader-tests PRIVATE "-Wl,--wrap=${provider}")
        endforeach()
        if(PE_PORT_SANITIZERS)
            target_compile_options(pe-transition-loader-tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(pe-transition-loader-tests PRIVATE -fsanitize=address,undefined -static-libasan -static-libubsan)
        endif()
        add_test(NAME transition-loader-control-flow COMMAND pe-transition-loader-tests)
        set_tests_properties(transition-loader-control-flow PROPERTIES TIMEOUT 30)
    endif()

    if(PE_PORT_SANITIZERS AND NOT MSVC)
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
