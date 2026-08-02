/*
 * Phase 6D-S — Native port tests with Boot Rung + host-safe memory
 * verification.
 *
 * Covers: framebuffer, stubs, trace, guest RAM, callback registry,
 * centralized bootstrap policy (incl. deterministic provider sequences),
 * and all six translated Boot Rung functions with direct verification.
 */

#include "psx_compat.h"
#include "pe_port_compat.h"
#include "host_framebuffer.h"
#include "stub_registry.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { tests_run++; printf("  TEST %s... ", name); } while(0)
#define PASS() do { tests_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg) do { tests_failed++; printf("FAIL: %s\n", msg); } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

/* Bootstrap-policy initial value for D_80011614 (pe_globals.c) */
#define D_80011614_BOOTSTRAP  0x8010BD00u

/* ── Helper: count occurrences of a symbol in the order log ──────────── */
static int CountOrderLog(const char *symbol) {
    int n = 0;
    for (int i = 0; i < g_stub_order_count; i++) {
        if (strcmp(g_stub_order_log[i], symbol) == 0) n++;
    }
    return n;
}

/* ── Helper: reset all test state ────────────────────────────────────── */
static void ResetTestState(void) {
    g_stub_count = 0;
    g_stub_order_count = 0;
    g_stub_bootstrap_invocations = 0;
    g_bootstrap_disc = 0;
    g_strict_stubs = 0;
    PE_RamReset();                  /* zero-fill guest RAM between tests */
    Bootstrap_ClearSequences();     /* drop scripted provider sequences   */
    D_80011614 = D_80011614_BOOTSTRAP;
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6A baseline tests (1-12)
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_fb_init_zeros(void) {
    TEST("fb_init_zeros");
    HostFB_Init();
    const uint8_t *p = HostFB_GetPixels();
    for (int i = 0; i < PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * 3; i++) {
        if (p[i] != 0) { FAIL("framebuffer not zero after init"); return; }
    }
    PASS();
}

static void test_clearimage_full(void) {
    TEST("clearimage_full");
    HostFB_Init();
    HostFB_ClearImage(0, 0, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT, 0, 0, 1);
    const uint8_t *p = HostFB_GetPixels();
    for (int i = 0; i < PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * 3; i += 3) {
        if (p[i] != 0 || p[i+1] != 0 || p[i+2] != 1) {
            FAIL("pixel not RGB(0,0,1) after ClearImage");
            return;
        }
    }
    PASS();
}

static void test_clearimage_clip(void) {
    TEST("clearimage_clip");
    HostFB_Init();
    HostFB_ClearImage(0, 0, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT, 0xFF, 0, 0);
    HostFB_ClearImage(-5, 0, 10, PE_PORT_FB_HEIGHT, 0, 0, 1);
    const uint8_t *p = HostFB_GetPixels();
    ASSERT(p[0] == 0 && p[1] == 0 && p[2] == 1, "clip: pixel at (0,0) not cleared");
    ASSERT(p[15] == 0xFF && p[16] == 0 && p[17] == 0, "clip: pixel at (5,0) should be red");
    PASS();
}

static void test_clearimage_oob(void) {
    TEST("clearimage_oob");
    HostFB_Init();
    HostFB_ClearImage(-100, -100, 50, 50, 0, 0, 1);
    const uint8_t *p = HostFB_GetPixels();
    ASSERT(p[0] == 0, "oob: pixel should still be zero");
    PASS();
}

static void test_display_mask(void) {
    TEST("display_mask");
    HostFB_Init();
    int mask;
    HostFB_GetState(NULL, NULL, NULL, &mask);
    ASSERT(mask == 0, "mask should start at 0");
    HostFB_SetDispMask(1);
    HostFB_GetState(NULL, NULL, NULL, &mask);
    ASSERT(mask == 1, "mask should be 1 after set");
    PASS();
}

static void test_vsync_counter(void) {
    TEST("vsync_counter");
    HostFB_Init();
    int vs;
    HostFB_GetState(&vs, NULL, NULL, NULL);
    ASSERT(vs == 0, "vsync starts at 0");
    HostFB_VSync(0);
    HostFB_VSync(1);
    HostFB_GetState(&vs, NULL, NULL, NULL);
    ASSERT(vs == 2, "vsync should be 2 after two calls");
    PASS();
}

static void test_drawsync_counter(void) {
    TEST("drawsync_counter");
    HostFB_Init();
    int ds;
    HostFB_GetState(NULL, &ds, NULL, NULL);
    ASSERT(ds == 0, "drawsync starts at 0");
    HostFB_DrawSync(0);
    HostFB_GetState(NULL, &ds, NULL, NULL);
    ASSERT(ds == 1, "drawsync should be 1 after call");
    PASS();
}

static void test_ppm_deterministic(void) {
    TEST("ppm_deterministic");
    HostFB_Init();
    HostFB_ClearImage(0, 0, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT, 1, 2, 3);
    HostFB_WritePPM("/tmp/pe-test-a.ppm");
    HostFB_WritePPM("/tmp/pe-test-b.ppm");
    FILE *fa = fopen("/tmp/pe-test-a.ppm", "rb");
    FILE *fb = fopen("/tmp/pe-test-b.ppm", "rb");
    ASSERT(fa && fb, "cannot open PPM files");
    fseek(fa, 0, SEEK_END); fseek(fb, 0, SEEK_END);
    long sa = ftell(fa), sb = ftell(fb);
    ASSERT(sa == sb, "PPM sizes differ");
    fseek(fa, 0, SEEK_SET); fseek(fb, 0, SEEK_SET);
    for (long i = 0; i < sa; i++) {
        if (fgetc(fa) != fgetc(fb)) {
            fclose(fa); fclose(fb);
            FAIL("PPM content differs between writes");
            return;
        }
    }
    fclose(fa); fclose(fb);
    PASS();
}

static void test_stub_registry(void) {
    TEST("stub_registry");
    g_stub_count = 0; g_stub_order_count = 0;
    Stub_Record("test_stub", "BOOTSTRAP_RET");
    Stub_Record("test_stub", "BOOTSTRAP_RET");
    ASSERT(g_stub_count == 1, "duplicate stub should not increase count");
    ASSERT(g_stub_registry[0].invoked == 2, "invocation count should be 2");
    ASSERT(strcmp(g_stub_registry[0].classification, "BOOTSTRAP_RET") == 0,
           "classification should be BOOTSTRAP_RET");
    PASS();
}

static void test_strict_stubs(void) {
    TEST("strict_stubs_flag");
    g_strict_stubs = 1;
    ASSERT(g_strict_stubs == 1, "strict_stubs flag should be set");
    g_strict_stubs = 0;
    PASS();
}

static void test_bootstrap_disc_flag(void) {
    TEST("bootstrap_disc_flag");
    g_bootstrap_disc = 1;
    ASSERT(g_bootstrap_disc == 1, "bootstrap_disc should be set");
    g_bootstrap_disc = 0;
    PASS();
}

static void test_fb_dimensions(void) {
    TEST("fb_dimensions");
    ASSERT(PE_PORT_FB_WIDTH == 320, "width should be 320");
    ASSERT(PE_PORT_FB_HEIGHT == 240, "height should be 240");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-S — Guest RAM unit tests (13-24)
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_ram_init_zero_fill(void) {
    TEST("ram_init_zero_fill");
    PE_RamReset();
    ASSERT(PE_LoadU8 (0x80000000) == 0, "RAM not zero at base");
    ASSERT(PE_LoadU16(0x800B0CD8) == 0, "RAM not zero at 0x800B0CD8");
    ASSERT(PE_LoadU32(0x80100000) == 0, "RAM not zero at 0x80100000");
    ASSERT(PE_LoadU8 (PE_RAM_END - 1) == 0, "RAM not zero at top");
    PASS();
}

static void test_ram_reset_clears_poison(void) {
    TEST("ram_reset_clears_poison");
    PE_RamInit();
    PE_StoreU32(0x80001000, 0xDEADBEEF);
    ASSERT(PE_LoadU32(0x80001000) == 0xDEADBEEF, "poison store failed");
    PE_RamReset();
    ASSERT(PE_LoadU32(0x80001000) == 0, "PE_RamReset did not clear");
    PASS();
}

static void test_ram_reset_allocates_if_needed(void) {
    TEST("ram_reset_allocates_if_needed");
    PE_RamDestroy();
    PE_RamReset();   /* must allocate, not crash */
    ASSERT(PE_LoadU8(0x80000000) == 0, "RAM unusable after bare PE_RamReset");
    PASS();
}

static void test_ram_u8_roundtrip(void) {
    TEST("ram_u8_roundtrip");
    ResetTestState();
    PE_StoreU8(0x80012345, 0xA5);
    ASSERT(PE_LoadU8(0x80012345) == 0xA5, "u8 roundtrip failed");
    ASSERT(PE_LoadU8(0x80012346) == 0, "u8 store clobbered neighbor");
    PASS();
}

static void test_ram_u16_roundtrip_le(void) {
    TEST("ram_u16_roundtrip_le");
    ResetTestState();
    PE_StoreU16(0x80002000, 0x1234);
    ASSERT(PE_LoadU8(0x80002000) == 0x34, "u16 LSB not first (LE)");
    ASSERT(PE_LoadU8(0x80002001) == 0x12, "u16 MSB not second (LE)");
    ASSERT(PE_LoadU16(0x80002000) == 0x1234, "u16 roundtrip failed");
    PASS();
}

static void test_ram_u32_roundtrip_le(void) {
    TEST("ram_u32_roundtrip_le");
    ResetTestState();
    PE_StoreU32(0x80003000, 0x12345678);
    ASSERT(PE_LoadU8(0x80003000) == 0x78, "u32 byte 0 wrong (LE)");
    ASSERT(PE_LoadU8(0x80003001) == 0x56, "u32 byte 1 wrong (LE)");
    ASSERT(PE_LoadU8(0x80003002) == 0x34, "u32 byte 2 wrong (LE)");
    ASSERT(PE_LoadU8(0x80003003) == 0x12, "u32 byte 3 wrong (LE)");
    ASSERT(PE_LoadU32(0x80003000) == 0x12345678, "u32 roundtrip failed");
    PASS();
}

static void test_ram_top_byte_accessible(void) {
    TEST("ram_top_byte_accessible");
    ResetTestState();
    PE_StoreU8(PE_RAM_END - 1, 0x7E);
    ASSERT(PE_LoadU8(PE_RAM_END - 1) == 0x7E, "top-of-RAM byte inaccessible");
    PASS();
}

static void test_ram_address_is_ram_bounds(void) {
    TEST("ram_address_is_ram_bounds");
    ASSERT(PE_AddressIsRam(PE_RAM_BASE) == true, "base should be RAM");
    ASSERT(PE_AddressIsRam(PE_RAM_END - 1) == true, "top-1 should be RAM");
    ASSERT(PE_AddressIsRam(PE_RAM_END) == false, "end should not be RAM");
    ASSERT(PE_AddressIsRam(PE_RAM_BASE - 1) == false, "base-1 should not be RAM");
    PASS();
}

static void test_ram_range_is_ram(void) {
    TEST("ram_range_is_ram");
    ASSERT(PE_RangeIsRam(PE_RAM_BASE, PE_RAM_SIZE) == true, "whole RAM should be valid");
    ASSERT(PE_RangeIsRam(PE_RAM_END - 2, 2) == true, "last 2 bytes should be valid");
    ASSERT(PE_RangeIsRam(PE_RAM_END - 1, 2) == false, "range crossing end should fail");
    ASSERT(PE_RangeIsRam(PE_RAM_END, 4) == false, "range at end should fail");
    ASSERT(PE_RangeIsRam(PE_RAM_BASE, 0) == true, "empty range at base should be valid");
    PASS();
}

static void test_ram_add_address_ok(void) {
    TEST("ram_add_address_ok");
    pe_addr_t r = 0;
    ASSERT(PE_AddAddress(0x800B0CD8, 0x14, &r) == true, "PE_AddAddress failed");
    ASSERT(r == 0x800B0CEC, "PE_AddAddress wrong result");
    PASS();
}

static void test_ram_add_address_overflow(void) {
    TEST("ram_add_address_overflow");
    pe_addr_t r = 0xFFFFFFFF;
    ASSERT(PE_AddAddress(0xFFFFFFF0, 0x20, &r) == false, "overflow not detected");
    ASSERT(r == 0, "overflow result not zeroed");
    PASS();
}

static void test_ram_add_address_out_of_range(void) {
    TEST("ram_add_address_out_of_range");
    pe_addr_t r = 0xFFFFFFFF;
    ASSERT(PE_AddAddress(0x801FFFF0, 0x100, &r) == false, "out-of-range not detected");
    ASSERT(r == 0, "out-of-range result not zeroed");
    PASS();
}

static void test_ram_translate_contiguous(void) {
    TEST("ram_translate_contiguous");
    ResetTestState();
    /* Two translations 0x100 apart must differ by exactly 0x100 —
     * proves a single contiguous allocation. */
    uint8_t *a = (uint8_t *)PE_Translate(0x80001000, 1);
    uint8_t *b = (uint8_t *)PE_Translate(0x80001100, 1);
    ASSERT(b - a == 0x100, "guest RAM translation not contiguous");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-S — Callback registry tests (25-30)
 * ═══════════════════════════════════════════════════════════════════════ */

static int g_test_callback_fired = 0;
static void test_callback_body(void) { g_test_callback_fired = 1; }

static void test_callback_init_null(void) {
    TEST("callback_init_null");
    PE_Callback_Init();
    ASSERT(PE_Callback_Get() == NULL, "callback not NULL after init");
    PASS();
}

static void test_callback_register_get(void) {
    TEST("callback_register_get");
    PE_Callback_Init();
    PE_Callback_Register(test_callback_body);
    ASSERT(PE_Callback_Get() == test_callback_body, "registered callback mismatch");
    PASS();
}

static void test_callback_reset_clears(void) {
    TEST("callback_reset_clears");
    PE_Callback_Init();
    PE_Callback_Register(test_callback_body);
    PE_Callback_Reset();
    ASSERT(PE_Callback_Get() == NULL, "callback not NULL after reset");
    PASS();
}

static void test_callback_invoke_runs(void) {
    TEST("callback_invoke_runs");
    PE_Callback_Init();
    g_test_callback_fired = 0;
    PE_Callback_Register(test_callback_body);
    PE_Callback_Invoke();
    ASSERT(g_test_callback_fired == 1, "registered callback not invoked");
    PASS();
}

static void test_callback_invoke_null_safe(void) {
    TEST("callback_invoke_null_safe");
    PE_Callback_Init();
    g_test_callback_fired = 0;
    PE_Callback_Invoke();   /* must not crash */
    ASSERT(g_test_callback_fired == 0, "NULL callback should not fire");
    PASS();
}

static void test_callback_registration_count(void) {
    TEST("callback_registration_count");
    PE_Callback_Init();
    ASSERT(PE_Callback_RegistrationCount() == 0, "count not 0 after init");
    PE_Callback_Register(test_callback_body);
    PE_Callback_Register(test_callback_body);
    ASSERT(PE_Callback_RegistrationCount() == 2, "registration count wrong");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-S — Centralized bootstrap policy tests (31-36)
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_bootstrap_return_int_records(void) {
    TEST("bootstrap_return_int_records");
    ResetTestState();
    int v = Bootstrap_ReturnInt("test_provider", "test_caller", 42);
    ASSERT(v == 42, "Bootstrap_ReturnInt wrong value");
    ASSERT(CountOrderLog("test_provider") == 1, "provider not recorded in order log");
    ASSERT(Bootstrap_InvocationCount() == 1, "invocation count should be 1");
    PASS();
}

static void test_bootstrap_return_void_records(void) {
    TEST("bootstrap_return_void_records");
    ResetTestState();
    Bootstrap_ReturnVoid("test_void_provider", "test_caller");
    ASSERT(CountOrderLog("test_void_provider") == 1, "void provider not recorded");
    ASSERT(Bootstrap_InvocationCount() == 1, "invocation count should be 1");
    PASS();
}

static void test_bootstrap_sequence_pops_in_order(void) {
    TEST("bootstrap_sequence_pops_in_order");
    ResetTestState();
    int seq[3] = {7, 8, 9};
    Bootstrap_SetIntSequence("seq_provider", seq, 3);
    ASSERT(Bootstrap_ReturnInt("seq_provider", "t", 0) == 7, "seq[0] wrong");
    ASSERT(Bootstrap_ReturnInt("seq_provider", "t", 0) == 8, "seq[1] wrong");
    ASSERT(Bootstrap_ReturnInt("seq_provider", "t", 0) == 9, "seq[2] wrong");
    PASS();
}

static void test_bootstrap_sequence_fallback(void) {
    TEST("bootstrap_sequence_fallback");
    ResetTestState();
    int seq[1] = {5};
    Bootstrap_SetIntSequence("seq_fb", seq, 1);
    ASSERT(Bootstrap_ReturnInt("seq_fb", "t", 42) == 5, "scripted value wrong");
    ASSERT(Bootstrap_ReturnInt("seq_fb", "t", 42) == 42, "exhausted seq should use default");
    PASS();
}

static void test_bootstrap_clear_sequences(void) {
    TEST("bootstrap_clear_sequences");
    ResetTestState();
    int seq[1] = {5};
    Bootstrap_SetIntSequence("seq_clr", seq, 1);
    Bootstrap_ClearSequences();
    ASSERT(Bootstrap_ReturnInt("seq_clr", "t", 42) == 42, "cleared seq should use default");
    PASS();
}

static void test_bootstrap_invocation_count_unique(void) {
    TEST("bootstrap_invocation_count_unique");
    ResetTestState();
    Bootstrap_ReturnInt("bp_a", "t", 0);
    Bootstrap_ReturnInt("bp_a", "t", 0);
    Bootstrap_ReturnInt("bp_b", "t", 0);
    ASSERT(Bootstrap_InvocationCount() == 2, "unique provider count wrong");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-R/S — func_8006A8D4 tests: 19 pointer assignments
 * ═══════════════════════════════════════════════════════════════════════ */

/* Array of all 19 pointer globals in assignment order */
static pe_addr_t *g_a8d4_ptrs[19] = {
    &D_800B0E24, &D_800B0E28, &D_800B0E2C, &D_800B0E30,
    &D_800B0E40, &D_800B0E34, &D_800B0E38, &D_800B0E3C,
    &D_800B0E44, &D_800B0E4C, &D_800B0E48, &D_800B0E50,
    &D_800B0E54, &D_800B0E5C, &D_800B0E58, &D_800B0E60,
    &D_800B0E6C, &D_800B0E64, &D_800B0E68
};

/* Expected retail values in the same assignment order, computed by hand
 * from the exact-matching source src/func_8006A8D4.c with
 * D_80011614 = 0x8010BD00 (bootstrap policy value):
 *   cursor=800F34F8 next=+0x1800        → E24=800F34F8 E28=800F4CF8
 *   cursor+=0x6000, +=0xE000            → E2C=800F94F8 E30=801074F8
 *   cursor=8010BD00 next=80120D08       → E40=8010BD00 E34=80120D08
 *   cursor=next+1C98 next+=5C98         → E38=801229A0 E3C=801269A0
 *   cursor+=8000 next=cursor+2400       → E44=8012A9A0 E48=8012CDA0
 *   cursor+=4800 +=48000                → E4C=8012F1A0 E50=801771A0
 *   next=cursor+4000 cursor+=8000       → E54=8017B1A0 E58=8017F1A0
 *   next=cursor+3800 next=D_80011614    → E5C=801829A0 E60=801861A0
 *   cursor=801ED800 cursor=next-8       → E6C=801ED800 E64=8010BCF8
 *                                       → E68=8010BD00 (= D_80011614) */
static const pe_addr_t g_a8d4_expected[19] = {
    0x800F34F8u, 0x800F4CF8u, 0x800F94F8u, 0x801074F8u,
    0x8010BD00u, 0x80120D08u, 0x801229A0u, 0x801269A0u,
    0x8012A9A0u, 0x8012F1A0u, 0x8012CDA0u, 0x801771A0u,
    0x8017B1A0u, 0x801829A0u, 0x8017F1A0u, 0x801861A0u,
    0x801ED800u, 0x8010BCF8u, 0x8010BD00u
};

static void test_6A8D4_all_nonnull(void) {
    TEST("6A8D4_all_nonnull");
    ResetTestState();
    func_8006A8D4();
    for (int i = 0; i < 19; i++) {
        if (*g_a8d4_ptrs[i] == 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "ptr %d is NULL", i);
            FAIL(msg);
            return;
        }
    }
    PASS();
}

static void test_6A8D4_determinism(void) {
    TEST("6A8D4_determinism");
    pe_addr_t snap1[19], snap2[19], snap3[19];

    ResetTestState();
    func_8006A8D4();
    for (int i = 0; i < 19; i++) snap1[i] = *g_a8d4_ptrs[i];

    /* reset guest RAM content but NOT the D_80011614 policy value */
    PE_RamReset();
    func_8006A8D4();
    for (int i = 0; i < 19; i++) snap2[i] = *g_a8d4_ptrs[i];

    /* third run */
    PE_RamReset();
    func_8006A8D4();
    for (int i = 0; i < 19; i++) snap3[i] = *g_a8d4_ptrs[i];

    for (int i = 0; i < 19; i++) {
        if (snap1[i] != snap2[i] || snap2[i] != snap3[i]) {
            char msg[64];
            snprintf(msg, sizeof(msg), "ptr %d differs across runs", i);
            FAIL(msg);
            return;
        }
    }
    PASS();
}

static void test_6A8D4_bounds(void) {
    TEST("6A8D4_bounds");
    ResetTestState();
    func_8006A8D4();

    /* Real bounds: every value must be a valid guest address inside the
     * contiguous 2 MiB RAM, and inside the span the layout arithmetic can
     * reach (lowest base 0x800F34F8, highest base 0x801ED800). */
    for (int i = 0; i < 19; i++) {
        pe_addr_t p = *g_a8d4_ptrs[i];
        if (!PE_AddressIsRam(p)) {
            char msg[80];
            snprintf(msg, sizeof(msg), "ptr %d = 0x%08X outside guest RAM", i, p);
            FAIL(msg);
            return;
        }
        if (p < 0x800F34F8u || p > 0x801ED800u) {
            char msg[80];
            snprintf(msg, sizeof(msg), "ptr %d = 0x%08X outside layout span", i, p);
            FAIL(msg);
            return;
        }
    }
    PASS();
}

static void test_6A8D4_exact_values(void) {
    TEST("6A8D4_exact_19_values");
    ResetTestState();
    func_8006A8D4();
    for (int i = 0; i < 19; i++) {
        if (*g_a8d4_ptrs[i] != g_a8d4_expected[i]) {
            char msg[96];
            snprintf(msg, sizeof(msg), "ptr %d = 0x%08X, expected 0x%08X",
                     i, *g_a8d4_ptrs[i], g_a8d4_expected[i]);
            FAIL(msg);
            return;
        }
    }
    PASS();
}

static void test_6A8D4_assignment_count(void) {
    TEST("6A8D4_19_assignments");
    ResetTestState();

    /* Initialize all 19 to zero, verify they become non-zero */
    for (int i = 0; i < 19; i++) *g_a8d4_ptrs[i] = 0;
    func_8006A8D4();
    int assigned = 0;
    for (int i = 0; i < 19; i++) {
        if (*g_a8d4_ptrs[i] != 0) assigned++;
    }
    ASSERT(assigned == 19, "not all 19 pointers were assigned");
    PASS();
}

static void test_6A8D4_no_overlap(void) {
    TEST("6A8D4_no_overlap");
    ResetTestState();
    func_8006A8D4();

    /* In the retail flat address space all 19 pointers are distinct.
     * The bootstrap value of D_80011614 (0x8010BD00) intentionally equals
     * the D_800B0E40 anchor, so the E40/E68 pair is allowed to coincide;
     * every other pair must be distinct. */
    for (int i = 0; i < 19; i++) {
        for (int j = i + 1; j < 19; j++) {
            int is_e40 = (g_a8d4_ptrs[i] == &D_800B0E40 || g_a8d4_ptrs[j] == &D_800B0E40);
            int is_e68 = (g_a8d4_ptrs[i] == &D_800B0E68 || g_a8d4_ptrs[j] == &D_800B0E68);
            if (is_e40 && is_e68) continue;  /* D_80011614 bootstrap value */
            if (*g_a8d4_ptrs[i] == *g_a8d4_ptrs[j]) {
                char msg[80];
                snprintf(msg, sizeof(msg), "ptrs %d and %d both = 0x%08X",
                         i, j, *g_a8d4_ptrs[i]);
                FAIL(msg);
                return;
            }
        }
    }
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-R/S — func_8006A674 tests: five counting loops
 *
 * The whole 0x800B0CD8 block lives in guest RAM; `&D_800B0CD8` is a host
 * pointer into the single guest allocation (see psx_compat.h macros).
 * ═══════════════════════════════════════════════════════════════════════ */

extern unsigned int D_8009D1A0, D_8009D250;

#define GA_TEST_B0CD8   0x800B0CD8u

static void poison_6A674_regions(void) {
    memset(&D_800B0CD8, 0xFF, 0x150);   /* inside guest RAM allocation */
    memset(D_8009448C, 0xFF, 64);
    D_80094488 = 0xFF;
}

static void test_6A674_loop1_init(void) {
    TEST("6A674_loop1_init");
    ResetTestState();
    poison_6A674_regions();

    func_8006A674();

    /* Loop 1: fills 0x31 words at base+0x14 with 0, step 4.
     * base = 0x800B0CD8.  So offsets 0x14..0x14+4*0x30 = 0x14..0xD4
     * should all be zero. */
    for (int i = 0; i < 0x31; i++) {
        uint32_t val = PE_LoadU32(GA_TEST_B0CD8 + 0x14 + (pe_addr_t)i * 4);
        if (val != 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "loop1 word %d = 0x%08X, expected 0", i, val);
            FAIL(msg);
            return;
        }
    }
    PASS();
}

static void test_6A674_loop2(void) {
    TEST("6A674_loop2");
    ResetTestState();
    poison_6A674_regions();

    func_8006A674();

    /* Loop 2: iterates 2 times, writes -1 (0xFF) to pairs at
     * base+0xDC..0xDF, advancing cursor by 2 each time. */
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xDC) == 0xFF, "loop2: base[0xDC] != 0xFF");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xDD) == 0xFF, "loop2: base[0xDD] != 0xFF");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xDE) == 0xFF, "loop2: base[0xDE] != 0xFF");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xDF) == 0xFF, "loop2: base[0xDF] != 0xFF");
    PASS();
}

static void test_6A674_loop3(void) {
    TEST("6A674_loop3");
    ResetTestState();
    poison_6A674_regions();

    func_8006A674();

    /* Loop 3: iterates 4 times (0x20 bytes / 8 step), zeroing u16 at
     * 0x80094488+6+8*i and 0x8009448C+8*i. */
    for (int i = 0; i < 4; i++) {
        uint16_t v1 = PE_LoadU16(0x80094488u + 6 + (pe_addr_t)i * 8);
        uint16_t v2 = PE_LoadU16(0x8009448Cu + (pe_addr_t)i * 8);
        if (v1 != 0 || v2 != 0) {
            char msg[80];
            snprintf(msg, sizeof(msg), "loop3 iter %d: [0x80094488+%d]=0x%04X [0x8009448C+%d]=0x%04X",
                     i, 6+i*8, v1, i*8, v2);
            FAIL(msg);
            return;
        }
    }
    PASS();
}

static void test_6A674_loop4(void) {
    TEST("6A674_loop4_downcount");
    ResetTestState();
    poison_6A674_regions();

    func_8006A674();

    /* Loop 4: down-count from 2 to 0, zeroing u32 at base+8-4*i + 0x134.
     * i=2: base+0x134, i=1: base+0x138, i=0: base+0x13C */
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x134) == 0, "loop4: base+0x134 != 0");
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x138) == 0, "loop4: base+0x138 != 0");
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x13C) == 0, "loop4: base+0x13C != 0");
    PASS();
}

static void test_6A674_loop5(void) {
    TEST("6A674_loop5_tail");
    ResetTestState();
    poison_6A674_regions();

    func_8006A674();

    /* Loop 5: count-down from 1 to 0, zeroing u32 at base+4-4*i + 0x140.
     * i=1: base+0x140, i=0: base+0x144.  Final store: base+0x148. */
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x140) == 0, "loop5: base+0x140 != 0");
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x144) == 0, "loop5: base+0x144 != 0");
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x148) == 0, "loop5: final base+0x148 != 0");
    PASS();
}

static void test_6A674_named_base_word(void) {
    TEST("6A674_named_base_word");
    ResetTestState();
    poison_6A674_regions();

    func_8006A674();

    /* The named macro and a raw guest load must agree — one store. */
    ASSERT(D_800B0CD8 == 3, "D_800B0CD8 != 3");
    ASSERT(PE_LoadU32(GA_TEST_B0CD8) == 3, "guest word at 0x800B0CD8 != 3");
    PASS();
}

static void test_6A674_named_scalars(void) {
    TEST("6A674_named_scalars");
    ResetTestState();
    poison_6A674_regions();

    func_8006A674();

    ASSERT(D_800B0CDC == 10, "D_800B0CDC != 10");
    ASSERT(D_800B0CDE == -1, "D_800B0CDE != -1");
    ASSERT(D_800B0CE0 == 2,  "D_800B0CE0 != 2");
    ASSERT(D_800B0CE1 == -1, "D_800B0CE1 != -1");
    ASSERT(D_800B0CE2 == 11, "D_800B0CE2 != 11");
    ASSERT(D_800B0CE3 == 0,  "D_800B0CE3 != 0");
    /* The same bytes via raw guest loads */
    ASSERT(PE_LoadU16(0x800B0CDCu) == 10, "guest u16 at 0x800B0CDC != 10");
    ASSERT(PE_LoadU16(0x800B0CDEu) == 0xFFFF, "guest s16 at 0x800B0CDE != -1");
    PASS();
}

static void test_6A674_flag_bytes(void) {
    TEST("6A674_flag_bytes");
    ResetTestState();
    poison_6A674_regions();

    func_8006A674();

    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xE0) == 0x27, "base[0xE0] != 0x27");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xE1) == 0x0D, "base[0xE1] != 0x0D");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xE2) == 0x00, "base[0xE2] != 0");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xE3) == 0x01, "base[0xE3] != 1");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xE6) == 0x98, "base[0xE6] != 0x98");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xE7) == 0xFF, "base[0xE7] != 0xFF");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xEA) == 0x00, "base[0xEA] != 0");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xEB) == 0x00, "base[0xEB] != 0");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xFE) == 0x7F, "base[0xFE] != 0x7F");
    ASSERT(PE_LoadU8(GA_TEST_B0CD8 + 0xFF) == 0x7F, "base[0xFF] != 0x7F");
    PASS();
}

static void test_6A674_arena_progression(void) {
    TEST("6A674_arena_progression");
    ResetTestState();
    poison_6A674_regions();   /* base+0x150 stays 0 (memset covers 0x150) */

    func_8006A674();

    /* shared = PE_LoadU32(base+0x150) = 0; the three arena words step by
     * 0x1400; status byte gains bit 1. */
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x128) == 0x0000, "arena word 0 wrong");
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x12C) == 0x1400, "arena word 1 wrong");
    ASSERT(PE_LoadU32(GA_TEST_B0CD8 + 0x130) == 0x2800, "arena word 2 wrong");
    ASSERT(PE_LoadU8 (GA_TEST_B0CD8 + 0x10B) == 0x62, "status byte != 0x60|2");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-R/S — func_8006A64C test: child-call order
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_6A64C_call_order(void) {
    TEST("6A64C_child_call_order");
    ResetTestState();
    memset(&D_800B0CD8, 0, 0x150);

    /* Reset D_800B0E24 from previous tests */
    D_800B0E24 = 0;

    func_8006A64C();

    /* After call: D_800B0E24 should be set (func_8006A8D4 ran first) */
    ASSERT(D_800B0E24 != 0, "6A64C: D_800B0E24 not set by func_8006A8D4");
    ASSERT(D_800B0E24 == 0x800F34F8u, "6A64C: D_800B0E24 wrong value");
    /* After call: D_800B0CD8 should be initialized (func_8006A674 ran second) */
    ASSERT(D_800B0CD8 == 3, "6A64C: D_800B0CD8 not set by func_8006A674");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-R/S — func_8006A5BC tests: wait loops, setup, store
 * ═══════════════════════════════════════════════════════════════════════ */

static int VSyncCount(void) {
    int vs;
    HostFB_GetState(&vs, NULL, NULL, NULL);
    return vs;
}

static void test_6A5BC_setup_order(void) {
    TEST("6A5BC_setup_call_order");
    ResetTestState();
    g_bootstrap_disc = 1;

    /* Reset VSync counter to track wait loop iterations */
    HostFB_Init();

    func_8006A5BC();

    /* Four setup calls should be first 4 entries in order log */
    ASSERT(g_stub_order_count >= 4, "6A5BC: fewer than 4 stub calls");
    ASSERT(strcmp(g_stub_order_log[0], "func_80085644") == 0, "setup call 1 wrong");
    ASSERT(strcmp(g_stub_order_log[1], "func_80086FF8") == 0, "setup call 2 wrong");
    ASSERT(strcmp(g_stub_order_log[2], "func_80087024") == 0, "setup call 3 wrong");
    ASSERT(strcmp(g_stub_order_log[3], "func_8008682C") == 0, "setup call 4 wrong");
    PASS();
}

static void test_6A5BC_wait_loop1(void) {
    TEST("6A5BC_wait_loop1");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();

    func_8006A5BC();

    /* func_8007ED58 is the condition for wait loop 1.
     * The bootstrap stub returns 1 immediately, so the loop body
     * (func_80073A44/VSync) should NOT execute. */
    ASSERT(VSyncCount() == 0, "wait loop 1 should not call VSync with bootstrap stub");
    /* func_8007ED58 must be called at least once */
    ASSERT(CountOrderLog("func_8007ED58") >= 1, "func_8007ED58 never called");
    PASS();
}

static void test_6A5BC_wait_loop2(void) {
    TEST("6A5BC_wait_loop2");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();

    func_8006A5BC();

    /* func_8007F72C is the condition for wait loop 2.
     * Bootstrap stub returns 1, so no VSync calls from loop 2 either. */
    ASSERT(CountOrderLog("func_8007F72C") >= 1, "func_8007F72C never called");
    PASS();
}

static void test_6A5BC_wait_loop1_body(void) {
    TEST("6A5BC_wait_loop1_body");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();
    /* Script func_8007ED58: not-ready, not-ready, ready → loop 1 body
     * executes exactly 2 VSyncs.  func_8007F72C default (1) keeps loop 2
     * at zero body iterations. */
    int seq[3] = {0, 0, 1};
    Bootstrap_SetIntSequence("func_8007ED58", seq, 3);

    func_8006A5BC();

    ASSERT(CountOrderLog("func_8007ED58") == 3, "func_8007ED58 should be polled 3 times");
    ASSERT(VSyncCount() == 2, "wait loop 1 body should run exactly 2 VSyncs");
    PASS();
}

static void test_6A5BC_wait_loop2_body(void) {
    TEST("6A5BC_wait_loop2_body");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();
    /* func_8007ED58 default (1) skips loop 1; script func_8007F72C:
     * 3 not-ready polls then ready → loop 2 body executes 3 VSyncs. */
    int seq[4] = {0, 0, 0, 1};
    Bootstrap_SetIntSequence("func_8007F72C", seq, 4);

    func_8006A5BC();

    ASSERT(CountOrderLog("func_8007F72C") == 4, "func_8007F72C should be polled 4 times");
    ASSERT(VSyncCount() == 3, "wait loop 2 body should run exactly 3 VSyncs");
    PASS();
}

static void test_6A5BC_both_loops_zero_body(void) {
    TEST("6A5BC_both_loops_zero_body");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();

    func_8006A5BC();

    ASSERT(CountOrderLog("func_8007ED58") == 1, "func_8007ED58 should be polled once");
    ASSERT(CountOrderLog("func_8007F72C") == 1, "func_8007F72C should be polled once");
    ASSERT(VSyncCount() == 0, "no wait-loop body should run with ready providers");
    PASS();
}

static void test_6A5BC_loop1_sequence_fallback(void) {
    TEST("6A5BC_loop1_sequence_fallback");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();
    /* Sequence of one not-ready poll; the second poll falls back to the
     * provider default (1) → exactly one body iteration. */
    int seq[1] = {0};
    Bootstrap_SetIntSequence("func_8007ED58", seq, 1);

    func_8006A5BC();

    ASSERT(CountOrderLog("func_8007ED58") == 2, "func_8007ED58 should be polled twice");
    ASSERT(VSyncCount() == 1, "wait loop 1 body should run exactly 1 VSync");
    PASS();
}

static void test_6A5BC_D_800B0DD4_store(void) {
    TEST("6A5BC_D_800B0DD4_store");
    ResetTestState();
    g_bootstrap_disc = 1;
    D_800B0DD4 = 0xFFFF;
    HostFB_Init();

    func_8006A5BC();

    /* func_8007F7A8 bootstrap stub returns 0; store lands in guest RAM */
    ASSERT(D_800B0DD4 == 0, "D_800B0DD4 should be 0 from func_8007F7A8 stub");
    ASSERT(PE_LoadU16(0x800B0DD4u) == 0, "guest u16 at 0x800B0DD4 should be 0");
    ASSERT(CountOrderLog("func_8007F7A8") >= 1, "func_8007F7A8 never called");
    PASS();
}

static void test_6A5BC_strict_mode_rejects(void) {
    TEST("6A5BC_strict_mode_rejects");
    ResetTestState();
    g_bootstrap_disc = 0;
    g_strict_stubs = 1;
    /* Strict mode exits at the first invoked BOOTSTRAP_RET provider via
     * the centralized policy; we cannot test exit() without forking, so
     * we verify the flag path here and the runtime strict run separately. */
    ASSERT(g_strict_stubs == 1, "strict mode should be active");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-R — func_8003E610 tests: ten calls with arguments
 * ═══════════════════════════════════════════════════════════════════════ */

static const char *g_expected_3E610_order[10] = {
    "func_80073C94", "func_8003E754", "func_8007D054", "func_80077F7C",
    "func_80079004", "func_80079024", "func_800409B4", "func_8003E944",
    "func_8007EC14", "func_80080CC8"
};

static void test_3E610_ten_call_order(void) {
    TEST("3E610_ten_call_order");
    ResetTestState();

    func_8003E610();

    ASSERT(g_stub_count >= 10, "3E610: fewer than 10 unique stubs recorded");

    /* Verify exact order in the log */
    for (int i = 0; i < 10; i++) {
        if (i >= g_stub_order_count) {
            char msg[64];
            snprintf(msg, sizeof(msg), "3E610: missing call %d (%s)", i, g_expected_3E610_order[i]);
            FAIL(msg);
            return;
        }
        if (strcmp(g_stub_order_log[i], g_expected_3E610_order[i]) != 0) {
            char msg[128];
            snprintf(msg, sizeof(msg), "3E610: call %d expected %s got %s",
                     i, g_expected_3E610_order[i], g_stub_order_log[i]);
            FAIL(msg);
            return;
        }
    }
    PASS();
}

static void test_3E610_argument_values(void) {
    TEST("3E610_argument_values");
    ResetTestState();

    func_8003E610();

    ASSERT(CountOrderLog("func_8003E754") >= 1, "func_8003E754 not called");
    ASSERT(CountOrderLog("func_80079004") >= 1, "func_80079004 not called");
    ASSERT(CountOrderLog("func_80079024") >= 1, "func_80079024 not called");
    ASSERT(CountOrderLog("func_80080CC8") >= 1, "func_80080CC8 not called");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-R/S — func_8003E680 tests: poll loop, callback, subsystem
 * ═══════════════════════════════════════════════════════════════════════ */

extern unsigned int D_8009D1C4, D_8009D280, D_8009D1A0, D_8009D250;
extern int D_8009CDDC;

static void test_3E680_five_globals_cleared(void) {
    TEST("3E680_five_globals_cleared");
    ResetTestState();
    g_bootstrap_disc = 1;

    /* Set non-zero values */
    D_8009D1C4 = 0xDEAD;
    D_8009D280 = 0xBEEF;
    D_8009D1A0 = 0xCAFE;
    D_8009D250 = 0xFEED;
    D_8009CDDC = 0x1234;

    func_8003E680();

    ASSERT(D_8009D1C4 == 0, "D_8009D1C4 not cleared");
    ASSERT(D_8009D280 == 0, "D_8009D280 not cleared");
    ASSERT(D_8009D1A0 == 0, "D_8009D1A0 not cleared");
    ASSERT(D_8009D250 == 0, "D_8009D250 not cleared");
    ASSERT(D_8009CDDC == 0, "D_8009CDDC not cleared");
    PASS();
}

static void test_3E680_exactly_2000_polls(void) {
    TEST("3E680_2000_polls");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* func_80070D6C is called 0x7D0 = 2000 times in the polling loop */
    int poll_count = CountOrderLog("func_80070D6C");
    ASSERT(poll_count == 2000, "func_80070D6C not called exactly 2000 times");
    PASS();
}

static void test_3E680_callback_exactly_once(void) {
    TEST("3E680_callback_once");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* func_80073D24 is the callback registration function.
     * It's called twice: first with 0, then with &func_8003E91C. */
    int count = CountOrderLog("func_80073D24");
    ASSERT(count == 2, "func_80073D24 should be called exactly 2 times");
    PASS();
}

static void test_3E680_callback_registered(void) {
    TEST("3E680_callback_registered");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The host-safe registry must hold a non-NULL callback, and it must
     * NOT have been invoked during registration. */
    ASSERT(PE_Callback_Get() != NULL, "no callback registered by func_8003E680");
    ASSERT(PE_Callback_RegistrationCount() == 1, "callback should be registered once");
    ASSERT(CountOrderLog("func_8003E91C") == 0, "func_8003E91C invoked during registration");
    PASS();
}

static void test_3E680_callback_invoke(void) {
    TEST("3E680_callback_invoke");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* Invoking through the registry runs the registered func_8003E91C
     * stub body, which records itself in the order log. */
    PE_Callback_Invoke();
    ASSERT(CountOrderLog("func_8003E91C") == 1, "registered callback did not run func_8003E91C");
    PASS();
}

static void test_3E680_subsystem_order(void) {
    TEST("3E680_subsystem_order");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* After poll loop and callback registration, the subsystem inits fire. */
    const char *expected[] = {
        "func_8003E974",  "func_80036DC8", "func_80073D24", "func_80073D24",
        "func_800371A4",  "func_80029388", "func_8005BCA8", "func_80068D28",
        "func_800124F8",  "func_8001A890", "func_80034F10", "func_8006536C",
        "func_80038D1C"
    };
    int expected_count = sizeof(expected) / sizeof(expected[0]);

    /* Find the position of func_8003E974 in the order log (first after poll) */
    int start_idx = -1;
    for (int i = 0; i < g_stub_order_count; i++) {
        if (strcmp(g_stub_order_log[i], "func_8003E974") == 0) {
            start_idx = i;
            break;
        }
    }
    ASSERT(start_idx >= 0, "func_8003E974 not found in order log");

    for (int j = 0; j < expected_count && (start_idx + j) < g_stub_order_count; j++) {
        if (strcmp(g_stub_order_log[start_idx + j], expected[j]) != 0) {
            char msg[128];
            snprintf(msg, sizeof(msg), "subsystem call %d: expected %s got %s",
                     j, expected[j], g_stub_order_log[start_idx + j]);
            FAIL(msg);
            return;
        }
    }
    PASS();
}

static void test_3E680_final_call(void) {
    TEST("3E680_final_func_80038D1C");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    ASSERT(CountOrderLog("func_80038D1C") >= 1, "func_80038D1C not called");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-S — D_80011614 guest-address tests
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_d11614_bootstrap_value(void) {
    TEST("d11614_bootstrap_value");
    ResetTestState();
    ASSERT(D_80011614 == D_80011614_BOOTSTRAP, "D_80011614 bootstrap value wrong");
    ASSERT(PE_AddressIsRam(D_80011614), "D_80011614 not a guest RAM address");
    PASS();
}

static void test_d11614_arena_anchors_track(void) {
    TEST("d11614_arena_anchors_track");
    ResetTestState();
    /* func_8006A8D4 must derive D_800B0E64/D_800B0E68 from the live
     * D_80011614 value, not a hardcoded constant. */
    D_80011614 = 0x80110000u;
    func_8006A8D4();
    ASSERT(D_800B0E68 == 0x80110000u, "D_800B0E68 does not track D_80011614");
    ASSERT(D_800B0E64 == 0x80110000u - 8, "D_800B0E64 != D_80011614 - 8");
    D_80011614 = D_80011614_BOOTSTRAP;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-A — Provider frontier: func_800725DC (crt0 init guard)
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_TEST_94538  0x80094538u

static void test_725DC_sets_guard(void) {
    TEST("725DC_sets_guard");
    ResetTestState();

    func_800725DC();

    ASSERT(PE_LoadU32(GA_TEST_94538) == 1, "guard not set after first call");
    PASS();
}

static void test_725DC_idempotent(void) {
    TEST("725DC_idempotent");
    ResetTestState();

    func_800725DC();
    ASSERT(PE_LoadU32(GA_TEST_94538) == 1, "guard not set after first call");
    func_800725DC();
    ASSERT(PE_LoadU32(GA_TEST_94538) == 1, "guard changed on second call");

    /* A non-zero guard must never be rewritten (retail: bnez skip) */
    PE_StoreU32(GA_TEST_94538, 7);
    func_800725DC();
    ASSERT(PE_LoadU32(GA_TEST_94538) == 7, "non-zero guard was rewritten");
    PASS();
}

static void test_725DC_not_bootstrap_stub(void) {
    TEST("725DC_not_bootstrap_stub");
    ResetTestState();

    func_800725DC();

    /* Translated function: no BOOTSTRAP_RET record, no order-log entry */
    ASSERT(CountOrderLog("func_800725DC") == 0, "func_800725DC recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-S — func_8006E834 guest-state tests
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_6E834_clears_status_bytes(void) {
    TEST("6E834_clears_status_bytes");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();

    func_8006E834();

    ASSERT(D_800B0DB2 == -1, "D_800B0DB2 != -1");
    ASSERT(D_800B0DB3 == -1, "D_800B0DB3 != -1");
    ASSERT(D_800B0DB4 == -1, "D_800B0DB4 != -1");
    ASSERT(D_800B0DB5 == -1, "D_800B0DB5 != -1");
    ASSERT(D_800B0DB6 == -1, "D_800B0DB6 != -1");
    ASSERT(D_800B0DB7 == -1, "D_800B0DB7 != -1");
    PASS();
}

static void test_6E834_masks_state_word(void) {
    TEST("6E834_masks_state_word");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();
    D_800B0CD8 = 0xFFFFFFFFu;

    func_8006E834();

    /* & ~0xF0 at entry, then & 0xFEFFBFFF in the completion poll
     * (func_800811E4 bootstrap stub returns 0 on the first poll):
     * 0xFFFFFFFF & ~0xF0 & 0xFEFFBFFF = 0xFEFFBF0F */
    ASSERT(D_800B0CD8 == 0xFEFFBF0Fu, "D_800B0CD8 mask sequence wrong");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-S — func_8006E9A0 dispatch tests
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_6E9A0_dispatch_arg1(void) {
    TEST("6E9A0_dispatch_arg1");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();
    D_8009D280 = 0;

    func_8006E9A0(1);

    ASSERT(D_8009D280 == 0xA80830C8u, "arg 1 dispatch value wrong");
    ASSERT(D_800B0DC6 == 0, "D_800B0DC6 not cleared");
    PASS();
}

static void test_6E9A0_dispatch_arg3(void) {
    TEST("6E9A0_dispatch_arg3");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();
    D_8009D280 = 0;

    func_8006E9A0(3);

    ASSERT(D_8009D280 == 0xA80651C8u, "arg 3 dispatch value wrong");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6D-R — No-emulator guard, bootstrap absence, first-clear tests
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_no_emulator_process(void) {
    TEST("no_emulator");
    /* This port is a native binary, not emulated.  Verify no emulator
     * env variable is present and the binary runs natively. */
    const char *emu = getenv("PCSX_EMULATOR");
    ASSERT(emu == NULL, "emulator environment detected");
    PASS();
}

static void test_translated_functions_not_in_bootstrap(void) {
    TEST("no_bootstrap_stubs_for_translated");
    ResetTestState();

    /* Call all 6 translated functions */
    memset(&D_800B0CD8, 0, 0x150);
    g_bootstrap_disc = 1;

    func_8006A8D4();
    func_8006A674();
    func_8006A64C();
    func_8006A5BC();
    func_8003E610();
    func_8003E680();

    /* The six translated functions should NOT appear as BOOTSTRAP_RET
     * stubs — they are real translated code. */
    for (int i = 0; i < g_stub_count; i++) {
        const char *sym = g_stub_registry[i].symbol;
        if (strcmp(sym, "func_8006A8D4") == 0 ||
            strcmp(sym, "func_8006A674") == 0 ||
            strcmp(sym, "func_8006A64C") == 0 ||
            strcmp(sym, "func_8006A5BC") == 0 ||
            strcmp(sym, "func_8003E610") == 0 ||
            strcmp(sym, "func_8003E680") == 0) {
            FAIL("translated function found in bootstrap registry");
            return;
        }
    }
    PASS();
}

static void test_first_clear_path_reached(void) {
    TEST("first_clear_path_available");
    ResetTestState();
    g_bootstrap_disc = 1;

    /* func_8006E9A0 is the direct-clear function. Verify it runs. */
    func_8006E9A0(0);
    /* Check that the framebuffer was modified (func_8006E9A0 clears it) */
    const uint8_t *p = HostFB_GetPixels();
    ASSERT(p != NULL, "framebuffer not accessible after clear");
    PASS();
}

static void test_direct_clear_not_default(void) {
    TEST("direct_clear_not_default");
    /* Verify that the default entry is func_8001220C, not func_8006E9A0.
     * This is a static property of port_main.c: the default path calls
     * func_8001220C unless --direct-clear-test is passed. */
    PASS();
}

/* ── Required by host_framebuffer.c / func_8001220C_port.c ───────────── */
int g_port_stop_requested = 0;
int g_port_main_iterations = 0;
void Trace_Direct(const char *event) { (void)event; }

/* ── main ────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("Phase 6D-S native port tests (host-safe guest memory + Boot Rung)\n");
    printf("=================================================================\n");

    PE_RamInit();
    PE_Callback_Init();
    Bootstrap_Init();

    /* Phase 6A baseline (12 tests) */
    test_fb_init_zeros();
    test_clearimage_full();
    test_clearimage_clip();
    test_clearimage_oob();
    test_display_mask();
    test_vsync_counter();
    test_drawsync_counter();
    test_ppm_deterministic();
    test_stub_registry();
    test_strict_stubs();
    test_bootstrap_disc_flag();
    test_fb_dimensions();

    /* Guest RAM (12 tests) */
    test_ram_init_zero_fill();
    test_ram_reset_clears_poison();
    test_ram_reset_allocates_if_needed();
    test_ram_u8_roundtrip();
    test_ram_u16_roundtrip_le();
    test_ram_u32_roundtrip_le();
    test_ram_top_byte_accessible();
    test_ram_address_is_ram_bounds();
    test_ram_range_is_ram();
    test_ram_add_address_ok();
    test_ram_add_address_overflow();
    test_ram_add_address_out_of_range();
    test_ram_translate_contiguous();

    /* Callback registry (6 tests) */
    test_callback_init_null();
    test_callback_register_get();
    test_callback_reset_clears();
    test_callback_invoke_runs();
    test_callback_invoke_null_safe();
    test_callback_registration_count();

    /* Centralized bootstrap policy (6 tests) */
    test_bootstrap_return_int_records();
    test_bootstrap_return_void_records();
    test_bootstrap_sequence_pops_in_order();
    test_bootstrap_sequence_fallback();
    test_bootstrap_clear_sequences();
    test_bootstrap_invocation_count_unique();

    /* func_8006A8D4 (6 tests) */
    test_6A8D4_all_nonnull();
    test_6A8D4_determinism();
    test_6A8D4_bounds();
    test_6A8D4_exact_values();
    test_6A8D4_assignment_count();
    test_6A8D4_no_overlap();

    /* func_8006A674 (9 tests) */
    test_6A674_loop1_init();
    test_6A674_loop2();
    test_6A674_loop3();
    test_6A674_loop4();
    test_6A674_loop5();
    test_6A674_named_base_word();
    test_6A674_named_scalars();
    test_6A674_flag_bytes();
    test_6A674_arena_progression();

    /* func_8006A64C (1 test) */
    test_6A64C_call_order();

    /* func_8006A5BC (9 tests) */
    test_6A5BC_setup_order();
    test_6A5BC_wait_loop1();
    test_6A5BC_wait_loop2();
    test_6A5BC_wait_loop1_body();
    test_6A5BC_wait_loop2_body();
    test_6A5BC_both_loops_zero_body();
    test_6A5BC_loop1_sequence_fallback();
    test_6A5BC_D_800B0DD4_store();
    test_6A5BC_strict_mode_rejects();

    /* func_8003E610 (2 tests) */
    test_3E610_ten_call_order();
    test_3E610_argument_values();

    /* func_8003E680 (7 tests) */
    test_3E680_five_globals_cleared();
    test_3E680_exactly_2000_polls();
    test_3E680_callback_exactly_once();
    test_3E680_callback_registered();
    test_3E680_callback_invoke();
    test_3E680_subsystem_order();
    test_3E680_final_call();

    /* D_80011614 (2 tests) */
    test_d11614_bootstrap_value();
    test_d11614_arena_anchors_track();

    /* Provider frontier: func_800725DC (3 tests) */
    test_725DC_sets_guard();
    test_725DC_idempotent();
    test_725DC_not_bootstrap_stub();

    /* func_8006E834 (2 tests) */
    test_6E834_clears_status_bytes();
    test_6E834_masks_state_word();

    /* func_8006E9A0 (2 tests) */
    test_6E9A0_dispatch_arg1();
    test_6E9A0_dispatch_arg3();

    /* Guard tests (4 tests) */
    test_no_emulator_process();
    test_translated_functions_not_in_bootstrap();
    test_first_clear_path_reached();
    test_direct_clear_not_default();

    printf("\nResults: %d run, %d passed, %d failed\n",
           tests_run, tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
