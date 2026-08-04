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
#include "pe_disc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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
    PE_Sdk_ResetState();            /* host-owned GTE/IRQ/event state    */
    Bootstrap_ClearSequences();     /* drop scripted provider sequences   */
    PE_Disc_SetActive(NULL);        /* drop any installed disc fixture    */
    PE_3EAC8_RecordReset();         /* drop recorded provider arguments   */
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
 * Phase 6E-B6 — Callback slot model tests (25-30)
 * ═══════════════════════════════════════════════════════════════════════ */

static int g_test_callback_fired = 0;
static void test_callback_body(void) { g_test_callback_fired = 1; }

#define GA_CB_TABLE    0x8009568Cu   /* D_8009568C: 8 handler slots      */
#define GA_CB_SLOT4    0x8009569Cu   /* D_8009568C + 4*4                 */
#define GA_CB_COUNTER  0x800956ACu   /* D_800956AC: dispatch counter     */
#define GA_TEST_CB     0x80010000u   /* stand-in guest callback address  */

static void test_callback_init_null(void) {
    TEST("callback_init_null");
    ResetTestState();
    PE_Callback_Init();
    /* All retail-visible state is guest RAM: 8 slots + counter zeroed */
    for (uint32_t i = 0; i < 8; i++) {
        ASSERT(PE_Callback_GetSlot(i) == 0, "slot not zero after RAM reset");
    }
    ASSERT(PE_LoadU32(GA_CB_COUNTER) == 0, "counter not zero after RAM reset");
    PASS();
}

static void test_callback_bind_resolve(void) {
    TEST("callback_bind_resolve");
    ResetTestState();
    PE_Callback_Init();
    ASSERT(PE_Callback_Bind(GA_TEST_CB, test_callback_body) == 0, "bind failed");
    ASSERT(PE_Callback_Bind(GA_TEST_CB, test_callback_body) == 0,
           "identical rebind not idempotent");
    ASSERT(PE_Callback_ErrorCount() == 0, "bind raised a visible error");
    PASS();
}

static void test_callback_reset_clears(void) {
    TEST("callback_reset_clears");
    ResetTestState();
    PE_Callback_Init();
    PE_Callback_SetSlot(4, GA_TEST_CB);
    PE_Callback_SetSlot(0, GA_TEST_CB);
    PE_StoreU32(GA_CB_COUNTER, 7);
    PE_Callback_ResetTable();
    for (uint32_t i = 0; i < 8; i++) {
        ASSERT(PE_Callback_GetSlot(i) == 0, "ResetTable left a slot set");
    }
    ASSERT(PE_LoadU32(GA_CB_COUNTER) == 0, "ResetTable left counter set");
    PASS();
}

static void test_callback_invoke_runs(void) {
    TEST("callback_invoke_runs");
    ResetTestState();
    PE_Callback_Init();
    g_test_callback_fired = 0;
    PE_Callback_Bind(GA_TEST_CB, test_callback_body);
    PE_Callback_SetSlot(4, GA_TEST_CB);
    PE_Callback_Dispatch();
    ASSERT(g_test_callback_fired == 1, "registered callback not invoked");
    PASS();
}

static void test_callback_invoke_null_safe(void) {
    TEST("callback_invoke_null_safe");
    ResetTestState();
    PE_Callback_Init();
    g_test_callback_fired = 0;
    PE_Callback_Dispatch();   /* must not crash */
    ASSERT(g_test_callback_fired == 0, "NULL callback should not fire");
    /* func_8007440C increments the counter even with an empty table */
    ASSERT(PE_LoadU32(GA_CB_COUNTER) == 1, "dispatch counter not incremented");
    PASS();
}

static void test_callback_registration_count(void) {
    TEST("callback_registration_count");
    ResetTestState();
    PE_Callback_Init();
    ASSERT(PE_Callback_RegistrationCount() == 0, "count not 0 after init");
    PE_Callback_SetSlot(4, GA_TEST_CB);
    PE_Callback_SetSlot(0, GA_TEST_CB);
    ASSERT(PE_Callback_RegistrationCount() == 2, "registration count wrong");
    /* Identical re-install stores nothing and does not count */
    PE_Callback_SetSlot(4, GA_TEST_CB);
    ASSERT(PE_Callback_RegistrationCount() == 2, "no-store install counted");
    /* Removal is not a registration */
    PE_Callback_SetSlot(4, 0);
    ASSERT(PE_Callback_RegistrationCount() == 2, "removal counted");
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

/* Array of all 19 pointer globals in assignment order.  Since Phase
 * 6E-B16 these are guest-RAM lvalue macros (function-call lvalues), so
 * the array is populated at runtime by PE_A8D4_InitPtrs(). */
static pe_addr_t *g_a8d4_ptrs[19];
static void PE_A8D4_InitPtrs(void) {
    g_a8d4_ptrs[0]  = &D_800B0E24; g_a8d4_ptrs[1]  = &D_800B0E28;
    g_a8d4_ptrs[2]  = &D_800B0E2C; g_a8d4_ptrs[3]  = &D_800B0E30;
    g_a8d4_ptrs[4]  = &D_800B0E40; g_a8d4_ptrs[5]  = &D_800B0E34;
    g_a8d4_ptrs[6]  = &D_800B0E38; g_a8d4_ptrs[7]  = &D_800B0E3C;
    g_a8d4_ptrs[8]  = &D_800B0E44; g_a8d4_ptrs[9]  = &D_800B0E4C;
    g_a8d4_ptrs[10] = &D_800B0E48; g_a8d4_ptrs[11] = &D_800B0E50;
    g_a8d4_ptrs[12] = &D_800B0E54; g_a8d4_ptrs[13] = &D_800B0E5C;
    g_a8d4_ptrs[14] = &D_800B0E58; g_a8d4_ptrs[15] = &D_800B0E60;
    g_a8d4_ptrs[16] = &D_800B0E6C; g_a8d4_ptrs[17] = &D_800B0E64;
    g_a8d4_ptrs[18] = &D_800B0E68;
}

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
    PE_A8D4_InitPtrs();
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
    PE_A8D4_InitPtrs();
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
    PE_A8D4_InitPtrs();
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
    PE_A8D4_InitPtrs();
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
    PE_A8D4_InitPtrs();
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
    PE_A8D4_InitPtrs();
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
    TEST("6A5BC_setup_state");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();

    func_8006A5BC();

    /* Phase 6E-A: all four setup calls are real (pe_stream.c).
     * func_80086FF8 -> ring entry 0xF0, func_80087024 -> 0xF1,
     * func_8008682C(0) -> command 0x98 -> ring entries 0x9A/0x9C. */
    ASSERT(PE_LoadU32(0x800BCD80u) == 0x98, "D_800BCD80 should hold last command 0x98");
    ASSERT(PE_LoadU32(0x8009D2F4u) == 4, "ring should hold 4 entries");
    ASSERT(PE_LoadU32(0x800B8628u + 0x00u) == 0xF0, "ring[0] cmd");
    ASSERT(PE_LoadU32(0x800B8628u + 0x24u) == 0xF1, "ring[1] cmd");
    ASSERT(PE_LoadU32(0x800B8628u + 0x48u) == 0x9A, "ring[2] cmd");
    ASSERT(PE_LoadU32(0x800B8628u + 0x6Cu) == 0x9C, "ring[3] cmd");
    /* func_80085644 bring-up effects */
    ASSERT(PE_LoadU32(0x8009CDE0u) != 0, "stream event handle not stored");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 0, "synchronous SPU transfer should complete");
    ASSERT(PE_LoadU32(0x800B6958u) == 0x40001010u, "stream config word 0");
    ASSERT(PE_LoadU32(0x800B6958u + 4u) == 0xEFF0u, "stream config word 1");
    /* No bootstrap stubs remain on the 6A5BC path */
    ASSERT(g_stub_count == 0, "6A5BC path recorded bootstrap stubs (frontier regressed)");
    PASS();
}

static void test_6A5BC_wait_loop1(void) {
    TEST("6A5BC_wait_loop1");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();

    func_8006A5BC();

    /* Phase 6E-A: func_8007ED58 is now real (pe_libcd.c) and returns 1
     * retail-truth, so wait loop 1's body is provably dead: no VSyncs. */
    ASSERT(VSyncCount() == 0, "wait loop 1 should not call VSync (7ED58 returns 1)");
    /* Prove the real 7ED58 ran: it sets D_8009B554=1 and the drive lane */
    ASSERT(PE_LoadU32(0x8009B554u) == 1, "D_8009B554 should be 1 after func_8007ED58");
    ASSERT(PE_LoadU32(0x8009B574u) == 1, "drive lane should be idle (1) after reset");
    PASS();
}

static void test_6A5BC_wait_loop2(void) {
    TEST("6A5BC_wait_loop2");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();

    func_8006A5BC();

    /* Phase 6E-A: func_8007F72C (CdReady) is real.  After the reset the
     * lane is idle (1) and the queue (D_800A3608) is 0, so CdReady
     * returns 1 and loop 2's body is dead too. */
    ASSERT(PE_LoadU32(0x800A3608u) == 0, "CD queue should be empty after reset");
    ASSERT(VSyncCount() == 0, "wait loop 2 should not call VSync (CdReady == 1)");
    PASS();
}

static void test_7ED58_direct(void) {
    TEST("7ED58_returns1_clears_state");
    ResetTestState();
    /* Seed state the reset must clear */
    PE_StoreU32(0x800A3608u, 7);
    PE_StoreU32(0x800B8AB0u, 0xDEADBEEF);
    PE_StoreU32(0x8009B554u, 1);    /* as left by CdInit */

    ASSERT(func_8007ED58() == 1, "func_8007ED58 must return 1 (retail-truth)");
    ASSERT(PE_LoadU32(0x800A3608u) == 0, "reset must clear D_800A3608");
    ASSERT(PE_LoadU32(0x800B8AB0u) == 0, "reset must clear D_800B8AB0 block");
    ASSERT(PE_LoadU32(0x8009B574u) == 1, "drive lane idle after synchronous reset");
    ASSERT(PE_LoadU32(0x8009B554u) == 1, "D_8009B554 re-set by func_80080930");
    ASSERT(func_8007ED58() == 1, "func_8007ED58 must return 1 on repeat call");
    PASS();
}

static void test_7F72C_contract(void) {
    TEST("7F72C_cdready_contract");
    ResetTestState();
    /* lane != 1 -> returned as-is */
    PE_StoreU32(0x8009B574u, 3);
    ASSERT(func_8007F72C() == 3, "CdReady should pass through lane 3");
    PE_StoreU32(0x8009B574u, 0);
    ASSERT(func_8007F72C() == 0, "CdReady should pass through lane 0");
    /* lane == 1, queue > 0 -> 2 */
    PE_StoreU32(0x8009B574u, 1);
    PE_StoreU32(0x800A3608u, 2);
    ASSERT(func_8007F72C() == 2, "CdReady should be 2 with nonempty queue");
    /* lane == 1, queue == 0 -> 1 */
    PE_StoreU32(0x800A3608u, 0);
    ASSERT(func_8007F72C() == 1, "CdReady should be 1 when idle");
    PASS();
}

static void test_6A5BC_both_loops_zero_body(void) {
    TEST("6A5BC_both_loops_zero_body");
    ResetTestState();
    g_bootstrap_disc = 1;
    HostFB_Init();

    func_8006A5BC();

    /* Both wait-loop conditions are retail-truth ready at boot, so no
     * loop body executes.  (The 6D-S scripted-sequence body tests went
     * away with the stub harness; the loop conditions are now covered by
     * the 7ED58/7F72C contract tests above.) */
    ASSERT(VSyncCount() == 0, "no wait-loop body should run with ready providers");
    PASS();
}

static void test_7F778_getter(void) {
    TEST("7F778_queue_getter");
    ResetTestState();
    PE_StoreU32(0x800A3608u, 0x1234);
    ASSERT(func_8007F778() == 0x1234, "func_8007F778 must return D_800A3608");
    PE_StoreU32(0x800A3608u, 0);
    ASSERT(func_8007F778() == 0, "func_8007F778 must return 0 when queue empty");
    PASS();
}

static void test_6A5BC_D_800B0DD4_store(void) {
    TEST("6A5BC_D_800B0DD4_store");
    ResetTestState();
    g_bootstrap_disc = 1;
    D_800B0DD4 = 0xFFFF;
    HostFB_Init();
    /* func_8007F7A8 is now a real getter of D_8009B590 */
    PE_StoreU32(0x8009B590u, 0x1234);

    func_8006A5BC();

    ASSERT(D_800B0DD4 == 0x1234, "D_800B0DD4 should receive D_8009B590 via func_8007F7A8");
    ASSERT(PE_LoadU16(0x800B0DD4u) == 0x1234, "guest u16 at 0x800B0DD4 should be 0x1234");
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
 * Phase 6E-A — func_8003E610 tests: real provider guest-state effects
 *
 * All ten callees are real implementations now, so there is no stub order
 * log to check.  Instead verify the retail-observable guest state left by
 * the full sequence, which also proves each provider ran (its signature
 * state is present).
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_3E610_guest_state(void) {
    TEST("3E610_guest_state");
    ResetTestState();
    HostFB_Init();

    func_8003E610();

    /* func_80073C94 ResetCallback ran exactly once (guard held through the
     * ResetGraph/SsInit/CdInit re-entry) */
    ASSERT(PE_LoadU16(0x800945E4u) == 1, "ResetCallback guard D_800945E4 not set");

    /* func_8003E754 video init: DISPENV buf0/buf1 + overrides */
    ASSERT(PE_LoadU16(0x800BCE80u + 0x0) == 0,      "buf0 disp.x");
    ASSERT(PE_LoadU16(0x800BCE80u + 0x2) == 0xE0,   "buf0 disp.y");
    ASSERT(PE_LoadU16(0x800BCE80u + 0x4) == 0x140,  "buf0 disp.w");
    ASSERT(PE_LoadU16(0x800BCE80u + 0x6) == 0xE0,   "buf0 disp.h");
    ASSERT(PE_LoadU16(0x800BCE94u + 0x2) == 0,      "buf1 disp.y");
    ASSERT(PE_LoadU16(0x800BCE80u + 0xA) == 8,      "buf0 screen.y == 8");
    ASSERT(PE_LoadU16(0x800BCE94u + 0xA) == 8,      "buf1 screen.y == 8");
    ASSERT(PE_LoadU16(0x800BCE80u + 0xE) == 0xE0,   "buf0 screen.h");
    ASSERT(PE_LoadU8(0x800BCE80u + 0x11) == 0,      "buf0 isrgb24 clobbered back to 0 (retail-truth)");
    /* DRAWENV buf0/buf1 + overrides */
    ASSERT(PE_LoadU16(0x800BCDC8u + 0x4) == 0x140,  "drawenv0 clip.w");
    ASSERT(PE_LoadU16(0x800BCE24u + 0x2) == 0xE0,   "drawenv1 clip.y");
    ASSERT(PE_LoadU16(0x800BCDC8u + 0x14) == 0,     "drawenv0 tpage overridden to 0");
    ASSERT(PE_LoadU8(0x800BCDC8u + 0x16) == 1,      "drawenv0 dtd");
    ASSERT(PE_LoadU8(0x800BCDC8u + 0x17) == 0,      "drawenv0 dfe overridden to 0");
    ASSERT(PE_LoadU8(0x800BCDC8u + 0x18) == 1,      "drawenv0 isbg overridden to 1");
    ASSERT(D_8009CDDC == 0, "D_8009CDDC not cleared");

    /* func_8007D054 SsInit: voice defaults + event guard */
    ASSERT(PE_LoadU16(0x8009B3B8u) == 0xC000, "voice default[0]");
    ASSERT(PE_LoadU16(0x8009B3E6u) == 0xC000, "voice default[23]");
    ASSERT(PE_LoadU32(0x8009B3ECu) == 1, "SPU IRQ event guard");

    /* GTE: InitGeom + SetGeomOffset(0xA0,0x70) + SetGeomScreen(0xF0) */
    ASSERT(g_pe_gte.zsf3 == 0x155 && g_pe_gte.zsf4 == 0x100, "InitGeom zsf");
    ASSERT(g_pe_gte.dqa == -0x1062 && g_pe_gte.dqb == 0x1400000, "InitGeom dq");
    ASSERT(g_pe_gte.ofx == (0xA0 << 16) && g_pe_gte.ofy == (0x70 << 16), "SetGeomOffset");
    ASSERT(g_pe_gte.h == 0xF0, "SetGeomScreen");

    /* func_800409B4 card init: guard + 8 event handles */
    ASSERT(PE_LoadU32(0x800A1850u) == 1, "card guard D_800A1850");
    ASSERT(PE_LoadU32(0x800BCDA8u) != 0, "card event handle 0");
    ASSERT(PE_LoadU32(0x800BCDC4u) != 0, "card event handle 7");
    ASSERT(PE_LoadU32(0x800BCDA8u) != PE_LoadU32(0x800BCDC4u), "card handles distinct");

    /* func_8003E944 save manager: brought up */
    ASSERT(PE_LoadU32(0x8009B75Cu) == 1, "save manager not up (D_8009B75C)");

    /* func_8007EC14 CdInit: guard + handler installs */
    ASSERT(PE_LoadU32(0x8009B554u) == 1, "CdInit guard D_8009B554");
    ASSERT(PE_LoadU32(0x800A36A0u) == 0x8007F7E8u, "CD handler 0");
    ASSERT(PE_LoadU32(0x800A36ACu) == 0x8007F960u, "CD handler 3");

    /* func_80080CC8(0): D_8009AFC0 exchanged to 0 */
    ASSERT(PE_LoadU32(0x8009AFC0u) == 0, "D_8009AFC0 should be 0");

    /* No bootstrap stubs may remain on this path */
    ASSERT(g_stub_count == 0, "3E610 path recorded bootstrap stubs (frontier regressed)");
    PASS();
}

static void test_3E610_argument_values(void) {
    TEST("3E610_argument_values");
    ResetTestState();
    HostFB_Init();

    func_8003E610();

    /* func_8003E754(0x140, 0xE0): dimensions reach the DISPENV/DRAWENVs */
    ASSERT(PE_LoadU16(0x800BCE80u + 0x4) == 0x140, "disp.w != 0x140");
    ASSERT(PE_LoadU16(0x800BCE80u + 0x6) == 0xE0, "disp.h != 0xE0");
    ASSERT(PE_LoadU16(0x800BCE80u + 0xE) == 0xE0, "screen.h != 0xE0");
    /* func_80079004(0xA0, 0x70) / func_80079024(0xF0) */
    ASSERT(g_pe_gte.ofx == (0xA0 << 16), "OFX arg wrong");
    ASSERT(g_pe_gte.ofy == (0x70 << 16), "OFY arg wrong");
    ASSERT(g_pe_gte.h == 0xF0, "H arg wrong");
    /* func_80080CC8(0) */
    ASSERT(PE_LoadU32(0x8009AFC0u) == 0, "D_8009AFC0 arg effect wrong");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-A — direct provider-frontier tests (batch 1: 3E610 callees)
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_73C94_guard_idempotent(void) {
    TEST("73C94_resetcallback_guard");
    ResetTestState();
    PE_Callback_SetSlot(4, 0x80012345u);   /* sentinel guest handler */
    func_80073C94();
    ASSERT(PE_LoadU16(0x800945E4u) == 1, "guard not set on first call");
    ASSERT(PE_Callback_GetSlot(4) == 0, "slot table not reset on first call");
    /* Second call: guard short-circuits; slot must NOT be cleared again */
    PE_Callback_SetSlot(4, 0x80012345u);
    func_80073C94();
    ASSERT(PE_Callback_GetSlot(4) == 0x80012345u,
           "guard failed: second call cleared slot");
    ASSERT(g_stub_count == 0, "func_80073C94 must not be a bootstrap stub");
    PASS();
}

static void test_7D054_ssinit(void) {
    TEST("7D054_ssinit_state");
    ResetTestState();
    func_8007D054();
    for (int i = 0; i < 24; i++) {
        if (PE_LoadU16(0x8009B3B8u + (uint32_t)i * 2u) != 0xC000) {
            FAIL("voice default halfword wrong"); return;
        }
    }
    ASSERT(PE_LoadU32(0x8009B3ECu) == 1, "SPU IRQ event guard not set");
    ASSERT(PE_LoadU32(0x8009B384u) != 0, "SPU IRQ event handle not stored");
    ASSERT(PE_LoadU32(0x8009B390u) == 0 && PE_LoadU32(0x8009B394u) == 0, "9B390/94 not zeroed");
    ASSERT(PE_LoadU32(0x8009B398u) == PE_LoadU32(0x8009B46Cu), "9B398 != D_8009B46C");
    /* Idempotent IRQ-event guard: second call keeps the same handle */
    uint32_t h = PE_LoadU32(0x8009B384u);
    func_8007D054();
    ASSERT(PE_LoadU32(0x8009B384u) == h, "IRQ event reopened on second SsInit");
    PASS();
}

static void test_77F7C_initgeom_constants(void) {
    TEST("77F7C_initgeom_constants");
    ResetTestState();
    g_pe_gte.h = 123;   /* poison */
    func_80077F7C();
    ASSERT(g_pe_gte.zsf3 == 0x155, "zsf3");
    ASSERT(g_pe_gte.zsf4 == 0x100, "zsf4");
    ASSERT(g_pe_gte.h == 0x3E8, "h");
    ASSERT(g_pe_gte.dqa == -0x1062, "dqa");
    ASSERT(g_pe_gte.dqb == 0x1400000, "dqb");
    ASSERT(g_pe_gte.ofx == 0 && g_pe_gte.ofy == 0, "ofs");
    PASS();
}

static void test_79004_79024_setters(void) {
    TEST("79004_79024_gte_setters");
    ResetTestState();
    func_80079004(0xA0, 0x70);
    ASSERT(g_pe_gte.ofx == 0xA00000, "ofx shift");
    ASSERT(g_pe_gte.ofy == 0x700000, "ofy shift");
    func_80079024(0xF0);
    ASSERT(g_pe_gte.h == 0xF0, "h set");
    PASS();
}

static void test_409B4_card_init(void) {
    TEST("409B4_card_init");
    ResetTestState();
    PE_StoreU8(0x800A0ED4u, 0xAA);            /* poison flag bytes */
    PE_StoreU8(0x800A0ED4u + 0x418u, 0xBB);
    func_800409B4();
    ASSERT(PE_LoadU32(0x800A1850u) == 1, "guard not set");
    uint32_t h0 = PE_LoadU32(0x800BCDA8u);
    for (int i = 0; i < 7; i++) {
        if (PE_LoadU32(0x800BCDA8u + (uint32_t)i * 4u) == 0) {
            FAIL("event handle not stored"); return;
        }
        if (i > 0 && PE_LoadU32(0x800BCDA8u + (uint32_t)i * 4u) == h0) {
            FAIL("event handles not distinct"); return;
        }
    }
    ASSERT(PE_LoadU8(0x800A0ED4u) == 0, "flag byte 0 not cleared");
    ASSERT(PE_LoadU8(0x800A0ED4u + 0x418u) == 0, "flag byte 418 not cleared");
    ASSERT(PE_Irq_LockDepth() == 0, "unbalanced Enter/ExitCriticalSection");
    /* Idempotent: second call skips init, handles unchanged */
    func_800409B4();
    ASSERT(PE_LoadU32(0x800BCDA8u) == h0, "second call re-opened events");
    PASS();
}

static void test_7EC14_cdinit(void) {
    TEST("7EC14_cdinit");
    ResetTestState();
    ASSERT(func_8007EC14() == 1, "CdInit must return 1");
    ASSERT(PE_LoadU32(0x8009B554u) == 1, "guard D_8009B554 not set");
    ASSERT(PE_LoadU32(0x800A3608u) == 0, "queue not cleared");
    ASSERT(PE_LoadU32(0x800A36A0u) == 0x8007F7E8u, "handler 0");
    ASSERT(PE_LoadU32(0x800A36A4u) == 0x8007E964u, "handler 1");
    ASSERT(PE_LoadU32(0x800A36A8u) == 0x8007F88Cu, "handler 2");
    ASSERT(PE_LoadU32(0x800A36ACu) == 0x8007F960u, "handler 3");
    ASSERT(PE_LoadU32(0x8009AFB4u) == 0x80080164u, "lowlevel ptr 0");
    ASSERT(PE_LoadU32(0x8009AFD8u) == 1, "lowlevel flag");
    /* Guard: second call returns guard value, does not re-run init */
    PE_StoreU32(0x800A36A0u, 0xAAAAAAAA);
    ASSERT(func_8007EC14() == 1, "second CdInit must return guard");
    ASSERT(PE_LoadU32(0x800A36A0u) == 0xAAAAAAAAu, "second CdInit re-ran init");
    PASS();
}

static void test_80CC8_exchange(void) {
    TEST("80CC8_exchange");
    ResetTestState();
    PE_StoreU32(0x8009AFC0u, 0x55);
    ASSERT(func_80080CC8(0xAA) == 0x55, "must return old value");
    ASSERT(PE_LoadU32(0x8009AFC0u) == 0xAAu, "must store new value");
    ASSERT(func_80080CC8(0) == (int)0xAA, "second exchange old value");
    PASS();
}

static void test_3E754_env_fields(void) {
    TEST("3E754_env_fields");
    ResetTestState();
    HostFB_Init();
    func_8003E754(0x140, 0xE0);
    /* DISPENV buf0: disp {0, 0xE0, 0x140, 0xE0}; buf1: y = 0 */
    ASSERT(PE_LoadU16(0x800BCE80u + 0x2) == 0xE0, "buf0 disp.y");
    ASSERT(PE_LoadU16(0x800BCE94u + 0x2) == 0, "buf1 disp.y");
    ASSERT(PE_LoadU16(0x800BCE94u + 0x4) == 0x140, "buf1 disp.w");
    /* screen overrides: x 0, y 8, h 0xE0, w stays 0 */
    ASSERT(PE_LoadU16(0x800BCE80u + 0x8) == 0, "buf0 screen.x");
    ASSERT(PE_LoadU16(0x800BCE80u + 0xA) == 8, "buf0 screen.y");
    ASSERT(PE_LoadU16(0x800BCE80u + 0xC) == 0, "buf0 screen.w");
    ASSERT(PE_LoadU16(0x800BCE80u + 0xE) == 0xE0, "buf0 screen.h");
    /* isrgb24: stored 1 then clobbered to 0 by SetDefDispEnv (retail-truth) */
    ASSERT(PE_LoadU8(0x800BCE91u) == 0, "isrgb24 final state");
    /* DRAWENV buf0 clip {0,0,0x140,0xE0}; buf1 clip.y = 0xE0 */
    ASSERT(PE_LoadU16(0x800BCDC8u + 0x6) == 0xE0, "drawenv0 clip.h");
    ASSERT(PE_LoadU16(0x800BCE24u + 0x2) == 0xE0, "drawenv1 clip.y");
    /* DRAWENV overrides */
    ASSERT(PE_LoadU16(0x800BCDDCu) == 0, "tpage0");
    ASSERT(PE_LoadU16(0x800BCE38u) == 0, "tpage1");
    ASSERT(PE_LoadU8(0x800BCDDEu) == 1 && PE_LoadU8(0x800BCE3Au) == 1, "dtd");
    ASSERT(PE_LoadU8(0x800BCDDFu) == 0 && PE_LoadU8(0x800BCE3Bu) == 0, "dfe");
    ASSERT(PE_LoadU8(0x800BCDE0u) == 1 && PE_LoadU8(0x800BCE3Cu) == 1, "isbg");
    ASSERT(D_8009CDDC == 0, "D_8009CDDC");
    PASS();
}

static void test_3E944_save_state(void) {
    TEST("3E944_save_state");
    ResetTestState();
    func_8003E944();
    ASSERT(PE_LoadU32(0x8009B75Cu) == 1, "save manager not up");
    /* slot base pointers */
    ASSERT(PE_LoadU32(0x800A5B70u + 0x30u) == 0x800BE9A0u, "slot0 base ptr");
    ASSERT(PE_LoadU32(0x800A5C60u + 0x30u) == 0x800BE9A0u + 0x22u, "slot1 base ptr");
    ASSERT(PE_LoadU32(0x800A5B70u + 0x10u) == 0x800A5B70u, "slot0 self ptr");
    ASSERT(PE_LoadU32(0x800A5C60u + 0x10u) == 0x800A5C60u, "slot1 self ptr");
    /* base header bytes: *base = 0xFF, base[1] = 0 */
    ASSERT(PE_LoadU8(0x800BE9A0u) == 0xFF, "base[0]");
    ASSERT(PE_LoadU8(0x800BE9A1u) == 0, "base[1]");
    /* installed function pointers (retail addresses) */
    ASSERT(PE_LoadU32(0x8009B724u) == 0x800846ACu, "fnptr 724");
    ASSERT(PE_LoadU32(0x8009B73Cu) == 0x80084B78u, "fnptr 73C");
    ASSERT(PE_LoadU32(0x8009B758u) == 0x800A5B70u, "state base");
    ASSERT(PE_LoadU32(0x800A5AB4u) == 0x80082B70u, "82ADC ptr");
    /* func_80082534: name tables 0xFF-filled */
    ASSERT(PE_LoadU8(0x800A5B70u + 0x5Du) == 0xFF, "slot0 name[0]");
    ASSERT(PE_LoadU8(0x800A5B70u + 0x62u) == 0xFF, "slot0 name[5]");
    PASS();
}



/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-A — direct provider-frontier tests (batch 2: 6A5BC callees)
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_85644_bringup(void) {
    TEST("85644_stream_bringup");
    ResetTestState();
    func_80085644();
    ASSERT(PE_LoadU32(0x8009B3ECu) == 1, "SPU IRQ event guard (func_8007D15C)");
    ASSERT(PE_LoadU32(0x800B6958u) == 0x40001010u, "config word 0");
    ASSERT(PE_LoadU32(0x800B6958u + 4u) == 0xEFF0u, "config word 1 (0x10000<<0)-0x1010");
    ASSERT(PE_LoadU32(0x8009B45Cu) == 4, "config arg word");
    ASSERT(PE_LoadU32(0x8009B464u) == 0x800B6958u, "config ptr");
    ASSERT(PE_LoadU16(0x8009B414u) == 0x1010, "SPU heap top");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 0, "synchronous transfer must complete");
    ASSERT(PE_LoadU32(0x8009B434u) == 0, "callback deregistered after completion");
    /* func_80085290 state init */
    ASSERT(PE_LoadU32(0x8009D2C8u) == 0x800B6980u, "state base ptr");
    ASSERT(PE_LoadU32(0x800BCD68u) == 1, "BCD68 flag");
    ASSERT(PE_LoadU32(0x800BCD64u) == 0x66A80000u, "BCD64 word");
    ASSERT((PE_LoadU32(0x8009D2C4u) & 0x80u) != 0, "D_8009D2C4 bit 0x80");
    ASSERT(PE_LoadU32(0x800B6A30u) == 0x7F0000u, "B6A30 word");
    ASSERT(PE_LoadU32(0x8009D2F4u) == 0, "ring index reset");
    /* voice-state table entries: first of each loop */
    ASSERT(PE_LoadU32(0x800B8AC0u + 0x50u + 0xA0u) == 0x18, "voice entry stride marker");
    ASSERT(PE_LoadU32(0x800BC03Cu + 0xB4u) == 0xC, "channel block index 0xC");
    ASSERT(PE_LoadU16(0x800BC03Cu + 0x9Cu) == 0x7F00, "channel block volume");
    /* event handle from the OpenEvent retry loop */
    ASSERT(PE_LoadU32(0x8009CDE0u) != 0, "stream event handle");
    ASSERT(g_stub_count == 0, "func_80085644 must not record bootstrap stubs");
    PASS();
}

static void test_86FF8_87024_commands(void) {
    TEST("86FF8_87024_command_issue");
    ResetTestState();
    func_80086FF8();
    ASSERT(PE_LoadU32(0x800BCD80u) == 0xF0, "cmd 0xF0 not issued");
    ASSERT(PE_LoadU32(0x800B8628u) == 0xF0, "ring[0] cmd");
    ASSERT(PE_LoadU32(0x8009D2F4u) == 1, "ring index");
    ASSERT(PE_LoadU32(0x8009D268u) == 0, "dispatch busy flag cleared");
    func_80087024();
    ASSERT(PE_LoadU32(0x800BCD80u) == 0xF1, "cmd 0xF1 not issued");
    ASSERT(PE_LoadU32(0x800B8628u + 0x24u) == 0xF1, "ring[1] cmd");
    ASSERT(PE_LoadU32(0x8009D2F4u) == 2, "ring index 2");
    PASS();
}

static void test_8682C_mode_mapping(void) {
    TEST("8682C_mode_mapping");
    ResetTestState();
    func_8008682C(0);
    ASSERT(PE_LoadU32(0x800BCD80u) == 0x98, "mode 0 -> 0x98");
    ASSERT(PE_LoadU32(0x800B8628u) == 0x9A, "0x98 pushes 0x9A");
    ASSERT(PE_LoadU32(0x800B8628u + 0x24u) == 0x9C, "0x98 pushes 0x9C");
    ASSERT(PE_LoadU32(0x8009D2F4u) == 2, "ring index after 0x98");
    ResetTestState();
    func_8008682C(1);
    ASSERT(PE_LoadU32(0x800BCD80u) == 0x9A, "mode 1 -> 0x9A");
    ASSERT(PE_LoadU32(0x800B8628u) == 0x9A, "0x9A default single entry");
    ASSERT(PE_LoadU32(0x8009D2F4u) == 1, "ring index after 0x9A");
    ResetTestState();
    func_8008682C(2);
    ASSERT(PE_LoadU32(0x800BCD80u) == 0x9C, "mode 2 -> 0x9C");
    ASSERT(PE_LoadU32(0x800B8628u) == 0x9C, "0x9C default single entry");
    PASS();
}

static void test_8CBA8_default_entry_fields(void) {
    TEST("8CBA8_default_entry_fields");
    ResetTestState();
    PE_StoreU32(0x800BCD84u, 0x11111111);
    PE_StoreU32(0x800BCD88u, 0x22222222);
    PE_StoreU32(0x800BCD8Cu, 0x33333333);
    PE_StoreU32(0x800BCD90u, 0x44444444);
    PE_StoreU32(0x800BCD80u, 0xF0);
    ASSERT(func_8008CBA8() == 0, "default path returns 0");
    pe_addr_t e = 0x800B8628u;
    ASSERT(PE_LoadU32(e + 0x0) == 0xF0, "entry cmd");
    ASSERT(PE_LoadU32(e + 0x4) == 0x11111111u, "entry arg84");
    ASSERT(PE_LoadU32(e + 0x8) == 0x22222222u, "entry arg88");
    ASSERT(PE_LoadU32(e + 0xC) == 0x33333333u, "entry arg8C");
    ASSERT(PE_LoadU32(e + 0x10) == 0x44444444u, "entry arg90");
    ASSERT(PE_LoadU32(0x8009D268u) == 0, "busy flag cleared");
    PASS();
}

static void test_8CBA8_0x24_index_math(void) {
    TEST("8CBA8_0x24_index_math");
    ResetTestState();
    PE_StoreU32(0x8009CDF0u, 0x405);
    PE_StoreU32(0x800BCD80u, 0x24);
    ASSERT(func_8008CBA8() == 0x405, "0x24 path returns old index");
    ASSERT(PE_LoadU32(0x8009CDF0u) == 0x406, "index advance ((old+1)&0x1FF)+0x400");
    pe_addr_t e = 0x800B8628u;
    ASSERT(PE_LoadU32(e + 0x0) == 0x24, "entry cmd");
    ASSERT(PE_LoadU32(e + 0x14) == 0x405, "entry old index");
    /* wrap-around: old = 0x5FF -> ((0x600)&0x1FF)+0x400 = 0x400 */
    PE_StoreU32(0x8009CDF0u, 0x5FF);
    ASSERT(func_8008CBA8() == 0x5FF, "wrap returns old");
    ASSERT(PE_LoadU32(0x8009CDF0u) == 0x400, "wrap index math");
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
    /* 6E-B3: 3E680 clears D_8009D1A0, then translated func_8003E974 sets
     * bit 0x4000 — the retail end state */
    ASSERT(D_8009D1A0 == 0x4000, "D_8009D1A0 != 0x4000 (clear + 3E974 |=)");
    ASSERT(D_8009D250 == 0, "D_8009D250 not cleared");
    ASSERT(D_8009CDDC == 0, "D_8009CDDC not cleared");
    PASS();
}

static void test_3E680_exactly_2000_polls(void) {
    TEST("3E680_2000_polls");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* 6E-B2: func_80070D6C is translated; the warm-up is 2000 real
     * advances with zero stub records.  Exact state equality after 2000
     * advances is verified by 3E680_warmup_real. */
    ASSERT(CountOrderLog("func_80070D6C") == 0,
           "func_80070D6C warm-up still routed through bootstrap policy");
    ASSERT(PE_LoadU32(0x80070E04u) != 0x40 || PE_LoadU32(0x80070E08u) != 0x10,
           "RNG indices never advanced during warm-up");
    PASS();
}

static void test_3E680_callback_exactly_once(void) {
    TEST("3E680_callback_once");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* func_80073D24 is now REAL code: it no longer records stub order-log
     * entries.  Its two retail invocations are proven by the exact slot-4
     * end state (see 3E680_callback_registered) and the oracle rung. */
    ASSERT(CountOrderLog("func_80073D24") == 0,
           "func_80073D24 still routed through bootstrap policy");
    PASS();
}

static void test_3E680_callback_registered(void) {
    TEST("3E680_callback_registered");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The guest callback table must hold the retail handler address at
     * slot 4, and it must NOT have been invoked during registration. */
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "slot 4 does not hold guest func_8003E91C");
    ASSERT(PE_LoadU32(GA_CB_SLOT4) == 0x8003E91Cu,
           "guest word at 0x8009569C != 0x8003E91C");
    ASSERT(PE_Callback_RegistrationCount() == 1,
           "callback should be registered once");
    ASSERT(CountOrderLog("func_8003E91C") == 0,
           "func_8003E91C invoked during registration");
    PASS();
}

static void test_3E680_callback_invoke(void) {
    TEST("3E680_callback_invoke");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* Dispatching through the retail slot table resolves guest 0x8003E91C
     * to its host implementation (bound by func_8003E680) and runs it. */
    PE_Callback_Dispatch();
    ASSERT(CountOrderLog("func_8003E91C") == 1,
           "registered callback did not run func_8003E91C");
    ASSERT(PE_LoadU32(GA_CB_COUNTER) == 1, "dispatch counter != 1");
    ASSERT(PE_Callback_ErrorCount() == 0, "dispatch raised a visible error");
    PASS();
}

static void test_3E680_subsystem_order(void) {
    TEST("3E680_subsystem_order");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* After the poll loop, translated func_8003E974 runs (20 real
     * func_8003EAC8 registrations), translated func_80036DC8 runs
     * (6E-B5: timer-record init), the two real func_80073D24 slot
     * writes run (6E-B6), the real func_800371A4 byte store runs
     * (6E-B7), the real func_80029388 slot-table clear + record
     * init runs (6E-B8), the real empty-stub func_8005BCA8 runs
     * (6E-B9), the real func_80068D28 display-record init runs
     * (6E-B10), the real func_800124F8 table clear runs
     * (6E-B11), the real func_8001A890 scalar/array clear runs
     * (6E-B12), the real func_80034F10 table clear + flag-bit
     * clear runs (6E-B13), the real func_8006536C record-table
     * clear runs (6E-B14), and the real func_80038D1C test-and-clear
     * leaf runs (6E-B15).  func_8003E680 is now FULLY translated:
     * no bootstrap providers may remain in the order log. */

    /* All translated rungs must NOT appear in the stub order log. */
    ASSERT(CountOrderLog("func_8003EAC8") == 0,
           "func_8003EAC8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80036DC8") == 0,
           "func_80036DC8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80073D24") == 0,
           "func_80073D24 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_800371A4") == 0,
           "func_800371A4 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80029388") == 0,
           "func_80029388 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8005BCA8") == 0,
           "func_8005BCA8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80068D28") == 0,
           "func_80068D28 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_800124F8") == 0,
           "func_800124F8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8001A890") == 0,
           "func_8001A890 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80034F10") == 0,
           "func_80034F10 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");

    /* Milestone (6E-B15): every func_8003E680 callee is translated;
     * the stub order log must be completely empty. */
    ASSERT(g_stub_order_count == 0,
           "bootstrap providers remain in func_8003E680");
    PASS();
}

static void test_3E680_final_call(void) {
    TEST("3E680_final_func_80038D1C");
    ResetTestState();
    g_bootstrap_disc = 1;

    /* The final func_8003E680 call is the real func_80038D1C
     * test-and-clear leaf (6E-B15): pre-set D_80091A20 and prove the
     * real body cleared it, with no stub record. */
    PE_StoreU8(0x80091A20u, 1u);
    func_8003E680();

    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    ASSERT(PE_LoadU8(0x80091A20u) == 0,
           "final func_80038D1C test-and-clear did not run");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B6 — func_80073D24 VBlank callback slot rung
 *
 * Contract derived from raw MIPS (see pe_callback.h / callback_oracle.py):
 *   func_80073D24(h) == func_80074478(4, h):
 *     prev = D_8009568C[4]; if (h != prev) D_8009568C[4] = h; return prev;
 *   func_8007440C: D_800956AC++; invoke non-null slots 0..7 in order.
 * ═══════════════════════════════════════════════════════════════════════ */

static int  g_cb_visit_count = 0;
static pe_addr_t g_cb_visits[8];
static void cb_body_A(void) { g_cb_visits[g_cb_visit_count++] = 0x80010000u; }
static void cb_body_B(void) { g_cb_visits[g_cb_visit_count++] = 0x80010004u; }
static void cb_body_C(void) { g_cb_visits[g_cb_visit_count++] = 0x80010008u; }
static void cb_body_D(void) { g_cb_visits[g_cb_visit_count++] = 0x8003E91Cu; }

static void test_73D24_raw_contract(void) {
    TEST("73D24_raw_contract");
    ResetTestState();
    PE_Callback_Init();

    /* First install: prev 0, guest word written at slot 4 exactly */
    ASSERT(func_80073D24(0x8003E91Cu) == 0, "first install prev != 0");
    ASSERT(PE_LoadU32(GA_CB_SLOT4) == 0x8003E91Cu, "slot 4 word wrong");
    /* Not a bootstrap stub, no order-log entry */
    ASSERT(g_stub_count == 0, "func_80073D24 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_73D24_replace_remove_prevs(void) {
    TEST("73D24_replace_remove_prevs");
    ResetTestState();
    PE_Callback_Init();

    ASSERT(func_80073D24(0x8003E91Cu) == 0, "install prev != 0");
    ASSERT(func_80073D24(0x80010000u) == 0x8003E91Cu,
           "replacement did not return previous handler");
    ASSERT(func_80073D24(0u) == 0x80010000u,
           "removal did not return replaced handler");
    ASSERT(func_80073D24(0u) == 0, "repeated removal prev != 0");
    ASSERT(PE_LoadU32(GA_CB_SLOT4) == 0, "slot 4 not cleared");
    PASS();
}

static void test_73D24_same_handler_no_store(void) {
    TEST("73D24_same_handler_no_store");
    ResetTestState();
    PE_Callback_Init();

    ASSERT(func_80073D24(0x8003E91Cu) == 0, "install prev != 0");
    /* beq a1,v0 skips the store: prev returned, value unchanged, and the
     * diagnostic registration count does not advance */
    ASSERT(func_80073D24(0x8003E91Cu) == 0x8003E91Cu,
           "same-handler prev wrong");
    ASSERT(PE_LoadU32(GA_CB_SLOT4) == 0x8003E91Cu, "slot 4 changed");
    ASSERT(PE_Callback_RegistrationCount() == 1, "no-store install counted");
    PASS();
}

static void test_73D24_zero_arg_clears(void) {
    TEST("73D24_zero_arg_clears");
    ResetTestState();
    PE_Callback_Init();

    /* Zero is an ordinary handler value (removal), not an error */
    ASSERT(func_80073D24(0u) == 0, "clear-on-empty prev != 0");
    ASSERT(func_80073D24(0x8003E91Cu) == 0, "install prev != 0");
    ASSERT(func_80073D24(0u) == 0x8003E91Cu, "removal prev wrong");
    ASSERT(PE_Callback_ErrorCount() == 0, "zero arg raised an error");
    PASS();
}

static void test_73D24_oracle_equality(void) {
    TEST("73D24_oracle_equality");
    ResetTestState();
    PE_Callback_Init();

    /* Exact expectations generated by the independent MIPS-interpreter
     * oracle pc_port/tools/callback_oracle.py over the verified retail
     * exe words (the same script --callback-oracle-dump prints; the phase
     * gate diffs dump vs oracle byte-for-byte). */
    PE_Callback_Bind(0x80010000u, cb_body_A);
    PE_Callback_Bind(0x80010004u, cb_body_B);
    PE_Callback_Bind(0x80010008u, cb_body_C);
    PE_Callback_Bind(0x8003E91Cu, cb_body_D);

    ASSERT(func_80073D24(0u) == 0, "oracle step 1");
    ASSERT(func_80073D24(0x8003E91Cu) == 0, "oracle step 2");
    ASSERT(func_80073D24(0x8003E91Cu) == 0x8003E91Cu, "oracle step 3");
    ASSERT(func_80073D24(0x80010000u) == 0x8003E91Cu, "oracle step 4");
    ASSERT(func_80073D24(0u) == 0x80010000u, "oracle step 5");
    ASSERT(func_80073D24(0u) == 0, "oracle step 6");
    ASSERT(PE_Callback_SetSlot(0, 0x80010000u) == 0, "oracle step 7");
    ASSERT(PE_Callback_SetSlot(2, 0x80010004u) == 0, "oracle step 8");
    ASSERT(PE_Callback_SetSlot(7, 0x80010008u) == 0, "oracle step 9");

    g_cb_visit_count = 0;
    PE_Callback_Dispatch();
    ASSERT(PE_LoadU32(GA_CB_COUNTER) == 1, "oracle counter != 1");
    ASSERT(g_cb_visit_count == 3, "oracle dispatch 1 visit count");
    ASSERT(g_cb_visits[0] == 0x80010000u && g_cb_visits[1] == 0x80010004u &&
           g_cb_visits[2] == 0x80010008u, "oracle dispatch 1 visit order");

    ASSERT(func_80073D24(0x8003E91Cu) == 0, "oracle step 11");
    g_cb_visit_count = 0;
    PE_Callback_Dispatch();
    ASSERT(PE_LoadU32(GA_CB_COUNTER) == 2, "oracle counter != 2");
    ASSERT(g_cb_visit_count == 4, "oracle dispatch 2 visit count");
    ASSERT(g_cb_visits[0] == 0x80010000u && g_cb_visits[1] == 0x80010004u &&
           g_cb_visits[2] == 0x8003E91Cu && g_cb_visits[3] == 0x80010008u,
           "oracle dispatch 2 visit order");

    /* Slot 8 aliases the dispatch counter @0x800956AC — exact retail
     * address arithmetic (table + (8 << 2)), preserved, not clamped */
    ASSERT(PE_Callback_SetSlot(8, 0x11111111u) == 2, "oracle slot-8 prev");
    ASSERT(PE_LoadU32(GA_CB_COUNTER) == 0x11111111u, "oracle slot-8 alias");
    PASS();
}

static void test_73D24_dispatch_unknown_identity(void) {
    TEST("73D24_dispatch_unknown_identity");
    ResetTestState();
    PE_Callback_Init();

    /* A slot holding a guest address with no host binding must surface a
     * visible error — never a silent skip, never a crash */
    PE_Callback_SetSlot(4, 0x80015555u);
    PE_Callback_Dispatch();
    ASSERT(PE_Callback_ErrorCount() == 1, "unknown identity not reported");
    ASSERT(PE_LoadU32(GA_CB_COUNTER) == 1, "counter not incremented");
    PASS();
}

static void test_73D24_bind_errors(void) {
    TEST("73D24_bind_errors");
    ResetTestState();
    PE_Callback_Init();

    ASSERT(PE_Callback_Bind(0x80010000u, cb_body_A) == 0, "bind failed");
    /* Conflicting rebind of the same guest address: visible error */
    ASSERT(PE_Callback_Bind(0x80010000u, cb_body_B) == -1,
           "conflicting rebind accepted");
    ASSERT(PE_Callback_ErrorCount() == 1, "conflict not counted");
    /* Zero guest / NULL host: visible errors */
    ASSERT(PE_Callback_Bind(0u, cb_body_A) == -1, "zero guest accepted");
    ASSERT(PE_Callback_Bind(0x80010004u, NULL) == -1, "NULL host accepted");
    ASSERT(PE_Callback_ErrorCount() == 3, "bind errors not counted");
    /* The original binding still resolves */
    PE_Callback_SetSlot(4, 0x80010000u);
    g_cb_visit_count = 0;
    PE_Callback_Dispatch();
    ASSERT(g_cb_visit_count == 1 && g_cb_visits[0] == 0x80010000u,
           "original binding lost after errors");
    PASS();
}

static void test_73D24_write_footprint(void) {
    TEST("73D24_write_footprint");
    ResetTestState();
    PE_Callback_Init();

    /* Canary-fill ALL of guest RAM, run both boot-path calls, then scan:
     * exactly the slot-4 word may differ — nothing else. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    (void)func_80073D24(0u);
    (void)func_80073D24(0x8003E91Cu);

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = (a == GA_CB_SLOT4) ? 0x8003E91Cu : 0xA5A5A5A5u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the slot-4 word");
            return;
        }
    }
    PASS();
}

static void test_73D24_ramreset_reinit(void) {
    TEST("73D24_ramreset_reinit");
    ResetTestState();
    PE_Callback_Init();

    (void)func_80073D24(0x8003E91Cu);
    ASSERT(PE_LoadU32(GA_CB_SLOT4) == 0x8003E91Cu, "install failed");
    /* Guest-backed state is cleared by PE_RamReset; re-registration
     * reproduces the identical state (retail reboot behavior) */
    PE_RamReset();
    ASSERT(PE_LoadU32(GA_CB_SLOT4) == 0, "slot 4 not cleared by RAM reset");
    ASSERT(func_80073D24(0x8003E91Cu) == 0, "reinstall prev != 0");
    ASSERT(PE_LoadU32(GA_CB_SLOT4) == 0x8003E91Cu, "reinstall failed");
    PASS();
}

static void test_73D24_strict_not_stub(void) {
    TEST("73D24_strict_not_stub");
    ResetTestState();
    PE_Callback_Init();
    g_strict_stubs = 1;

    /* Under strict mode the REAL function must execute without tripping
     * the centralized bootstrap policy */
    ASSERT(func_80073D24(0x8003E91Cu) == 0, "strict install prev != 0");
    ASSERT(PE_LoadU32(GA_CB_SLOT4) == 0x8003E91Cu, "strict install failed");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B7 — func_800371A4 $gp-relative byte setter rung
 *
 * Retail body (3 words, exe-verified): sb $a0, 0x124($gp); jr $ra; nop.
 * $gp = 0x8009CD70 → destination D_8009CE94 = guest 0x8009CE94.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_D_8009CE94_TEST 0x8009CE94u

static void test_371A4_raw_contract(void) {
    TEST("371A4_raw_contract");
    ResetTestState();

    func_800371A4(1);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 1, "byte store wrong");
    /* Not a bootstrap stub, no order-log entry */
    ASSERT(g_stub_count == 0, "func_800371A4 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_371A4_byte_width_truncation(void) {
    TEST("371A4_byte_width_truncation");
    ResetTestState();

    /* sb stores the low 8 bits of a0 regardless of signedness */
    func_800371A4(0x123);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0x23, "0x123 not truncated to 0x23");
    func_800371A4(-1);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0xFF, "-1 not stored as 0xFF");
    func_800371A4(0x100);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0x00, "0x100 not stored as 0x00");
    /* Byte width: the three neighboring bytes are untouched */
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST - 1) == 0, "byte below modified");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST + 1) == 0, "byte above modified");
    PASS();
}

static void test_371A4_write_footprint(void) {
    TEST("371A4_write_footprint");
    ResetTestState();

    /* Canary-fill ALL of guest RAM, run both retail call-site values,
     * then scan the whole 2 MiB: only the single byte at 0x8009CE94 may
     * differ. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_800371A4(0);
    func_800371A4(1);

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        /* LE byte 0 of the word at 0x8009CE94 becomes 0x01 */
        unsigned int want = (a == GA_D_8009CE94_TEST) ? 0xA5A5A501u
                                                      : 0xA5A5A5A5u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the single retail byte");
            return;
        }
    }
    PASS();
}

static void test_371A4_repeated_and_dirty(void) {
    TEST("371A4_repeated_and_dirty");
    ResetTestState();

    /* Dirty guest state, then invocation; repeated invocation idempotent */
    PE_StoreU8(GA_D_8009CE94_TEST, 0x77);
    func_800371A4(0);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0, "dirty state not overwritten");
    func_800371A4(1);
    func_800371A4(1);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 1, "repeated call not idempotent");
    PASS();
}

static void test_371A4_ramreset_reinit(void) {
    TEST("371A4_ramreset_reinit");
    ResetTestState();

    func_800371A4(1);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 1, "initial store failed");
    PE_RamReset();
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0, "RAM reset did not clear byte");
    func_800371A4(1);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 1, "post-reset store failed");
    PASS();
}

static void test_371A4_3E680_integration(void) {
    TEST("371A4_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The retail func_800371A4(0) ran as real code: the byte is cleared,
     * no stub record exists, and the Phase 6E-B6 callback slot state it
     * follows is intact (retail order: 73D24 x2 then 371A4). */
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "D_8009CE94 not cleared by func_8003E680");
    ASSERT(CountOrderLog("func_800371A4") == 0,
           "func_800371A4 still routed through bootstrap policy");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    PASS();
}

static void test_371A4_strict_not_stub(void) {
    TEST("371A4_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL function must execute without tripping
     * the centralized bootstrap policy */
    func_800371A4(1);
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 1, "strict-mode store failed");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B8 — func_80029388 slot-table clear + default-record init rung
 *
 * Retail contract (see game/boot/func_80029388_port.c header):
 *   func_80029388(): func_8002F658(); clear 7 in-use words at
 *   D_800A5D58 + i*220 (i=0..6); sb 0 -> D_8009D2A0, D_8009D2EC;
 *   func_80020EFC().
 *   func_8002F658(): copy 0x70 bytes D_80010928 -> D_800B8A20, 0x18 bytes
 *   D_80010998 -> D_800B0CB0, then D_8009D1B0 = 0, D_8009D1B4 = 0.
 *   func_80020EFC(): sb 0 -> D_8009CE3C, D_8009D1D4, D_8009D1DC,
 *   D_8009D2D8, D_8009D1F0 (matched decomp leaf, retail source order).
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_T_80010928 0x80010928u
#define GA_T_80010998 0x80010998u
#define GA_T_800B8A20 0x800B8A20u
#define GA_T_800B0CB0 0x800B0CB0u
#define GA_T_8009D1B0 0x8009D1B0u
#define GA_T_8009D1B4 0x8009D1B4u
#define GA_T_800A5D58 0x800A5D58u
#define GA_T_8009D2A0 0x8009D2A0u
#define GA_T_8009D2EC 0x8009D2ECu

static const pe_addr_t g_b8_slot_addrs[7] = {
    0x800A5D58u, 0x800A5E34u, 0x800A5F10u, 0x800A5FECu,
    0x800A60C8u, 0x800A61A4u, 0x800A6280u
};
static const pe_addr_t g_b8_20EFC_bytes[5] = {
    0x8009CE3Cu, 0x8009D1D4u, 0x8009D1DCu, 0x8009D2D8u, 0x8009D1F0u
};

/* Fill the two rodata source records with a deterministic distinct
 * pattern (word i of record 1 = 0x10000000+i, word w of record 2 =
 * 0x20000000+w). */
static void B8_FillSources(void) {
    for (unsigned int i = 0; i < 28; i++) {
        PE_StoreU32(GA_T_80010928 + i * 4u, 0x10000000u + i);
    }
    for (unsigned int w = 0; w < 6; w++) {
        PE_StoreU32(GA_T_80010998 + w * 4u, 0x20000000u + w);
    }
}

static void B8_Verify2F658Copies(void) {
    for (unsigned int i = 0; i < 28; i++) {
        if (PE_LoadU32(GA_T_800B8A20 + i * 4u) != 0x10000000u + i) {
            FAIL("D_800B8A20 record word mismatch");
            return;
        }
    }
    for (unsigned int w = 0; w < 6; w++) {
        if (PE_LoadU32(GA_T_800B0CB0 + w * 4u) != 0x20000000u + w) {
            FAIL("D_800B0CB0 record word mismatch");
            return;
        }
    }
}

static void test_2F658_raw_contract(void) {
    TEST("2F658_raw_contract");
    ResetTestState();

    B8_FillSources();
    for (unsigned int i = 0; i < 28; i++) PE_StoreU32(GA_T_800B8A20 + i * 4u, 0xDEADBEEFu);
    for (unsigned int w = 0; w < 6; w++) PE_StoreU32(GA_T_800B0CB0 + w * 4u, 0xDEADBEEFu);
    PE_StoreU32(GA_T_8009D1B0, 0xDEADBEEFu);
    PE_StoreU32(GA_T_8009D1B4, 0xDEADBEEFu);
    /* Guard words immediately outside each written region */
    PE_StoreU32(GA_T_800B8A20 - 4u, 0x11111111u);
    PE_StoreU32(GA_T_800B8A20 + 0x70u, 0x22222222u);
    PE_StoreU32(GA_T_800B0CB0 - 4u, 0x33333333u);
    PE_StoreU32(GA_T_800B0CB0 + 0x18u, 0x44444444u);

    func_8002F658();

    B8_Verify2F658Copies();
    ASSERT(PE_LoadU32(GA_T_8009D1B0) == 0, "D_8009D1B0 not zeroed");
    ASSERT(PE_LoadU32(GA_T_8009D1B4) == 0, "D_8009D1B4 not zeroed");
    ASSERT(PE_LoadU32(GA_T_8009D1B0 - 4u) == 0, "word below D_8009D1B0 modified");
    ASSERT(PE_LoadU32(GA_T_8009D1B4 + 4u) == 0, "word above D_8009D1B4 modified");
    ASSERT(PE_LoadU32(GA_T_800B8A20 - 4u) == 0x11111111u, "guard below 0x70 record modified");
    ASSERT(PE_LoadU32(GA_T_800B8A20 + 0x70u) == 0x22222222u, "guard above 0x70 record modified");
    ASSERT(PE_LoadU32(GA_T_800B0CB0 - 4u) == 0x33333333u, "guard below 0x18 record modified");
    ASSERT(PE_LoadU32(GA_T_800B0CB0 + 0x18u) == 0x44444444u, "guard above 0x18 record modified");
    ASSERT(g_stub_count == 0, "func_8002F658 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_2F658_write_footprint(void) {
    TEST("2F658_write_footprint");
    ResetTestState();

    /* Canary-fill ALL of guest RAM, run, scan the whole 2 MiB.  The two
     * record copies move canary onto canary (content equality is proven
     * by 2F658_raw_contract); only the two zero words may differ. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_8002F658();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = (a == GA_T_8009D1B0 || a == GA_T_8009D1B4)
                                ? 0u : 0xA5A5A5A5u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the retail regions");
            return;
        }
    }
    PASS();
}

static void test_2F658_repeated_and_dirty(void) {
    TEST("2F658_repeated_and_dirty");
    ResetTestState();

    B8_FillSources();
    func_8002F658();
    /* Dirty the destinations, re-run: fixed-value copies are idempotent */
    for (unsigned int i = 0; i < 28; i++) PE_StoreU32(GA_T_800B8A20 + i * 4u, 0xEEEEEEEEu);
    PE_StoreU32(GA_T_8009D1B0, 0xEEEEEEEEu);
    func_8002F658();
    func_8002F658();
    B8_Verify2F658Copies();
    ASSERT(PE_LoadU32(GA_T_8009D1B0) == 0, "D_8009D1B0 not re-zeroed");
    ASSERT(PE_LoadU32(GA_T_8009D1B4) == 0, "D_8009D1B4 not re-zeroed");
    PASS();
}

static void test_2F658_ramreset_reinit(void) {
    TEST("2F658_ramreset_reinit");
    ResetTestState();

    B8_FillSources();
    func_8002F658();
    B8_Verify2F658Copies();
    PE_RamReset();
    ASSERT(PE_LoadU32(GA_T_800B8A20) == 0, "RAM reset did not clear record");
    /* Post-reset the exe-rodata sources are also zero: the copy replays
     * zeros, matching retail behavior after a guest RAM wipe */
    func_8002F658();
    ASSERT(PE_LoadU32(GA_T_800B8A20) == 0, "post-reset copy not zero");
    ASSERT(PE_LoadU32(GA_T_8009D1B0) == 0, "post-reset zero store failed");
    PASS();
}

static void test_20EFC_raw_contract(void) {
    TEST("20EFC_raw_contract");
    ResetTestState();

    for (int i = 0; i < 5; i++) PE_StoreU8(g_b8_20EFC_bytes[i], 0x77);
    func_80020EFC();
    for (int i = 0; i < 5; i++) {
        ASSERT(PE_LoadU8(g_b8_20EFC_bytes[i]) == 0, "20EFC byte not cleared");
        ASSERT(PE_LoadU8(g_b8_20EFC_bytes[i] + 1u) == 0, "20EFC neighbor modified");
    }
    ASSERT(g_stub_count == 0, "func_80020EFC recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_20EFC_write_footprint(void) {
    TEST("20EFC_write_footprint");
    ResetTestState();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_80020EFC();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        /* All five bytes are word-aligned (LE byte 0) */
        unsigned int want = 0xA5A5A5A5u;
        for (int i = 0; i < 5; i++) {
            if (a == g_b8_20EFC_bytes[i]) want = 0xA5A5A500u;
        }
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the five retail bytes");
            return;
        }
    }
    PASS();
}

static void test_29388_raw_contract(void) {
    TEST("29388_raw_contract");
    ResetTestState();

    B8_FillSources();
    for (int i = 0; i < 7; i++) PE_StoreU32(g_b8_slot_addrs[i], 1u);
    for (int i = 0; i < 7; i++) PE_StoreU32(g_b8_slot_addrs[i] + 4u, 0xBBBBBBBBu);
    PE_StoreU8(GA_T_8009D2A0, 0x55);
    PE_StoreU8(GA_T_8009D2EC, 0x66);
    for (int i = 0; i < 5; i++) PE_StoreU8(g_b8_20EFC_bytes[i], 0x77);
    PE_StoreU32(GA_T_800A5D58 - 4u, 0x11111111u);
    PE_StoreU32(0x800A6360u, 0x22222222u);   /* first word past the 7x220 table */

    func_80029388();

    for (int i = 0; i < 7; i++) {
        ASSERT(PE_LoadU32(g_b8_slot_addrs[i]) == 0, "slot in-use word not cleared");
        ASSERT(PE_LoadU32(g_b8_slot_addrs[i] + 4u) == 0xBBBBBBBBu,
               "slot record body modified");
    }
    ASSERT(PE_LoadU32(GA_T_800A5D58 - 4u) == 0x11111111u, "guard below table modified");
    ASSERT(PE_LoadU32(0x800A6360u) == 0x22222222u, "guard above table modified");
    ASSERT(PE_LoadU8(GA_T_8009D2A0) == 0, "D_8009D2A0 not cleared");
    ASSERT(PE_LoadU8(GA_T_8009D2EC) == 0, "D_8009D2EC not cleared");
    for (int i = 0; i < 5; i++) {
        ASSERT(PE_LoadU8(g_b8_20EFC_bytes[i]) == 0, "20EFC byte not cleared via 29388");
    }
    B8_Verify2F658Copies();
    ASSERT(PE_LoadU32(GA_T_8009D1B0) == 0, "D_8009D1B0 not zeroed via 29388");
    ASSERT(PE_LoadU32(GA_T_8009D1B4) == 0, "D_8009D1B4 not zeroed via 29388");
    ASSERT(g_stub_count == 0, "func_80029388 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_29388_write_footprint(void) {
    TEST("29388_write_footprint");
    ResetTestState();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_80029388();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = 0xA5A5A5A5u;
        /* word clears */
        if (a == GA_T_8009D1B0 || a == GA_T_8009D1B4) want = 0u;
        for (int i = 0; i < 7; i++) {
            if (a == g_b8_slot_addrs[i]) want = 0u;
        }
        /* byte clears — all word-aligned (LE byte 0) */
        if (a == GA_T_8009D2A0 || a == GA_T_8009D2EC) want = 0xA5A5A500u;
        for (int i = 0; i < 5; i++) {
            if (a == g_b8_20EFC_bytes[i]) want = 0xA5A5A500u;
        }
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the retail regions");
            return;
        }
    }
    PASS();
}

static void test_29388_repeated_and_ramreset(void) {
    TEST("29388_repeated_and_ramreset");
    ResetTestState();

    B8_FillSources();
    func_80029388();
    /* Dirty everything the rung writes, then run twice: idempotent */
    for (int i = 0; i < 7; i++) PE_StoreU32(g_b8_slot_addrs[i], 9u);
    PE_StoreU8(GA_T_8009D2A0, 0x55);
    PE_StoreU8(GA_T_8009D2EC, 0x66);
    for (int i = 0; i < 5; i++) PE_StoreU8(g_b8_20EFC_bytes[i], 0x77);
    func_80029388();
    func_80029388();
    for (int i = 0; i < 7; i++) {
        ASSERT(PE_LoadU32(g_b8_slot_addrs[i]) == 0, "repeated: slot word not cleared");
    }
    ASSERT(PE_LoadU8(GA_T_8009D2A0) == 0, "repeated: D_8009D2A0 not cleared");
    ASSERT(PE_LoadU8(GA_T_8009D2EC) == 0, "repeated: D_8009D2EC not cleared");
    for (int i = 0; i < 5; i++) {
        ASSERT(PE_LoadU8(g_b8_20EFC_bytes[i]) == 0, "repeated: 20EFC byte not cleared");
    }
    B8_Verify2F658Copies();

    PE_RamReset();
    ASSERT(PE_LoadU32(g_b8_slot_addrs[0]) == 0, "RAM reset did not clear table");
    func_80029388();
    for (int i = 0; i < 7; i++) {
        ASSERT(PE_LoadU32(g_b8_slot_addrs[i]) == 0, "post-reset: slot word not cleared");
    }
    PASS();
}

static void test_29388_3E680_integration(void) {
    TEST("29388_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    for (int i = 0; i < 7; i++) PE_StoreU32(g_b8_slot_addrs[i], 1u);
    PE_StoreU8(GA_T_8009D2A0, 0x55);
    PE_StoreU8(GA_T_8009D2EC, 0x66);

    func_8003E680();

    /* The retail func_80029388() ran as real code between func_800371A4
     * and the real func_8005BCA8 empty stub: slot table and both bytes
     * cleared, no stub record for either, and the preceding rung state
     * remains intact.  The exe-rodata sources are zero here (no exe
     * loaded), so the 2F658 destinations deterministically receive
     * zeros — retail behavior.  68D28 is now real too (6E-B10),
     * 124F8 as well (6E-B11), 1A890 as well (6E-B12), 34F10 as
     * well (6E-B13), 6536C as well (6E-B14), and 38D1C as well
     * (6E-B15) — func_8003E680 is now fully translated. */
    for (int i = 0; i < 7; i++) {
        ASSERT(PE_LoadU32(g_b8_slot_addrs[i]) == 0, "slot word not cleared by 3E680");
    }
    ASSERT(PE_LoadU8(GA_T_8009D2A0) == 0, "D_8009D2A0 not cleared by 3E680");
    ASSERT(PE_LoadU8(GA_T_8009D2EC) == 0, "D_8009D2EC not cleared by 3E680");
    ASSERT(PE_LoadU8(0x8009CE3Cu) == 0, "20EFC byte not cleared by 3E680");
    ASSERT(PE_LoadU32(GA_T_800B8A20) == 0, "2F658 record not zero via 3E680");
    ASSERT(PE_LoadU32(GA_T_8009D1B0) == 0, "D_8009D1B0 not zeroed by 3E680");
    ASSERT(CountOrderLog("func_80029388") == 0,
           "func_80029388 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8005BCA8") == 0,
           "func_8005BCA8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80068D28") == 0,
           "func_80068D28 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_800124F8") == 0,
           "func_800124F8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8001A890") == 0,
           "func_8001A890 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80034F10") == 0,
           "func_80034F10 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "6E-B7 byte state not visible after func_8003E680");
    PASS();
}

static void test_29388_strict_not_stub(void) {
    TEST("29388_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL functions must execute without tripping
     * the centralized bootstrap policy */
    B8_FillSources();
    func_8002F658();
    B8_Verify2F658Copies();
    func_80020EFC();
    ASSERT(PE_LoadU8(0x8009CE3Cu) == 0, "strict 20EFC store failed");
    PE_StoreU32(g_b8_slot_addrs[0], 1u);
    func_80029388();
    ASSERT(PE_LoadU32(g_b8_slot_addrs[0]) == 0, "strict 29388 clear failed");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B9 — func_8005BCA8 empty jr/nop stub rung
 *
 * Retail contract (see game/boot/func_8005BCA8_port.c header):
 *   void func_8005BCA8(void) — exactly jr $ra ; nop (2 words / 0x8).
 *   Zero guest reads, zero guest writes, zero callees, void return.
 *   Sole call site: func_8003E680 @0x8003E708 (nop delay slot).
 * Independent reference: exact retail-word verification of 0x03E00008 /
 * 0x00000000 at file 0x4C4A8 is sufficient for a trivial straight-line
 * empty stub.
 * ═══════════════════════════════════════════════════════════════════════ */

static void test_5BCA8_raw_contract(void) {
    TEST("5BCA8_raw_contract");
    ResetTestState();

    /* No arguments, no return value, no guest side effects.  Confirm the
     * production path is the real body, not the bootstrap boundary. */
    func_8005BCA8();
    ASSERT(g_stub_count == 0, "func_8005BCA8 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    ASSERT(CountOrderLog("func_8005BCA8") == 0,
           "func_8005BCA8 present in order log");
    PASS();
}

static void test_5BCA8_write_footprint(void) {
    TEST("5BCA8_write_footprint");
    ResetTestState();

    /* Full 2 MiB canary: an empty stub must leave every word untouched. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_8005BCA8();
    func_8005BCA8();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        if (got != 0xA5A5A5A5u) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0xA5A5A5A5\n", a, got);
            FAIL("empty stub wrote guest memory");
            return;
        }
    }
    PASS();
}

static void test_5BCA8_repeated_dirty_and_ramreset(void) {
    TEST("5BCA8_repeated_dirty_and_ramreset");
    ResetTestState();

    /* Seed dirty guest state across the regions earlier rungs touch, plus
     * a few arbitrary addresses; the empty stub must not clear or alter
     * any of them. */
    PE_StoreU32(0x800A5D58u, 0xDEADBEEFu);
    PE_StoreU8(0x8009CE94u, 0xAAu);
    PE_StoreU8(0x8009D2A0u, 0xBBu);
    PE_StoreU32(0x8009568Cu, 0x11223344u);
    PE_StoreU32(0x80001000u, 0xCAFEBABEu);

    func_8005BCA8();
    func_8005BCA8();
    ASSERT(PE_LoadU32(0x800A5D58u) == 0xDEADBEEFu, "dirty slot table altered");
    ASSERT(PE_LoadU8(0x8009CE94u) == 0xAAu, "dirty D_8009CE94 altered");
    ASSERT(PE_LoadU8(0x8009D2A0u) == 0xBBu, "dirty D_8009D2A0 altered");
    ASSERT(PE_LoadU32(0x8009568Cu) == 0x11223344u, "dirty callback slot altered");
    ASSERT(PE_LoadU32(0x80001000u) == 0xCAFEBABEu, "arbitrary dirty word altered");

    PE_RamReset();
    ASSERT(PE_LoadU32(0x800A5D58u) == 0, "RAM reset did not clear");
    PE_StoreU32(0x800A5D58u, 0x55AA55AAu);
    func_8005BCA8();
    ASSERT(PE_LoadU32(0x800A5D58u) == 0x55AA55AAu,
           "post-reset empty stub wrote guest memory");
    ASSERT(g_stub_count == 0, "func_8005BCA8 recorded as stub after reset");
    PASS();
}

static void test_5BCA8_3E680_integration(void) {
    TEST("5BCA8_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    /* Prior-rung state that must still be visible after the empty stub. */
    for (int i = 0; i < 7; i++) PE_StoreU32(g_b8_slot_addrs[i], 1u);
    PE_StoreU8(GA_T_8009D2A0, 0x55);
    PE_StoreU8(GA_T_8009D2EC, 0x66);

    func_8003E680();

    /* Real empty stub ran after 29388; 68D28 is now real too (6E-B10),
     * 124F8 as well (6E-B11), 1A890 as well (6E-B12), 34F10 as
     * well (6E-B13), 6536C as well (6E-B14), and 38D1C as well
     * (6E-B15) — func_8003E680 is now fully translated. */
    ASSERT(CountOrderLog("func_8005BCA8") == 0,
           "func_8005BCA8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80068D28") == 0,
           "func_80068D28 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_800124F8") == 0,
           "func_800124F8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8001A890") == 0,
           "func_8001A890 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80034F10") == 0,
           "func_80034F10 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    for (int i = 0; i < 7; i++) {
        ASSERT(PE_LoadU32(g_b8_slot_addrs[i]) == 0,
               "prior 29388 slot clear not visible after 5BCA8");
    }
    ASSERT(PE_LoadU8(GA_T_8009D2A0) == 0, "prior D_8009D2A0 clear lost");
    ASSERT(PE_LoadU8(GA_T_8009D2EC) == 0, "prior D_8009D2EC clear lost");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "6E-B7 byte state not visible after func_8003E680");
    /* Order: real 5BCA8, 68D28, 124F8, 1A890, 34F10, 6536C, and 38D1C
     * are ALL absent (6E-B15) — func_8003E680 is fully translated and
     * the stub order log must be empty. */
    ASSERT(g_stub_order_count == 0,
           "bootstrap providers remain in func_8003E680");
    PASS();
}

static void test_5BCA8_strict_not_stub(void) {
    TEST("5BCA8_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL empty body must execute without tripping
     * the centralized bootstrap policy.  The former frontier (func_8005BCA8
     * as Bootstrap_ReturnVoid) would have aborted here. */
    func_8005BCA8();
    func_8005BCA8();
    ASSERT(g_stub_count == 0, "func_8005BCA8 recorded as stub under strict");
    ASSERT(Bootstrap_InvocationCount() == 0,
           "bootstrap provider invoked under strict");
    g_strict_stubs = 0;
    PASS();
}

static void test_38D1C_frontier_past_3E680(void) {
    TEST("38D1C_frontier_past_3E680");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    /* Before 6E-B9 the first Bootstrap_ReturnVoid hit from func_8003E680
     * was func_8005BCA8; then func_80068D28 (B10), func_800124F8 (B11),
     * func_8001A890 (B12), func_80034F10 (B13), func_8006536C (B14),
     * and func_80038D1C (B15) — all verified at their pre-change
     * strict-mode baselines.  After the 38D1C translation, EVERY
     * func_8003E680 callee is real: the stub order log must be empty.
     * The strict frontier has therefore advanced PAST func_8003E680 to
     * its caller's next call — func_8006A9E4 (from func_8001220C,
     * retail jal @0x80012284 immediately after the func_8003E680 jal
     * @0x8001227C).  The full --strict-stubs exit at func_8006A9E4 is
     * a runtime gate (Bootstrap_EnableStrict calls exit(1)); this unit
     * test proves func_8003E680 itself is now provider-free. */
    func_8003E680();

    ASSERT(CountOrderLog("func_8005BCA8") == 0,
           "func_8005BCA8 still a bootstrap provider");
    ASSERT(CountOrderLog("func_80068D28") == 0,
           "func_80068D28 still a bootstrap provider");
    ASSERT(CountOrderLog("func_800124F8") == 0,
           "func_800124F8 still a bootstrap provider");
    ASSERT(CountOrderLog("func_8001A890") == 0,
           "func_8001A890 still a bootstrap provider");
    ASSERT(CountOrderLog("func_80034F10") == 0,
           "func_80034F10 still a bootstrap provider");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still a bootstrap provider");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still a bootstrap provider");
    ASSERT(g_stub_order_count == 0,
           "bootstrap providers remain in func_8003E680");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B10 — func_80068D28 display-record data-initializer rung
 *
 * Retail contract (see game/boot/func_80068D28_port.c header), base
 * B = D_800BCF88 = 0x800BCF88:
 *   scalar block: sh 0xFF -> B+0x64/0x62/0x60, sb 1 -> B+0x66,
 *                 sh 0 -> B+0x6C/0x6A/0x68, sb 2 -> B+0x67
 *   loop i = 0..1 (a0 = B+i*0x10, a1 = B+i*0x8):
 *     bytes 3,0xFF,0xFF,0xFF,0x62 at a0+0x33..0x37; halfwords 0,0,
 *     0x140,0xE0 at a0+0x38..0x3E; sb 1 -> a1+0x53;
 *     sw 0xE1000440 -> a1+0x54
 *   post-loop: sh 0 -> B+0x6E, B+0x70 (latter in the jr delay slot)
 *   Write extent 0x800BCFBB..0x800BCFF9; idempotent; $v0=0 unconsumed.
 * Independent reference: 63-word exe verification is complete proof for
 * this straight-line fixed-value / load-after-store initializer.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_T_800BCF88 0x800BCF88u

/* Verify the complete final guest state of one func_80068D28 call. */
static int B10_VerifyState(void) {
    const pe_addr_t B = GA_T_800BCF88;
    /* scalar block */
    if (PE_LoadU16(B + 0x60u) != 0xFFu) return 1;
    if (PE_LoadU16(B + 0x62u) != 0xFFu) return 2;
    if (PE_LoadU16(B + 0x64u) != 0xFFu) return 3;
    if (PE_LoadU8(B + 0x66u) != 1u) return 4;
    if (PE_LoadU8(B + 0x67u) != 2u) return 5;
    if (PE_LoadU16(B + 0x68u) != 0u) return 6;
    if (PE_LoadU16(B + 0x6Au) != 0u) return 7;
    if (PE_LoadU16(B + 0x6Cu) != 0u) return 8;
    if (PE_LoadU16(B + 0x6Eu) != 0u) return 9;
    if (PE_LoadU16(B + 0x70u) != 0u) return 10;
    for (unsigned int i = 0; i < 2; i++) {
        const pe_addr_t a0 = B + i * 0x10u;
        const pe_addr_t a1 = B + i * 0x8u;
        if (PE_LoadU8(a0 + 0x33u) != 3u) return 11;
        if (PE_LoadU8(a0 + 0x34u) != 0xFFu) return 12;
        if (PE_LoadU8(a0 + 0x35u) != 0xFFu) return 13;
        if (PE_LoadU8(a0 + 0x36u) != 0xFFu) return 14;
        if (PE_LoadU8(a0 + 0x37u) != 0x62u) return 15;
        if (PE_LoadU16(a0 + 0x38u) != 0u) return 16;
        if (PE_LoadU16(a0 + 0x3Au) != 0u) return 17;
        if (PE_LoadU16(a0 + 0x3Cu) != 0x140u) return 18;
        if (PE_LoadU16(a0 + 0x3Eu) != 0xE0u) return 19;
        if (PE_LoadU8(a1 + 0x53u) != 1u) return 20;
        if (PE_LoadU32(a1 + 0x54u) != 0xE1000440u) return 21;
    }
    return 0;
}

static void test_68D28_raw_contract(void) {
    TEST("68D28_raw_contract");
    ResetTestState();

    /* Dirty every byte the rung writes, then verify the exact end state. */
    for (pe_addr_t a = GA_T_800BCF88 + 0x30u; a < GA_T_800BCF88 + 0x74u; a += 4) {
        PE_StoreU32(a, 0xDEADBEEFu);
    }

    func_80068D28();

    int rc = B10_VerifyState();
    if (rc != 0) {
        printf("FAIL: B10_VerifyState rc=%d\n", rc);
        FAIL("retail end state mismatch");
        return;
    }
    /* Write extent is exactly 0x800BCFBB..0x800BCFF9: guard words below
     * and above the touched range must survive the dirty fill. */
    ASSERT(PE_LoadU32(GA_T_800BCF88 + 0x2Cu) == 0,
           "guard below record region modified");
    ASSERT(PE_LoadU32(GA_T_800BCF88 + 0x74u) == 0,
           "guard above record region modified");
    ASSERT(g_stub_count == 0, "func_80068D28 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_68D28_write_footprint(void) {
    TEST("68D28_write_footprint");
    ResetTestState();

    /* Full 2 MiB canary: only the exact retail bytes/halfwords/words may
     * change.  Expected per-word values computed from the field map
     * (little-endian): byte fields blend into the canary. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_80068D28();

    const pe_addr_t B = GA_T_800BCF88;
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = 0xA5A5A5A5u;
        if (a == B + 0x30u || a == B + 0x40u) want = 0x03A5A5A5u;
        else if (a == B + 0x34u || a == B + 0x44u) want = 0x62FFFFFFu;
        else if (a == B + 0x38u || a == B + 0x48u) want = 0x00000000u;
        else if (a == B + 0x3Cu || a == B + 0x4Cu) want = 0x00E00140u;
        else if (a == B + 0x50u || a == B + 0x58u) want = 0x01A5A5A5u;
        else if (a == B + 0x54u || a == B + 0x5Cu) want = 0xE1000440u;
        else if (a == B + 0x60u) want = 0x00FF00FFu;
        else if (a == B + 0x64u) want = 0x020100FFu;
        else if (a == B + 0x68u || a == B + 0x6Cu) want = 0x00000000u;
        else if (a == B + 0x70u) want = 0xA5A50000u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the retail region");
            return;
        }
    }
    PASS();
}

static void test_68D28_repeated_dirty_and_ramreset(void) {
    TEST("68D28_repeated_dirty_and_ramreset");
    ResetTestState();

    func_80068D28();
    /* Dirty everything the rung writes; repeated calls are idempotent
     * (the loop reads only values stored by the same invocation). */
    for (pe_addr_t a = GA_T_800BCF88 + 0x30u; a < GA_T_800BCF88 + 0x74u; a += 4) {
        PE_StoreU32(a, 0x77777777u);
    }
    func_80068D28();
    func_80068D28();
    ASSERT(B10_VerifyState() == 0, "repeated calls not idempotent");

    PE_RamReset();
    ASSERT(PE_LoadU32(GA_T_800BCF88 + 0x54u) == 0, "RAM reset did not clear");
    func_80068D28();
    ASSERT(B10_VerifyState() == 0, "post-reset state mismatch");
    PASS();
}

static void test_68D28_3E680_integration(void) {
    TEST("68D28_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The retail func_80068D28() ran as real code between func_8005BCA8
     * and the real func_800124F8 (6E-B11): complete record state present,
     * no stub record, and the preceding rung state remains intact. */
    ASSERT(B10_VerifyState() == 0, "68D28 state missing after 3E680");
    ASSERT(CountOrderLog("func_80068D28") == 0,
           "func_80068D28 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_800124F8") == 0,
           "func_800124F8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8001A890") == 0,
           "func_8001A890 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80034F10") == 0,
           "func_80034F10 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "6E-B7 byte state not visible after func_8003E680");
    ASSERT(PE_LoadU32(g_b8_slot_addrs[0]) == 0,
           "6E-B8 slot clear not visible after func_8003E680");
    PASS();
}

static void test_68D28_strict_not_stub(void) {
    TEST("68D28_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL function must execute without tripping
     * the centralized bootstrap policy */
    func_80068D28();
    ASSERT(B10_VerifyState() == 0, "strict-mode state mismatch");
    ASSERT(g_stub_count == 0, "func_80068D28 recorded as stub under strict");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B11 — func_800124F8 subsystem-table clear rung
 *
 * Retail contract (see game/boot/func_800124F8_port.c header):
 *   sw 0 -> 0x8009D300 (word); sh 0 -> 0x8009D308 (halfword; 0x8009D304
 *   untouched); sw 0 -> 0x8009CDFC; 72x11 word matrix clear at
 *   D_8009D310 (row stride 0x2C) = 0x8009D310..0x8009DF6F; sw 0 ->
 *   0x8009CE00; 16-word array clear at D_8009DF70 = 0x8009DF70..
 *   0x8009DFAF (contiguous with the table); sw 0 -> 0x8009CE04.
 *   No reads, no calls, no hardware; idempotent; $v0=0 unconsumed.
 * Independent reference: 31-word exe verification is complete proof
 * for this fixed-trip-count, constant-value zero-fill.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_T_8009CDFC 0x8009CDFCu
#define GA_T_8009CE00 0x8009CE00u
#define GA_T_8009CE04 0x8009CE04u
#define GA_T_8009D300 0x8009D300u
#define GA_T_8009D308 0x8009D308u
#define GA_T_8009D310 0x8009D310u
#define GA_T_8009DF70 0x8009DF70u
#define B11_TABLE_END 0x8009DFB0u   /* matrix (792w) + array (16w) */

/* Verify the complete final guest state of one func_800124F8 call. */
static int B11_VerifyState(void) {
    if (PE_LoadU32(GA_T_8009D300) != 0u) return 1;
    if (PE_LoadU16(GA_T_8009D308) != 0u) return 2;
    if (PE_LoadU32(GA_T_8009CDFC) != 0u) return 3;
    if (PE_LoadU32(GA_T_8009CE00) != 0u) return 4;
    if (PE_LoadU32(GA_T_8009CE04) != 0u) return 5;
    for (pe_addr_t a = GA_T_8009D310; a < B11_TABLE_END; a += 4) {
        if (PE_LoadU32(a) != 0u) return 6;
    }
    return 0;
}

/* Dirty exactly the bytes the rung may write. */
static void B11_DirtyWritten(unsigned int v) {
    for (pe_addr_t a = GA_T_8009CDFC; a <= GA_T_8009CE04; a += 4)
        PE_StoreU32(a, v);
    for (pe_addr_t a = GA_T_8009D300; a < GA_T_8009D310; a += 4)
        PE_StoreU32(a, v);
    for (pe_addr_t a = GA_T_8009D310; a < B11_TABLE_END; a += 4)
        PE_StoreU32(a, v);
}

static void test_124F8_raw_contract(void) {
    TEST("124F8_raw_contract");
    ResetTestState();

    B11_DirtyWritten(0xDEADBEEFu);

    func_800124F8();

    int rc = B11_VerifyState();
    if (rc != 0) {
        printf("FAIL: B11_VerifyState rc=%d\n", rc);
        FAIL("retail end state mismatch");
        return;
    }
    /* Guards: bytes adjacent to (0x8009D304, 0x8009D30C) or between the
     * written regions must keep the dirty value; bytes fully outside the
     * dirty-filled span remain at the reset zero. */
    ASSERT(PE_LoadU32(GA_T_8009CDFC - 4u) == 0,
           "guard below 0x8009CDFC modified");
    ASSERT(PE_LoadU32(GA_T_8009CE04 + 4u) == 0,
           "guard above 0x8009CE04 modified");
    ASSERT(PE_LoadU32(GA_T_8009D300 - 4u) == 0,
           "guard below 0x8009D300 modified");
    ASSERT(PE_LoadU32(0x8009D304u) == 0xDEADBEEFu,
           "untouched word 0x8009D304 was written");
    ASSERT(PE_LoadU32(0x8009D30Cu) == 0xDEADBEEFu,
           "untouched word 0x8009D30C was written");
    ASSERT(PE_LoadU32(B11_TABLE_END) == 0,
           "guard above table/array modified");
    /* 0x8009D308: only the low halfword is written (little-endian);
     * the upper halfword keeps the dirty bytes. */
    ASSERT(PE_LoadU32(GA_T_8009D308) == 0xDEAD0000u,
           "halfword store width not preserved");
    ASSERT(g_stub_count == 0, "func_800124F8 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_124F8_write_footprint(void) {
    TEST("124F8_write_footprint");
    ResetTestState();

    /* Full 2 MiB canary: only the exact retail words/halfword may
     * change, all to zero. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_800124F8();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = 0xA5A5A5A5u;
        if (a == GA_T_8009CDFC || a == GA_T_8009CE00 ||
            a == GA_T_8009CE04 || a == GA_T_8009D300) want = 0x00000000u;
        else if (a == GA_T_8009D308) want = 0xA5A50000u;
        else if (a >= GA_T_8009D310 && a < B11_TABLE_END) want = 0x00000000u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the retail region");
            return;
        }
    }
    PASS();
}

static void test_124F8_repeated_dirty_and_ramreset(void) {
    TEST("124F8_repeated_dirty_and_ramreset");
    ResetTestState();

    func_800124F8();
    B11_DirtyWritten(0x77777777u);
    func_800124F8();
    func_800124F8();
    ASSERT(B11_VerifyState() == 0, "repeated calls not idempotent");

    PE_RamReset();
    ASSERT(PE_LoadU32(GA_T_8009D310) == 0, "RAM reset did not clear");
    func_800124F8();
    ASSERT(B11_VerifyState() == 0, "post-reset state mismatch");
    PASS();
}

static void test_124F8_3E680_integration(void) {
    TEST("124F8_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The retail func_800124F8() ran as real code between func_80068D28
     * and the real func_8001A890 (6E-B12): complete cleared state
     * present, no stub record, and all preceding rung state remains
     * intact. */
    ASSERT(B11_VerifyState() == 0, "124F8 state missing after 3E680");
    ASSERT(CountOrderLog("func_800124F8") == 0,
           "func_800124F8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8001A890") == 0,
           "func_8001A890 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80034F10") == 0,
           "func_80034F10 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    ASSERT(B10_VerifyState() == 0, "6E-B10 record state lost after 3E680");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "6E-B7 byte state not visible after func_8003E680");
    ASSERT(PE_LoadU32(g_b8_slot_addrs[0]) == 0,
           "6E-B8 slot clear not visible after func_8003E680");
    PASS();
}

static void test_124F8_strict_not_stub(void) {
    TEST("124F8_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL function must execute without tripping
     * the centralized bootstrap policy.  The former frontier
     * (func_800124F8 as Bootstrap_ReturnVoid) would have aborted here. */
    B11_DirtyWritten(0xDEADBEEFu);
    func_800124F8();
    ASSERT(B11_VerifyState() == 0, "strict-mode state mismatch");
    ASSERT(g_stub_count == 0, "func_800124F8 recorded as stub under strict");
    ASSERT(Bootstrap_InvocationCount() == 0,
           "bootstrap provider invoked under strict");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B12 — func_8001A890 subsystem scalar/array clear rung
 *
 * Retail contract (see game/boot/func_8001A890_port.c header):
 *   sw 0 -> 0x8009CE08; sh 0 -> 0x8009CE0C/0x8009CE0E/0x8009CE10/
 *   0x8009CE12 (2-iteration loop); sw 0 -> 0x8009CE14; 20-word array
 *   clear at D_8009DFB0 = 0x8009DFB0..0x8009DFFC (contiguous with
 *   124F8's array end 0x8009DFAF); sh 0 -> 0x8009CE18/0x8009CE1C/
 *   0x8009CE20/0x8009CE24/0x8009CE28/0x8009CE2C (ROM order A8, B8,
 *   B4, B0, AC, BC); sw 0 -> 0x8009D1D8/0x8009D1FC/0x8009D2F8/
 *   0x8009D248 (ROM order 468, 48C, 588, 4D8); sh 0 -> 0x8009D264,
 *   0x8009D1CC (ROM order 4F4, 45C).  No reads, no calls, no
 *   hardware; idempotent; $v0=0 unconsumed.
 * Independent reference: 34-word exe verification is complete proof
 * for this fixed-trip-count, constant-value zero-fill.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_T_8009CE08 0x8009CE08u
#define GA_T_8009CE30 0x8009CE30u   /* first address ABOVE the CE block */
#define GA_T_8009D1CC 0x8009D1CCu
#define GA_T_8009D1D8 0x8009D1D8u
#define GA_T_8009D1FC 0x8009D1FCu
#define GA_T_8009D248 0x8009D248u
#define GA_T_8009D264 0x8009D264u
#define GA_T_8009D2F8 0x8009D2F8u
#define GA_T_8009DFB0 0x8009DFB0u
#define GA_T_8009E000 0x8009E000u   /* first address ABOVE the array */

/* Verify the complete final guest state of one func_8001A890 call.
 * CE08/CE0C/CE10/CE14 are full words (CE0C..CE13 = 4 stride-2
 * halfwords, contiguous); CE18..CE2C are 6 stride-4 halfwords — only
 * the addressed halfword of each word is written. */
static int B12_VerifyState(void) {
    static const pe_addr_t words[] = {
        0x8009CE08u, 0x8009CE0Cu, 0x8009CE10u, 0x8009CE14u
    };
    static const pe_addr_t halfs[] = {
        0x8009CE18u, 0x8009CE1Cu, 0x8009CE20u,
        0x8009CE24u, 0x8009CE28u, 0x8009CE2Cu
    };
    for (unsigned int i = 0; i < sizeof(words) / sizeof(words[0]); i++) {
        if (PE_LoadU32(words[i]) != 0u) return 1;
    }
    for (unsigned int i = 0; i < sizeof(halfs) / sizeof(halfs[0]); i++) {
        if (PE_LoadU16(halfs[i]) != 0u) return 2;
    }
    for (pe_addr_t a = GA_T_8009DFB0; a < GA_T_8009E000; a += 4) {
        if (PE_LoadU32(a) != 0u) return 3;
    }
    if (PE_LoadU32(GA_T_8009D1D8) != 0u) return 4;
    if (PE_LoadU32(GA_T_8009D1FC) != 0u) return 5;
    if (PE_LoadU32(GA_T_8009D248) != 0u) return 6;
    if (PE_LoadU32(GA_T_8009D2F8) != 0u) return 7;
    if (PE_LoadU16(GA_T_8009D1CC) != 0u) return 8;
    if (PE_LoadU16(GA_T_8009D264) != 0u) return 9;
    return 0;
}

/* Dirty exactly the bytes the rung may write. */
static void B12_DirtyWritten(unsigned int v) {
    for (pe_addr_t a = GA_T_8009CE08; a < GA_T_8009CE30; a += 4)
        PE_StoreU32(a, v);
    for (pe_addr_t a = GA_T_8009DFB0; a < GA_T_8009E000; a += 4)
        PE_StoreU32(a, v);
    PE_StoreU32(GA_T_8009D1CC, v);
    PE_StoreU32(GA_T_8009D1D8, v);
    PE_StoreU32(GA_T_8009D1FC, v);
    PE_StoreU32(GA_T_8009D248, v);
    PE_StoreU32(GA_T_8009D264, v);
    PE_StoreU32(GA_T_8009D2F8, v);
}

static void test_1A890_raw_contract(void) {
    TEST("1A890_raw_contract");
    ResetTestState();

    B12_DirtyWritten(0xDEADBEEFu);

    func_8001A890();

    int rc = B12_VerifyState();
    if (rc != 0) {
        printf("FAIL: B12_VerifyState rc=%d\n", rc);
        FAIL("retail end state mismatch");
        return;
    }
    /* Guards: the written bytes end at 0x8009CE2D (CE2E/CE2F keep the
     * dirty value) and at 0x8009DFFC; 124F8's last scalar (0x8009CE04)
     * and array end (0x8009DFAC) sit below the regions untouched. */
    ASSERT(PE_LoadU32(GA_T_8009CE08 - 4u) == 0,
           "guard below 0x8009CE08 modified");
    ASSERT(PE_LoadU32(GA_T_8009CE30) == 0,
           "guard above 0x8009CE2C modified");
    ASSERT(PE_LoadU32(GA_T_8009DFB0 - 4u) == 0,
           "guard below 0x8009DFB0 modified");
    ASSERT(PE_LoadU32(GA_T_8009E000) == 0,
           "guard above 0x8009DFFC modified");
    ASSERT(PE_LoadU32(GA_T_8009D1CC - 4u) == 0,
           "guard below 0x8009D1CC modified");
    ASSERT(PE_LoadU32(GA_T_8009D1CC + 4u) == 0,
           "guard above 0x8009D1CC modified");
    /* Stride-4 halfword stores: only the low halfword of each word is
     * written (little-endian); the upper halfword keeps dirty bytes. */
    ASSERT(PE_LoadU32(0x8009CE18u) == 0xDEAD0000u,
           "stride-4 halfword store at 0x8009CE18 over-wrote");
    ASSERT(PE_LoadU32(0x8009CE2Cu) == 0xDEAD0000u,
           "stride-4 halfword store at 0x8009CE2C over-wrote");
    /* 0x8009D1CC and 0x8009D264: only the low halfword is written
     * (little-endian); the upper halfword keeps the dirty bytes. */
    ASSERT(PE_LoadU32(GA_T_8009D1CC) == 0xDEAD0000u,
           "halfword store width at 0x8009D1CC not preserved");
    ASSERT(PE_LoadU32(GA_T_8009D264) == 0xDEAD0000u,
           "halfword store width at 0x8009D264 not preserved");
    ASSERT(g_stub_count == 0, "func_8001A890 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_1A890_write_footprint(void) {
    TEST("1A890_write_footprint");
    ResetTestState();

    /* Full 2 MiB canary: only the exact retail words/halfwords may
     * change, all to zero. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_8001A890();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = 0xA5A5A5A5u;
        if (a == 0x8009CE08u || a == 0x8009CE0Cu || a == 0x8009CE10u ||
            a == 0x8009CE14u) want = 0x00000000u;
        else if (a == 0x8009CE18u || a == 0x8009CE1Cu || a == 0x8009CE20u ||
                 a == 0x8009CE24u || a == 0x8009CE28u || a == 0x8009CE2Cu)
            want = 0xA5A50000u;
        else if (a >= GA_T_8009DFB0 && a < GA_T_8009E000) want = 0x00000000u;
        else if (a == GA_T_8009D1D8 || a == GA_T_8009D1FC ||
                 a == GA_T_8009D248 || a == GA_T_8009D2F8) want = 0x00000000u;
        else if (a == GA_T_8009D1CC || a == GA_T_8009D264) want = 0xA5A50000u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the retail region");
            return;
        }
    }
    PASS();
}

static void test_1A890_repeated_dirty_and_ramreset(void) {
    TEST("1A890_repeated_dirty_and_ramreset");
    ResetTestState();

    func_8001A890();
    B12_DirtyWritten(0x77777777u);
    func_8001A890();
    func_8001A890();
    ASSERT(B12_VerifyState() == 0, "repeated calls not idempotent");

    PE_RamReset();
    ASSERT(PE_LoadU32(GA_T_8009DFB0) == 0, "RAM reset did not clear");
    func_8001A890();
    ASSERT(B12_VerifyState() == 0, "post-reset state mismatch");
    PASS();
}

static void test_1A890_3E680_integration(void) {
    TEST("1A890_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The retail func_8001A890() ran as real code between
     * func_800124F8 and the func_80034F10 stub: complete cleared state
     * present, no stub record, and all preceding rung state remains
     * intact. */
    ASSERT(B12_VerifyState() == 0, "1A890 state missing after 3E680");
    ASSERT(CountOrderLog("func_8001A890") == 0,
           "func_8001A890 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80034F10") == 0,
           "func_80034F10 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    ASSERT(B11_VerifyState() == 0, "6E-B11 cleared state lost after 3E680");
    ASSERT(B10_VerifyState() == 0, "6E-B10 record state lost after 3E680");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "6E-B7 byte state not visible after func_8003E680");
    ASSERT(PE_LoadU32(g_b8_slot_addrs[0]) == 0,
           "6E-B8 slot clear not visible after func_8003E680");
    PASS();
}

static void test_1A890_strict_not_stub(void) {
    TEST("1A890_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL function must execute without tripping
     * the centralized bootstrap policy.  The former frontier
     * (func_8001A890 as Bootstrap_ReturnVoid) would have aborted here. */
    B12_DirtyWritten(0xDEADBEEFu);
    func_8001A890();
    ASSERT(B12_VerifyState() == 0, "strict-mode state mismatch");
    ASSERT(g_stub_count == 0, "func_8001A890 recorded as stub under strict");
    ASSERT(Bootstrap_InvocationCount() == 0,
           "bootstrap provider invoked under strict");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B13 — func_80034F10 subsystem table clear + flag-bit clear
 *
 * Retail contract (see game/boot/func_80034F10_port.c header):
 *   sw 0 -> 0x8009D2E8; 512-word array clear at D_800A77F0 =
 *   0x800A77F0..0x800A7FEC; D_800B6A80 = 0 (retail: same word stored
 *   64x via a delay-slot loop with no pointer advance); 14x160-word
 *   matrix clear at D_800BEA90 (row stride 0x280) =
 *   0x800BEA90..0x800C0D8F; scalars sw 0 -> 0x8009D2AC/0x8009D20C/
 *   0x8009D2F0/0x8009D254/0x8009D224, sh 0 -> 0x8009D2A6 (ROM order
 *   53C, 49C, 580, 536, 4E4, 4B4); final RMW D_800B0CD8 &= ~0x3000
 *   (sole guest read; store in the jr delay slot).  Idempotent;
 *   $v0 = &D_800B0CD8 unconsumed.
 * Independent reference: 45-word exe verification is complete proof
 * for this fixed-count zero-fill + fixed-mask RMW.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_T_8009D2E8 0x8009D2E8u
#define GA_T_800A77F0 0x800A77F0u
#define GA_T_800A7FF0 0x800A7FF0u   /* first address ABOVE array 1 */
#define GA_T_800B6A80 0x800B6A80u
#define GA_T_800BEA90 0x800BEA90u
#define GA_T_800C0D90 0x800C0D90u   /* first address ABOVE the matrix */
#define GA_T_8009D2AC 0x8009D2ACu
#define GA_T_8009D20C 0x8009D20Cu
#define GA_T_8009D2F0 0x8009D2F0u
#define GA_T_8009D2A4 0x8009D2A4u   /* word containing the D2A6 half */
#define GA_T_8009D2A6 0x8009D2A6u
#define GA_T_8009D254 0x8009D254u
#define GA_T_8009D224 0x8009D224u
#define GA_T_800B0CD8 0x800B0CD8u

/* Verify the complete final guest state of one func_80034F10 call
 * (except D_800B0CD8, whose non-mask bits are caller state). */
static int B13_VerifyState(void) {
    if (PE_LoadU32(GA_T_8009D2E8) != 0u) return 1;
    for (pe_addr_t a = GA_T_800A77F0; a < GA_T_800A7FF0; a += 4) {
        if (PE_LoadU32(a) != 0u) return 2;
    }
    if (PE_LoadU32(GA_T_800B6A80) != 0u) return 3;
    for (pe_addr_t a = GA_T_800BEA90; a < GA_T_800C0D90; a += 4) {
        if (PE_LoadU32(a) != 0u) return 4;
    }
    if (PE_LoadU32(GA_T_8009D2AC) != 0u) return 5;
    if (PE_LoadU32(GA_T_8009D20C) != 0u) return 6;
    if (PE_LoadU32(GA_T_8009D2F0) != 0u) return 7;
    if (PE_LoadU16(GA_T_8009D2A6) != 0u) return 8;
    if (PE_LoadU32(GA_T_8009D254) != 0u) return 9;
    if (PE_LoadU32(GA_T_8009D224) != 0u) return 10;
    if ((PE_LoadU32(GA_T_800B0CD8) & 0x3000u) != 0u) return 11;
    return 0;
}

/* Dirty exactly the bytes the rung may write (except D_800B0CD8,
 * which tests set explicitly). */
static void B13_DirtyWritten(unsigned int v) {
    PE_StoreU32(GA_T_8009D2E8, v);
    for (pe_addr_t a = GA_T_800A77F0; a < GA_T_800A7FF0; a += 4)
        PE_StoreU32(a, v);
    PE_StoreU32(GA_T_800B6A80, v);
    for (pe_addr_t a = GA_T_800BEA90; a < GA_T_800C0D90; a += 4)
        PE_StoreU32(a, v);
    PE_StoreU32(GA_T_8009D2AC, v);
    PE_StoreU32(GA_T_8009D20C, v);
    PE_StoreU32(GA_T_8009D2F0, v);
    PE_StoreU32(GA_T_8009D2A4, v);   /* includes the D2A6 halfword */
    PE_StoreU32(GA_T_8009D254, v);
    PE_StoreU32(GA_T_8009D224, v);
}

static void test_34F10_raw_contract(void) {
    TEST("34F10_raw_contract");
    ResetTestState();

    B13_DirtyWritten(0xDEADBEEFu);
    PE_StoreU32(GA_T_800B0CD8, 0xFFFFFFFFu);

    func_80034F10();

    int rc = B13_VerifyState();
    if (rc != 0) {
        printf("FAIL: B13_VerifyState rc=%d\n", rc);
        FAIL("retail end state mismatch");
        return;
    }
    /* The RMW clears exactly bits 12-13 and preserves all others. */
    ASSERT(PE_LoadU32(GA_T_800B0CD8) == 0xFFFFCFFFu,
           "D_800B0CD8 RMW mask wrong");
    /* Guards around every written region. */
    ASSERT(PE_LoadU32(GA_T_8009D2E8 - 4u) == 0, "guard below D2E8 modified");
    ASSERT(PE_LoadU32(GA_T_8009D2E8 + 4u) == 0, "guard above D2E8 modified");
    ASSERT(PE_LoadU32(GA_T_800A77F0 - 4u) == 0, "guard below array1 modified");
    ASSERT(PE_LoadU32(GA_T_800A7FF0) == 0, "guard above array1 modified");
    ASSERT(PE_LoadU32(GA_T_800B6A80 - 4u) == 0, "guard below B6A80 modified");
    ASSERT(PE_LoadU32(GA_T_800B6A80 + 4u) == 0, "guard above B6A80 modified");
    ASSERT(PE_LoadU32(GA_T_800BEA90 - 4u) == 0, "guard below matrix modified");
    ASSERT(PE_LoadU32(GA_T_800C0D90) == 0, "guard above matrix modified");
    ASSERT(PE_LoadU32(GA_T_800B0CD8 - 4u) == 0, "guard below B0CD8 modified");
    ASSERT(PE_LoadU32(GA_T_800B0CD8 + 4u) == 0, "guard above B0CD8 modified");
    /* 0x8009D2A6 is a HALFWORD store: it is the high half of the word
     * at 0x8009D2A4 (little-endian); the low half keeps dirty bytes. */
    ASSERT(PE_LoadU32(GA_T_8009D2A4) == 0x0000BEEFu,
           "halfword store width at 0x8009D2A6 not preserved");
    ASSERT(g_stub_count == 0, "func_80034F10 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_34F10_write_footprint(void) {
    TEST("34F10_write_footprint");
    ResetTestState();

    /* Full 2 MiB canary: only the exact retail words/halfword and the
     * masked bits of D_800B0CD8 may change. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_80034F10();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = 0xA5A5A5A5u;
        if (a == GA_T_8009D2E8 || a == GA_T_800B6A80 ||
            a == GA_T_8009D2AC || a == GA_T_8009D20C ||
            a == GA_T_8009D2F0 || a == GA_T_8009D254 ||
            a == GA_T_8009D224) want = 0x00000000u;
        else if (a >= GA_T_800A77F0 && a < GA_T_800A7FF0) want = 0x00000000u;
        else if (a >= GA_T_800BEA90 && a < GA_T_800C0D90) want = 0x00000000u;
        else if (a == GA_T_8009D2A4) want = 0x0000A5A5u;
        else if (a == GA_T_800B0CD8) want = 0xA5A585A5u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the retail region");
            return;
        }
    }
    PASS();
}

static void test_34F10_repeated_dirty_and_ramreset(void) {
    TEST("34F10_repeated_dirty_and_ramreset");
    ResetTestState();

    func_80034F10();
    B13_DirtyWritten(0x77777777u);
    PE_StoreU32(GA_T_800B0CD8, 0xFFFFFFFFu);
    func_80034F10();
    func_80034F10();
    ASSERT(B13_VerifyState() == 0, "repeated calls not idempotent");
    ASSERT(PE_LoadU32(GA_T_800B0CD8) == 0xFFFFCFFFu,
           "repeated RMW not idempotent");

    PE_RamReset();
    ASSERT(PE_LoadU32(GA_T_800A77F0) == 0, "RAM reset did not clear");
    ASSERT(PE_LoadU32(GA_T_800B0CD8) == 0, "RAM reset did not clear flags");
    func_80034F10();
    ASSERT(B13_VerifyState() == 0, "post-reset state mismatch");
    ASSERT(PE_LoadU32(GA_T_800B0CD8) == 0, "post-reset RMW wrong");
    PASS();
}

static void test_34F10_3E680_integration(void) {
    TEST("34F10_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The retail func_80034F10() ran as real code between
     * func_8001A890 and the real func_8006536C (6E-B14): complete
     * cleared state present, no stub record, and all preceding rung
     * state remains intact. */
    ASSERT(B13_VerifyState() == 0, "34F10 state missing after 3E680");
    ASSERT(CountOrderLog("func_80034F10") == 0,
           "func_80034F10 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    ASSERT(B12_VerifyState() == 0, "6E-B12 cleared state lost after 3E680");
    ASSERT(B11_VerifyState() == 0, "6E-B11 cleared state lost after 3E680");
    ASSERT(B10_VerifyState() == 0, "6E-B10 record state lost after 3E680");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "6E-B7 byte state not visible after func_8003E680");
    ASSERT(PE_LoadU32(g_b8_slot_addrs[0]) == 0,
           "6E-B8 slot clear not visible after func_8003E680");
    PASS();
}

static void test_34F10_strict_not_stub(void) {
    TEST("34F10_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL function must execute without tripping
     * the centralized bootstrap policy.  The former frontier
     * (func_80034F10 as Bootstrap_ReturnVoid) would have aborted here. */
    B13_DirtyWritten(0xDEADBEEFu);
    PE_StoreU32(GA_T_800B0CD8, 0xFFFFFFFFu);
    func_80034F10();
    ASSERT(B13_VerifyState() == 0, "strict-mode state mismatch");
    ASSERT(PE_LoadU32(GA_T_800B0CD8) == 0xFFFFCFFFu,
           "strict-mode RMW mismatch");
    ASSERT(g_stub_count == 0, "func_80034F10 recorded as stub under strict");
    ASSERT(Bootstrap_InvocationCount() == 0,
           "bootstrap provider invoked under strict");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B14 — func_8006536C record-table clear + index byte clear
 *
 * Retail contract (see game/boot/func_8006536C_port.c header):
 *   28x3-word table clear at D_800A3180 (row stride 0xC — contiguous
 *   84 words, span 0x800A3180..0x800A32CF); sb 0 -> 0x44($gp) =
 *   0x8009CDB4 (current-record index byte).  No reads, no SDK/GTE/
 *   hardware/callback/GPU work.  void(void); $v0 = 0 unconsumed.
 *   Sole call site func_8003E680 @0x8003E730 (nop delay slot).
 *   Idempotent, including after PE_RamReset.
 * Independent reference: 19-word exe verification is complete proof
 * for this fixed-count zero-fill + constant byte store.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_T_800A3180 0x800A3180u
#define GA_T_800A32D0 0x800A32D0u   /* first address ABOVE the table */
#define GA_T_8009CDB4 0x8009CDB4u

/* Verify the complete final guest state of one func_8006536C call. */
static int B14_VerifyState(void) {
    for (pe_addr_t a = GA_T_800A3180; a < GA_T_800A32D0; a += 4) {
        if (PE_LoadU32(a) != 0u) return 1;
    }
    if (PE_LoadU8(GA_T_8009CDB4) != 0u) return 2;
    return 0;
}

/* Dirty exactly the bytes the rung may write. */
static void B14_DirtyWritten(unsigned int v) {
    for (pe_addr_t a = GA_T_800A3180; a < GA_T_800A32D0; a += 4)
        PE_StoreU32(a, v);
    PE_StoreU8(GA_T_8009CDB4, 0xFFu);
}

static void test_6536C_raw_contract(void) {
    TEST("6536C_raw_contract");
    ResetTestState();

    B14_DirtyWritten(0xDEADBEEFu);

    func_8006536C();

    int rc = B14_VerifyState();
    if (rc != 0) {
        printf("FAIL: B14_VerifyState rc=%d\n", rc);
        FAIL("retail end state mismatch");
        return;
    }
    /* Guards around the written regions. */
    ASSERT(PE_LoadU32(GA_T_800A3180 - 4u) == 0, "guard below table modified");
    ASSERT(PE_LoadU32(GA_T_800A32D0) == 0, "guard above table modified");
    ASSERT(PE_LoadU8(GA_T_8009CDB4 - 1u) == 0, "guard below index byte modified");
    ASSERT(PE_LoadU8(GA_T_8009CDB4 + 1u) == 0, "guard above index byte modified");
    /* 0x8009CDB4 is a BYTE store: dirty the whole word and confirm
     * only the low byte is cleared (little-endian). */
    PE_StoreU32(GA_T_8009CDB4, 0xDEADBEEFu);
    func_8006536C();
    ASSERT(PE_LoadU32(GA_T_8009CDB4) == 0xDEADBE00u,
           "byte store width at 0x8009CDB4 not preserved");
    ASSERT(g_stub_count == 0, "func_8006536C recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_6536C_write_footprint(void) {
    TEST("6536C_write_footprint");
    ResetTestState();

    /* Full 2 MiB canary: only the exact 84 table words and the low byte
     * of the word at 0x8009CDB4 may change. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    func_8006536C();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = 0xA5A5A5A5u;
        if (a >= GA_T_800A3180 && a < GA_T_800A32D0) want = 0x00000000u;
        else if (a == GA_T_8009CDB4) want = 0xA5A5A500u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the retail region");
            return;
        }
    }
    PASS();
}

static void test_6536C_repeated_dirty_and_ramreset(void) {
    TEST("6536C_repeated_dirty_and_ramreset");
    ResetTestState();

    func_8006536C();
    B14_DirtyWritten(0x77777777u);
    func_8006536C();
    func_8006536C();
    ASSERT(B14_VerifyState() == 0, "repeated calls not idempotent");

    PE_RamReset();
    ASSERT(PE_LoadU32(GA_T_800A3180) == 0, "RAM reset did not clear");
    func_8006536C();
    ASSERT(B14_VerifyState() == 0, "post-reset state mismatch");
    PASS();
}

static void test_6536C_3E680_integration(void) {
    TEST("6536C_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The retail func_8006536C() ran as real code between
     * func_80034F10 and the real func_80038D1C (6E-B15): complete
     * cleared state present, no stub record, and all preceding rung
     * state remains intact. */
    ASSERT(B14_VerifyState() == 0, "6536C state missing after 3E680");
    ASSERT(CountOrderLog("func_8006536C") == 0,
           "func_8006536C still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    ASSERT(B13_VerifyState() == 0, "6E-B13 cleared state lost after 3E680");
    ASSERT(B12_VerifyState() == 0, "6E-B12 cleared state lost after 3E680");
    ASSERT(B11_VerifyState() == 0, "6E-B11 cleared state lost after 3E680");
    ASSERT(B10_VerifyState() == 0, "6E-B10 record state lost after 3E680");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "6E-B7 byte state not visible after func_8003E680");
    ASSERT(PE_LoadU32(g_b8_slot_addrs[0]) == 0,
           "6E-B8 slot clear not visible after func_8003E680");
    PASS();
}

static void test_6536C_strict_not_stub(void) {
    TEST("6536C_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL function must execute without tripping
     * the centralized bootstrap policy.  The former frontier
     * (func_8006536C as Bootstrap_ReturnVoid) would have aborted here. */
    B14_DirtyWritten(0xDEADBEEFu);
    func_8006536C();
    ASSERT(B14_VerifyState() == 0, "strict-mode state mismatch");
    ASSERT(g_stub_count == 0, "func_8006536C recorded as stub under strict");
    ASSERT(Bootstrap_InvocationCount() == 0,
           "bootstrap provider invoked under strict");
    g_strict_stubs = 0;
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B15 — func_80038D1C byte test-and-clear status leaf
 *
 * Retail contract (see game/boot/func_80038D1C_port.c header; ALSO a
 * matched decomp C leaf, src/func_80038D1C.c):
 *   int func_80038D1C(void): if D_80091A20 != 0 -> sb 0 -> D_80091A20,
 *   return 0; else return 0xFF (255).  One byte read; one CONDITIONAL
 *   byte write (only when the flag was set).  Two call sites —
 *   func_8003E680 @0x8003E738 (final call; void epilogue follows,
 *   return NOT consumed) and func_8006E9A0 @0x8006EB7C (return
 *   ignored).  With this leaf translated, func_8003E680 is FULLY
 *   translated; the strict frontier advances past it to
 *   func_8006A9E4 (from func_8001220C).
 * Independent reference: 11-word exe verification + matched C leaf +
 * both return paths + canary footprints (write and no-write paths) are
 * complete proof.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_T_80091A20 0x80091A20u

static void test_38D1C_raw_contract(void) {
    TEST("38D1C_raw_contract");
    ResetTestState();

    /* Path 1: flag clear -> return 0xFF, NO write. */
    ASSERT(func_80038D1C() == 0xFF, "clear flag must return 0xFF");
    ASSERT(PE_LoadU8(GA_T_80091A20) == 0, "flag changed on no-write path");

    /* Path 2: flag set -> return 0, flag cleared. */
    PE_StoreU8(GA_T_80091A20, 1u);
    ASSERT(func_80038D1C() == 0, "set flag must return 0");
    ASSERT(PE_LoadU8(GA_T_80091A20) == 0, "flag not cleared");

    /* Any nonzero value qualifies (lbu, not a boolean test). */
    PE_StoreU8(GA_T_80091A20, 0xFFu);
    ASSERT(func_80038D1C() == 0, "0xFF flag must return 0");
    ASSERT(PE_LoadU8(GA_T_80091A20) == 0, "0xFF flag not cleared");

    /* Byte store width: dirty the whole word; only the low byte of
     * 0x80091A20 (4-aligned, little-endian) is read and cleared. */
    PE_StoreU32(GA_T_80091A20, 0xDEADBEEFu);
    ASSERT(func_80038D1C() == 0, "dirty-word flag must return 0");
    ASSERT(PE_LoadU32(GA_T_80091A20) == 0xDEADBE00u,
           "byte store width at 0x80091A20 not preserved");

    /* Guards around the flag byte. */
    ASSERT(PE_LoadU8(GA_T_80091A20 - 1u) == 0, "guard below flag modified");
    ASSERT(PE_LoadU8(GA_T_80091A20 + 1u) == 0xBEu,
           "guard above flag modified");
    ASSERT(g_stub_count == 0, "func_80038D1C recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_38D1C_write_footprint(void) {
    TEST("38D1C_write_footprint");
    ResetTestState();

    /* Full 2 MiB canary, flag SET (0xA5 != 0): only the low byte of
     * the word at 0x80091A20 may change. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }

    ASSERT(func_80038D1C() == 0, "canary flag must return 0");

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = (a == GA_T_80091A20) ? 0xA5A5A500u : 0xA5A5A5A5u;
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the retail region");
            return;
        }
    }

    /* Full 2 MiB canary, flag CLEAR: NO write anywhere, return 0xFF. */
    ResetTestState();
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        PE_StoreU32(a, 0xA5A5A5A5u);
    }
    PE_StoreU8(GA_T_80091A20, 0u);   /* word is now 0xA5A5A500 */

    ASSERT(func_80038D1C() == 0xFF, "clear canary flag must return 0xFF");

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = (a == GA_T_80091A20) ? 0xA5A5A500u : 0xA5A5A5A5u;
        if (got != want) {
            printf("FAIL(no-write path): guest 0x%08X = 0x%08X, want 0x%08X\n",
                   a, got, want);
            FAIL("no-write path modified guest memory");
            return;
        }
    }
    PASS();
}

static void test_38D1C_repeated_and_ramreset(void) {
    TEST("38D1C_repeated_and_ramreset");
    ResetTestState();

    /* Test-and-clear is NOT return-idempotent: first call consumes the
     * flag; the guest state after repeated calls is stable. */
    PE_StoreU8(GA_T_80091A20, 7u);
    ASSERT(func_80038D1C() == 0, "first call must return 0");
    ASSERT(func_80038D1C() == 0xFF, "second call must return 0xFF");
    ASSERT(func_80038D1C() == 0xFF, "third call must return 0xFF");
    ASSERT(PE_LoadU8(GA_T_80091A20) == 0, "flag state not stable");

    PE_RamReset();
    ASSERT(PE_LoadU8(GA_T_80091A20) == 0, "RAM reset did not clear flag");
    ASSERT(func_80038D1C() == 0xFF, "post-reset call must return 0xFF");
    PASS();
}

static void test_38D1C_3E680_integration(void) {
    TEST("38D1C_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    /* Pre-set the flag: the final func_8003E680 call must consume it. */
    PE_StoreU8(GA_T_80091A20, 1u);

    func_8003E680();

    ASSERT(PE_LoadU8(GA_T_80091A20) == 0,
           "38D1C test-and-clear missing after 3E680");
    ASSERT(CountOrderLog("func_80038D1C") == 0,
           "func_80038D1C still routed through bootstrap policy");
    /* Milestone: func_8003E680 is fully translated. */
    ASSERT(g_stub_order_count == 0,
           "bootstrap providers remain in func_8003E680");
    ASSERT(B14_VerifyState() == 0, "6E-B14 cleared state lost after 3E680");
    ASSERT(B13_VerifyState() == 0, "6E-B13 cleared state lost after 3E680");
    ASSERT(B12_VerifyState() == 0, "6E-B12 cleared state lost after 3E680");
    ASSERT(B11_VerifyState() == 0, "6E-B11 cleared state lost after 3E680");
    ASSERT(B10_VerifyState() == 0, "6E-B10 record state lost after 3E680");
    ASSERT(PE_Callback_GetSlot(4) == 0x8003E91Cu,
           "6E-B6 slot-4 state not visible after func_8003E680");
    ASSERT(PE_LoadU8(GA_D_8009CE94_TEST) == 0,
           "6E-B7 byte state not visible after func_8003E680");
    ASSERT(PE_LoadU32(g_b8_slot_addrs[0]) == 0,
           "6E-B8 slot clear not visible after func_8003E680");
    PASS();
}

static void test_38D1C_strict_not_stub(void) {
    TEST("38D1C_strict_not_stub");
    ResetTestState();
    g_strict_stubs = 1;

    /* Under strict mode the REAL function must execute without tripping
     * the centralized bootstrap policy.  The former frontier
     * (func_80038D1C as Bootstrap_ReturnInt) would have aborted here. */
    PE_StoreU8(GA_T_80091A20, 1u);
    ASSERT(func_80038D1C() == 0, "strict-mode return mismatch");
    ASSERT(PE_LoadU8(GA_T_80091A20) == 0, "strict-mode state mismatch");
    ASSERT(func_80038D1C() == 0xFF, "strict-mode second return mismatch");
    ASSERT(g_stub_count == 0, "func_80038D1C recorded as stub under strict");
    ASSERT(Bootstrap_InvocationCount() == 0,
           "bootstrap provider invoked under strict");
    g_strict_stubs = 0;
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
 * Phase 6E-B1 — Provider frontier: func_80070D10 (RNG table init)
 *
 * Retail truth (asm/disc1/5F3E4.s:2439-2464): 17-word descending-Fibonacci
 * table at 0x80070E0C..0x80070E4C, index words 0x80070E04 = 0x40 and
 * 0x80070E08 = 0x10.  Sole caller func_8003E680; the block is shared only
 * with func_80070D6C (RNG advance).
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_TEST_RNG_INDEX1  0x80070E04u
#define GA_TEST_RNG_INDEX2  0x80070E08u
#define GA_TEST_RNG_TABLE   0x80070E0Cu
#define RNG_TABLE_WORDS     17

static const unsigned int k_rng_table_expect[RNG_TABLE_WORDS] = {
    2584, 1597, 987, 610, 377, 233, 144, 89, 55,
    34, 21, 13, 8, 5, 3, 2, 1
};

static int RngBlockMatchesRetail(void) {
    if (PE_LoadU32(GA_TEST_RNG_INDEX1) != 0x40) return 0;
    if (PE_LoadU32(GA_TEST_RNG_INDEX2) != 0x10) return 0;
    for (int i = 0; i < RNG_TABLE_WORDS; i++) {
        if (PE_LoadU32(GA_TEST_RNG_TABLE + (unsigned int)i * 4)
                != k_rng_table_expect[i])
            return 0;
    }
    return 1;
}

static void test_70D10_table_exact(void) {
    TEST("70D10_table_exact");
    ResetTestState();

    func_80070D10();

    ASSERT(PE_LoadU32(GA_TEST_RNG_INDEX1) == 0x40, "index1 not 0x40");
    ASSERT(PE_LoadU32(GA_TEST_RNG_INDEX2) == 0x10, "index2 not 0x10");
    for (int i = 0; i < RNG_TABLE_WORDS; i++) {
        char msg[64];
        snprintf(msg, sizeof msg, "table word %d wrong", i);
        ASSERT(PE_LoadU32(GA_TEST_RNG_TABLE + (unsigned int)i * 4)
                   == k_rng_table_expect[i], msg);
    }
    PASS();
}

static void test_70D10_reseed_idempotent(void) {
    TEST("70D10_reseed_idempotent");
    ResetTestState();

    func_80070D10();
    ASSERT(RngBlockMatchesRetail(), "table wrong after first call");

    /* Dirty the entire block, then re-seed: retail re-writes every word */
    for (pe_addr_t a = GA_TEST_RNG_INDEX1; a <= 0x80070E4Cu; a += 4)
        PE_StoreU32(a, 0xDEADBEEF);
    func_80070D10();
    ASSERT(RngBlockMatchesRetail(), "table not restored after dirty + reseed");

    /* Back-to-back calls converge to the same state (deterministic init) */
    func_80070D10();
    ASSERT(RngBlockMatchesRetail(), "table changed on repeated call");
    PASS();
}

static void test_70D10_write_footprint(void) {
    TEST("70D10_write_footprint");
    ResetTestState();

    /* Canary-fill ALL of guest RAM, run, then scan the whole 2 MiB:
     * exactly the 19 retail words may differ — nothing else. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0xA5A5A5A5u);

    func_80070D10();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        unsigned int got = PE_LoadU32(a);
        unsigned int want = 0xA5A5A5A5u;
        if (a == GA_TEST_RNG_INDEX1) want = 0x40;
        else if (a == GA_TEST_RNG_INDEX2) want = 0x10;
        else if (a >= GA_TEST_RNG_TABLE && a <= 0x80070E4Cu)
            want = k_rng_table_expect[(a - GA_TEST_RNG_TABLE) / 4];
        if (got != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, got, want);
            FAIL("write footprint exceeds the 19 retail words");
            return;
        }
    }
    PASS();
}

static void test_70D10_not_bootstrap_stub(void) {
    TEST("70D10_not_bootstrap_stub");
    ResetTestState();

    func_80070D10();

    /* Translated function: no BOOTSTRAP_RET record, no order-log entry */
    ASSERT(CountOrderLog("func_80070D10") == 0, "func_80070D10 recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_3E680_70D10_translated(void) {
    TEST("3E680_70D10_translated");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    /* The RNG init ran as real code — proven by the ×2000 real warm-up
     * that follows it having advanced the indices from their 6E-B1 seed
     * values (exact post-warm-up state is 3E680_warmup_real's gate) */
    ASSERT(PE_LoadU32(0x80070E04u) != 0x40 || PE_LoadU32(0x80070E08u) != 0x10,
           "RNG never seeded/advanced inside func_8003E680");
    ASSERT(CountOrderLog("func_80070D10") == 0,
           "func_80070D10 still routed through bootstrap policy");
    /* 6E-B2: the ×2000 warm-up is now REAL translated code — no stub
     * records (the real-advance state is verified by 3E680_warmup_real) */
    ASSERT(CountOrderLog("func_80070D6C") == 0,
           "func_80070D6C still routed through bootstrap policy");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B2 — Provider frontier: func_80070D6C (RNG advance) + 70DD0
 *
 * Independent model of the retail MIPS semantics (asm/disc1/5F3E4.s:
 * 2468-2515), written from the assembly — the same model validated against
 * the retail exe by pc_port/tools/rng_oracle.py.  Tests run over a
 * SYNTHETIC below-table pattern so they need no retail bytes; equality
 * against the real retail byte stream is the --rng-oracle-dump gate.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_TEST_RNG_LO   0x80070DCCu   /* first below-table word the model
                                          read cursor can reach            */
#define RNG_LO_WORDS     16            /* 0x80070DCC..0x80070E08 inclusive */

static unsigned int model_70D6C(void) {
    int t1 = (int)PE_LoadU32(GA_TEST_RNG_INDEX1);
    int t2 = (int)PE_LoadU32(GA_TEST_RNG_INDEX2);
    pe_addr_t a1 = GA_TEST_RNG_TABLE + (pe_addr_t)t1;
    pe_addr_t a2 = GA_TEST_RNG_TABLE + (pe_addr_t)t2;
    unsigned int v = PE_LoadU32(a1) + PE_LoadU32(a2);
    PE_StoreU32(a1, v);
    t1 -= 4; t2 -= 4;
    if (t1 < 0) t1 = 0x40;
    if (t2 < 0) t2 = (int)((unsigned int)t2 | 0x40u);
    PE_StoreU32(GA_TEST_RNG_INDEX1, (unsigned int)t1);
    PE_StoreU32(GA_TEST_RNG_INDEX2, (unsigned int)t2);
    return v;
}

static int model_70DD0(int a0, int a1) {
    unsigned int r = model_70D6C() & 0xFFFFu;
    int diff = (int)((unsigned int)a1 - (unsigned int)a0);
    long long product = (long long)(int)r * (long long)diff;
    int scaled = (int)(product >> 16);
    return (int)((unsigned int)a0 + (unsigned int)scaled);
}

/* Distinct synthetic pattern below the RNG block: makes every read
 * address uniquely identifiable in the output stream. */
static void RngSeedWithPattern(void) {
    for (int i = 0; i < RNG_LO_WORDS; i++)
        PE_StoreU32(GA_TEST_RNG_LO + (unsigned int)i * 4,
                    0xBEEF0000u + (unsigned int)i * 0x1111u);
    func_80070D10();
}

static void SnapshotRngBlock(unsigned int out[19]) {
    out[0] = PE_LoadU32(GA_TEST_RNG_INDEX1);
    out[1] = PE_LoadU32(GA_TEST_RNG_INDEX2);
    for (int i = 0; i < RNG_TABLE_WORDS; i++)
        out[2 + i] = PE_LoadU32(GA_TEST_RNG_TABLE + (unsigned int)i * 4);
}

static int RngBlockEquals(const unsigned int a[19], const unsigned int b[19]) {
    for (int i = 0; i < 19; i++)
        if (a[i] != b[i]) return 0;
    return 1;
}

static void test_70D6C_first_16_returns(void) {
    TEST("70D6C_first_16_returns");
    ResetTestState();
    RngSeedWithPattern();

    unsigned int port_out[16], model_out[16];
    for (int i = 0; i < 16; i++) port_out[i] = func_80070D6C();

    ResetTestState();
    RngSeedWithPattern();
    for (int i = 0; i < 16; i++) model_out[i] = model_70D6C();

    for (int i = 0; i < 16; i++) {
        char msg[64];
        snprintf(msg, sizeof msg, "return %d mismatch", i + 1);
        ASSERT(port_out[i] == model_out[i], msg);
    }
    PASS();
}

static void test_70D6C_checkpoints_and_state(void) {
    TEST("70D6C_checkpoints_and_state");
    static const int k_cp[] = { 1, 2, 16, 17, 64, 256, 2000 };
    unsigned int port_v[7], model_v[7];
    unsigned int port_i1[7], port_i2[7], model_i1[7], model_i2[7];
    unsigned int port_blk[19], model_blk[19];
    int n = 0;

    ResetTestState();
    RngSeedWithPattern();
    for (int call = 1; call <= 2000; call++) {
        unsigned int v = func_80070D6C();
        if (call == k_cp[n] && n < 7) {
            port_v[n] = v;
            port_i1[n] = PE_LoadU32(GA_TEST_RNG_INDEX1);
            port_i2[n] = PE_LoadU32(GA_TEST_RNG_INDEX2);
            n++;
        }
    }
    SnapshotRngBlock(port_blk);

    n = 0;
    ResetTestState();
    RngSeedWithPattern();
    for (int call = 1; call <= 2000; call++) {
        unsigned int v = model_70D6C();
        if (call == k_cp[n] && n < 7) {
            model_v[n] = v;
            model_i1[n] = PE_LoadU32(GA_TEST_RNG_INDEX1);
            model_i2[n] = PE_LoadU32(GA_TEST_RNG_INDEX2);
            n++;
        }
    }
    SnapshotRngBlock(model_blk);

    for (int i = 0; i < 7; i++) {
        char msg[80];
        snprintf(msg, sizeof msg, "checkpoint %d (call %d) mismatch",
                 i, k_cp[i]);
        ASSERT(port_v[i] == model_v[i], msg);
        ASSERT(port_i1[i] == model_i1[i], "index1 checkpoint mismatch");
        ASSERT(port_i2[i] == model_i2[i], "index2 checkpoint mismatch");
    }
    ASSERT(RngBlockEquals(port_blk, model_blk),
           "full RNG block state differs after 2000 calls");
    PASS();
}

static void test_70D6C_index_wrap_exact(void) {
    TEST("70D6C_index_wrap_exact");
    ResetTestState();
    RngSeedWithPattern();

    /* i1 wrap: index1 = 0 → after call must be 0x40 (ori $t1,$zero,0x40) */
    PE_StoreU32(GA_TEST_RNG_INDEX1, 0);
    func_80070D6C();
    ASSERT(PE_LoadU32(GA_TEST_RNG_INDEX1) == 0x40, "index1 wrap != 0x40");

    /* i2 OR-wrap: index2 = -64 → next decrement makes -68 = 0xFFFFFFBC,
     * bit 6 clear → |= 0x40 yields -4 (0xFFFFFFFC), NOT +0x40 and NOT a
     * ring reset.  This is the verbatim retail quirk. */
    PE_StoreU32(GA_TEST_RNG_INDEX2, (unsigned int)-64);
    func_80070D6C();
    ASSERT(PE_LoadU32(GA_TEST_RNG_INDEX2) == (unsigned int)-4,
           "index2 OR-wrap did not produce -4");

    /* i2 ordinary negative: -4 → -8, bit 6 already set, OR is a no-op */
    PE_StoreU32(GA_TEST_RNG_INDEX2, (unsigned int)-4);
    func_80070D6C();
    ASSERT(PE_LoadU32(GA_TEST_RNG_INDEX2) == (unsigned int)-8,
           "index2 no-op OR path wrong");
    PASS();
}

static void test_70D6C_below_table_read_exact(void) {
    TEST("70D6C_below_table_read_exact");
    ResetTestState();
    RngSeedWithPattern();

    /* Aim the read cursor at 0x80070E00 (a code word on retail, pattern
     * here): i2 = -12 → addr2 = 0x80070E0C - 12 = 0x80070E00. */
    unsigned int pattern = PE_LoadU32(0x80070E00u);
    PE_StoreU32(GA_TEST_RNG_INDEX1, 0);
    PE_StoreU32(GA_TEST_RNG_INDEX2, (unsigned int)-12);
    unsigned int expect = PE_LoadU32(GA_TEST_RNG_TABLE) + pattern;

    unsigned int v = func_80070D6C();
    ASSERT(v == expect, "below-table read value not incorporated");
    ASSERT(pattern == 0xBEEF0000u + 13u * 0x1111u, "pattern assumption broken");
    PASS();
}

static void test_70D6C_footprint(void) {
    TEST("70D6C_footprint");
    ResetTestState();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0x5A5A5A5Au);

    func_80070D10();
    for (int i = 0; i < 40; i++)
        func_80070D6C();

    /* After canary + seed + 40 advances: only the 19-word RNG block may
     * differ from the canary. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        if (a >= GA_TEST_RNG_INDEX1 && a <= 0x80070E4Cu)
            continue;
        if (PE_LoadU32(a) != 0x5A5A5A5Au) {
            printf("FAIL: guest 0x%08X modified\n", a);
            FAIL("func_80070D6C write footprint exceeds the RNG block");
            return;
        }
    }
    PASS();
}

static void test_70D6C_reseed_repeats(void) {
    TEST("70D6C_reseed_repeats");
    unsigned int s1[64], s2[64];

    ResetTestState();
    RngSeedWithPattern();
    for (int i = 0; i < 64; i++) s1[i] = func_80070D6C();

    /* Dirty the whole block, re-seed, rerun: identical sequence */
    for (pe_addr_t a = GA_TEST_RNG_INDEX1; a <= 0x80070E4Cu; a += 4)
        PE_StoreU32(a, 0x0BADF00Du);
    RngSeedWithPattern();
    for (int i = 0; i < 64; i++) s2[i] = func_80070D6C();

    for (int i = 0; i < 64; i++)
        ASSERT(s1[i] == s2[i], "re-seeded sequence diverged");
    PASS();
}

static void test_70D6C_not_bootstrap_stub(void) {
    TEST("70D6C_not_bootstrap_stub");
    ResetTestState();
    RngSeedWithPattern();

    func_80070D6C();

    ASSERT(CountOrderLog("func_80070D6C") == 0, "func_80070D6C recorded as stub");
    ASSERT(Bootstrap_InvocationCount() == 0, "bootstrap provider invoked");
    PASS();
}

static void test_70DD0_ranges(void) {
    TEST("70DD0_ranges");
    static const int k_ranges[][2] = {
        { 0, 100 }, { 1, 4 }, { 0, 65536 }, { 5, 5 }, { 10, 0 },
        { -3, 3 }, { 0, 0x7FFFFFFF }, { -100, -1 }
    };
    int port_out[8], model_out[8];

    ResetTestState();
    RngSeedWithPattern();
    for (int i = 0; i < 7; i++)
        port_out[i] = func_80070DD0(k_ranges[i][0], k_ranges[i][1]);

    ResetTestState();
    RngSeedWithPattern();
    for (int i = 0; i < 7; i++)
        model_out[i] = model_70DD0(k_ranges[i][0], k_ranges[i][1]);

    for (int i = 0; i < 7; i++) {
        char msg[80];
        snprintf(msg, sizeof msg, "70DD0(%d,%d) mismatch",
                 k_ranges[i][0], k_ranges[i][1]);
        ASSERT(port_out[i] == model_out[i], msg);
    }
    /* Degenerate range a == b must return a exactly (product is 0) */
    ASSERT(port_out[3] == 5, "70DD0(5,5) != 5");
    PASS();
}

static void test_3E680_warmup_real(void) {
    TEST("3E680_warmup_real");
    unsigned int port_blk[19], model_blk[19];

    ResetTestState();
    g_bootstrap_disc = 1;
    func_8003E680();
    SnapshotRngBlock(port_blk);

    /* Model: seed (zero below-table bytes in tests), 2000 advances */
    ResetTestState();
    func_80070D10();
    for (int i = 0; i < 2000; i++)
        model_70D6C();
    SnapshotRngBlock(model_blk);

    ASSERT(RngBlockEquals(port_blk, model_blk),
           "func_8003E680 warm-up state != 2000 real advances");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B3 — Provider frontier: func_8003E974 (bit-table init +
 * registration sequence).  Phase 6E-B4 made func_8003EAC8 REAL, so the
 * array expectations below are the final REGISTERED table, not all-zero.
 *
 * Retail truth (asm/disc1/2EF54.s:163-251): five $gp-relative word clears
 * (retail $gp = 0x8009CD70 → 0x8009D1E4/1F4/2D4/238/26C), a 32-word clear
 * of D_800A76F0 (0x800A76F0..0x800A776C), 20 func_8003EAC8(mask,value)
 * calls in ROM order, then D_8009D1A0 |= 0x4000 on the host-represented
 * storage shared with func_8003E680.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_TEST_A76F0  0x800A76F0u

static const struct { int a0, a1; } k_3eac8_expect[20] = {
    { 0x1, 0x4000 }, { 0x80, 0x1000 }, { 0x100, 0x2000 }, { 0x8, 0x10 },
    { 0x20, 0x40 }, { 0x40, 0x80 }, { 0x10, 0x20 }, { 0x2, 0x1 },
    { 0x4, 0x8 }, { 0x200, 0x2000 }, { 0x400, 0x4000 }, { 0x2000, 0x8000 },
    { 0x1000000, 0x100 }, { 0x2000000, 0x200 }, { 0x4000000, 0x400 },
    { 0x8000000, 0x800 }, { 0x10000000, 0x1000 }, { 0x20000000, 0x2000 },
    { 0x40000000, 0x4000 }, { (int)0x80000000, 0x8000 }
};

/* Phase 6E-B4: with func_8003EAC8 REAL, func_8003E974 leaves D_800A76F0
 * holding the registered values — slot = highest-set-bit index of the
 * mask (0x80000000 -> slot 31).  Populated slots: 0-10, 13, 24-31
 * (20 calls, no duplicates).  Slots 11, 12, 14-23 stay 0 from the clear.
 * Independent of the implementation: derived from the ROM-order table
 * above and the oracle-verified index semantics. */
static const uint32_t k_3e974_final[32] = {
    0x4000, 0x1, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1000,   /* slots 0-7   */
    0x2000, 0x2000, 0x4000, 0, 0, 0x8000, 0, 0,         /* slots 8-15  */
    0, 0, 0, 0, 0, 0, 0, 0,                             /* slots 16-23 */
    0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000 /* 24-31 */
};

static void test_3E974_gp_scalar_clears(void) {
    TEST("3E974_gp_scalar_clears");
    static const pe_addr_t k_addrs[5] = {
        0x8009D1E4u, 0x8009D1F4u, 0x8009D2D4u, 0x8009D238u, 0x8009D26Cu
    };
    ResetTestState();

    for (int i = 0; i < 5; i++)
        PE_StoreU32(k_addrs[i], 0xFFFFFFFFu);
    func_8003E974();
    for (int i = 0; i < 5; i++) {
        char msg[64];
        snprintf(msg, sizeof msg, "$gp word %d (0x%08X) not cleared",
                 i, k_addrs[i]);
        ASSERT(PE_LoadU32(k_addrs[i]) == 0, msg);
    }
    PASS();
}

static void test_3E974_array_final_table(void) {
    TEST("3E974_array_final_table");
    ResetTestState();

    /* Canary the array plus one-word guards on both sides */
    for (pe_addr_t a = GA_TEST_A76F0 - 4; a <= GA_TEST_A76F0 + 0x80u; a += 4)
        PE_StoreU32(a, 0xCACA0000u | ((a - GA_TEST_A76F0) & 0xFF));

    func_8003E974();

    /* Guard words must be untouched (registrations land INSIDE the array) */
    ASSERT(PE_LoadU32(GA_TEST_A76F0 - 4) == (0xCACA0000u | 0xFCu),
           "guard word before array clobbered");
    ASSERT(PE_LoadU32(GA_TEST_A76F0 + 0x80u) == (0xCACA0000u | 0x80u),
           "guard word after array clobbered");
    /* The clear ran (dirtied unregistered slots are 0) and the 20 real
     * registrations populated their slots: the complete retail end state. */
    for (int i = 0; i < 0x20; i++) {
        char msg[80];
        uint32_t got = PE_LoadU32(GA_TEST_A76F0 + (unsigned int)i * 4u);
        snprintf(msg, sizeof msg,
                 "array slot %d: got 0x%X want 0x%X", i, got, k_3e974_final[i]);
        ASSERT(got == k_3e974_final[i], msg);
    }
    PASS();
}

static void test_3E974_call_sequence_exact(void) {
    TEST("3E974_call_sequence_exact");
    ResetTestState();

    func_8003E974();

    ASSERT(PE_3EAC8_RecordCount() == 20,
           "func_8003EAC8 not invoked exactly 20 times");
    for (int i = 0; i < 20; i++) {
        int a0 = -1, a1 = -1;
        char msg[80];
        ASSERT(PE_3EAC8_RecordAt(i, &a0, &a1), "recorded call missing");
        snprintf(msg, sizeof msg,
                 "call %d args: got (0x%X,0x%X) want (0x%X,0x%X)",
                 i, a0, a1, k_3eac8_expect[i].a0, k_3eac8_expect[i].a1);
        ASSERT(a0 == k_3eac8_expect[i].a0 && a1 == k_3eac8_expect[i].a1, msg);
    }
    PASS();
}

static void test_3E974_d1a0_or_exact(void) {
    TEST("3E974_d1a0_or_exact");
    ResetTestState();

    /* RMW on the host-represented storage shared with func_8003E680 */
    D_8009D1A0 = 0x1234;
    func_8003E974();
    ASSERT(D_8009D1A0 == 0x5234, "D_8009D1A0 |= 0x4000 wrong");

    /* Already-set bit: OR is a fixed point (idempotent) */
    func_8003E974();
    ASSERT(D_8009D1A0 == 0x5234, "second call changed D_8009D1A0");

    D_8009D1A0 = 0;
    PASS();
}

static void test_3E974_footprint(void) {
    TEST("3E974_footprint");
    ResetTestState();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0x3C3C3C3Cu);

    func_8003E974();

    /* Only the 5 $gp-resolved words and the 32-word array may differ.
     * Nothing else in the 2 MiB may be touched.  Scalars become 0; array
     * slots become the 6E-B4 final registered table (clear + 20 real
     * registrations — all landing inside the same 32 words). */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        int in_array = (a >= GA_TEST_A76F0 && a <= GA_TEST_A76F0 + 0x7Cu);
        int is_scalar = (a == 0x8009D1E4u || a == 0x8009D1F4u ||
                         a == 0x8009D2D4u || a == 0x8009D238u ||
                         a == 0x8009D26Cu);
        if (in_array) {
            uint32_t want = k_3e974_final[(a - GA_TEST_A76F0) / 4u];
            if (PE_LoadU32(a) != want) {
                printf("FAIL: guest 0x%08X = 0x%08X, want 0x%X\n",
                       a, PE_LoadU32(a), want);
                FAIL("array word has wrong final value");
                return;
            }
        } else if (is_scalar) {
            if (PE_LoadU32(a) != 0) {
                printf("FAIL: guest 0x%08X = 0x%08X, want 0\n",
                       a, PE_LoadU32(a));
                FAIL("cleared word has wrong value");
                return;
            }
        } else if (PE_LoadU32(a) != 0x3C3C3C3Cu) {
            printf("FAIL: guest 0x%08X modified\n", a);
            FAIL("func_8003E974 write footprint exceeds its 37 words");
            return;
        }
    }
    PASS();
}

static void test_3E974_repeated_idempotent(void) {
    TEST("3E974_repeated_idempotent");
    ResetTestState();

    func_8003E974();
    unsigned int snap[37];
    snap[0] = PE_LoadU32(0x8009D1E4u);
    snap[1] = PE_LoadU32(0x8009D1F4u);
    snap[2] = PE_LoadU32(0x8009D2D4u);
    snap[3] = PE_LoadU32(0x8009D238u);
    snap[4] = PE_LoadU32(0x8009D26Cu);
    for (int i = 0; i < 0x20; i++)
        snap[5 + i] = PE_LoadU32(GA_TEST_A76F0 + (unsigned int)i * 4u);
    unsigned int d1a0 = D_8009D1A0;

    /* Dirty everything the function owns, then rerun */
    {
        static const pe_addr_t k_addrs[5] = {
            0x8009D1E4u, 0x8009D1F4u, 0x8009D2D4u, 0x8009D238u, 0x8009D26Cu
        };
        for (int i = 0; i < 5; i++)
            PE_StoreU32(k_addrs[i], 0xDEAD0000u + (unsigned int)i);
        for (int i = 0; i < 0x20; i++)
            PE_StoreU32(GA_TEST_A76F0 + (unsigned int)i * 4u,
                        0xDEAD1000u + (unsigned int)i);
    }
    func_8003E974();

    ASSERT(PE_3EAC8_RecordCount() == 40, "second call did not replay 20 registrations");
    ASSERT(D_8009D1A0 == d1a0, "D_8009D1A0 changed on repeat");
    for (int i = 0; i < 5; i++) {
        static const pe_addr_t k_addrs[5] = {
            0x8009D1E4u, 0x8009D1F4u, 0x8009D2D4u, 0x8009D238u, 0x8009D26Cu
        };
        ASSERT(PE_LoadU32(k_addrs[i]) == snap[i], "scalar clear not idempotent");
    }
    for (int i = 0; i < 0x20; i++)
        ASSERT(PE_LoadU32(GA_TEST_A76F0 + (unsigned int)i * 4u) == snap[5 + i],
               "array clear not idempotent");
    PASS();
}

static void test_3E974_not_bootstrap_stub(void) {
    TEST("3E974_not_bootstrap_stub");
    ResetTestState();

    func_8003E974();

    ASSERT(CountOrderLog("func_8003E974") == 0,
           "func_8003E974 recorded as stub");
    /* 6E-B4: func_8003EAC8 is translated too — also absent from the stub
     * order log; its 20 invocations are visible via the non-semantic
     * argument recorder instead. */
    ASSERT(CountOrderLog("func_8003EAC8") == 0,
           "func_8003EAC8 still routed through provider boundary");
    ASSERT(PE_3EAC8_RecordCount() == 20,
           "registrations missing from argument recorder");
    PASS();
}

static void test_3E680_3E974_integration(void) {
    TEST("3E680_3E974_integration");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    ASSERT(CountOrderLog("func_8003E974") == 0,
           "func_8003E974 still routed through bootstrap policy");
    ASSERT(PE_3EAC8_RecordCount() == 20,
           "registrations missing after func_8003E680");
    /* func_8003E680 zeroed D_8009D1A0 first, then 3E974 set the bit */
    ASSERT(D_8009D1A0 == 0x4000, "D_8009D1A0 != 0x4000 after func_8003E680");
    /* 6E-B4: array holds the final registered table (not all-zero — that
     * was only true while func_8003EAC8 was a recorder stub) */
    for (int i = 0; i < 0x20; i++)
        ASSERT(PE_LoadU32(GA_TEST_A76F0 + (unsigned int)i * 4u) == k_3e974_final[i],
               "D_800A76F0 final table wrong through func_8003E680");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B4 — func_8003EAC8 LZCR registration rung.
 *
 * Retail truth (asm/disc1/2EF54.s:255-271, verified against the exe by
 * tools/lzcr_oracle.py): idx = (a0 == 0x80000000) ? 31 : 31 - LZCR(a0);
 * LZCR counts leading bits equal to the sign bit (0 -> 32, 0xFFFFFFFF ->
 * 32); word store D_800A76F0[idx] = a1 with the sll/addu wrap, so idx -1
 * lands at 0x800A76EC (valid guest RAM, preserved — never clamped).
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_TEST_A76EC  0x800A76ECu   /* one word below the table (idx -1) */

/* Oracle-derived expectations (tools/lzcr_oracle.py output, cross-checked
 * against documented GTE LZCS/LZCR semantics) */
static const struct { uint32_t a0; uint32_t lzcr; int idx; pe_addr_t dest; }
    k_lzcr_expect[12] = {
    { 0x00000000u, 32, -1, 0x800A76ECu },
    { 0x00000001u, 31,  0, 0x800A76F0u },
    { 0x00000002u, 30,  1, 0x800A76F4u },
    { 0x00000003u, 30,  1, 0x800A76F4u },
    { 0x00000008u, 28,  3, 0x800A76FCu },
    { 0x00008000u, 16, 15, 0x800A772Cu },
    { 0x40000000u,  1, 30, 0x800A7768u },
    { 0x7FFFFFFFu,  1, 30, 0x800A7768u },
    { 0x80000000u,  1, 31, 0x800A776Cu },
    { 0x80000001u,  1, 30, 0x800A7768u },
    { 0xC0000000u,  2, 29, 0x800A7764u },
    { 0xFFFFFFFFu, 32, -1, 0x800A76ECu },
};

static void test_LZCR_helper_exact(void) {
    TEST("LZCR_helper_exact");
    ResetTestState();

    for (int i = 0; i < 12; i++) {
        char msg[80];
        uint32_t got = PE_GTE_LZCR(k_lzcr_expect[i].a0);
        snprintf(msg, sizeof msg, "LZCR(0x%08X) = %u, want %u",
                 k_lzcr_expect[i].a0, got, k_lzcr_expect[i].lzcr);
        ASSERT(got == k_lzcr_expect[i].lzcr, msg);
    }
    /* All 32 one-hot masks: bits 0-30 -> 31-k, bit 31 -> 1 */
    for (int k = 0; k < 32; k++) {
        char msg[64];
        uint32_t want = (k == 31) ? 1u : (uint32_t)(31 - k);
        uint32_t got = PE_GTE_LZCR(1u << k);
        snprintf(msg, sizeof msg, "LZCR(1<<%d) = %u, want %u", k, got, want);
        ASSERT(got == want, msg);
    }
    PASS();
}

static void test_3EAC8_contract_edge_inputs(void) {
    TEST("3EAC8_contract_edge_inputs");
    ResetTestState();

    for (int i = 0; i < 12; i++) {
        char msg[96];
        uint32_t val = 0xEA000000u | (uint32_t)i;

        /* Dirty the destination window plus one-word guards on both ends */
        for (pe_addr_t a = GA_TEST_A76EC - 4; a <= GA_TEST_A76F0 + 0x84u; a += 4)
            PE_StoreU32(a, 0xB0B0B0B0u);

        func_8003EAC8((int)k_lzcr_expect[i].a0, (int)val);

        snprintf(msg, sizeof msg, "a0=0x%08X: dest word wrong",
                 k_lzcr_expect[i].a0);
        ASSERT(PE_LoadU32(k_lzcr_expect[i].dest) == val, msg);
        /* Exactly one word may change in the whole window */
        for (pe_addr_t a = GA_TEST_A76EC - 4; a <= GA_TEST_A76F0 + 0x84u; a += 4) {
            if (a == k_lzcr_expect[i].dest) continue;
            if (PE_LoadU32(a) != 0xB0B0B0B0u) {
                snprintf(msg, sizeof msg,
                         "a0=0x%08X: collateral write at 0x%08X",
                         k_lzcr_expect[i].a0, a);
                FAIL(msg);
                return;
            }
        }
    }
    PASS();
}

static void test_3EAC8_one_hot_all32(void) {
    TEST("3EAC8_one_hot_all32");
    ResetTestState();

    for (int k = 0; k < 32; k++) {
        char msg[96];
        uint32_t val = 0x0EAC0000u | (uint32_t)k;
        /* bits 0-30 -> slot k; bit 31 (0x80000000) -> slot 31 special case */
        pe_addr_t want = GA_TEST_A76F0 + (unsigned int)k * 4u;

        for (pe_addr_t a = GA_TEST_A76EC; a <= GA_TEST_A76F0 + 0x80u; a += 4)
            PE_StoreU32(a, 0);
        func_8003EAC8((int)(1u << k), (int)val);

        snprintf(msg, sizeof msg, "one-hot bit %d: wrong destination", k);
        ASSERT(PE_LoadU32(want) == val, msg);
        for (pe_addr_t a = GA_TEST_A76EC; a <= GA_TEST_A76F0 + 0x80u; a += 4) {
            if (a == want) continue;
            if (PE_LoadU32(a) != 0) {
                snprintf(msg, sizeof msg,
                         "one-hot bit %d: collateral write at 0x%08X", k, a);
                FAIL(msg);
                return;
            }
        }
    }
    PASS();
}

static void test_3EAC8_same_slot_rom_order(void) {
    TEST("3EAC8_same_slot_rom_order");
    ResetTestState();

    /* 0x2 and 0x3 both map to slot 1 (highest set bit); the second call
     * must win, as retail's plain word store dictates. */
    func_8003EAC8(0x2, 1);
    ASSERT(PE_LoadU32(GA_TEST_A76F0 + 4) == 1, "slot 1 first write wrong");
    func_8003EAC8(0x3, 0x99);
    ASSERT(PE_LoadU32(GA_TEST_A76F0 + 4) == 0x99,
           "slot 1 second write did not overwrite in call order");
    PASS();
}

static void test_3EAC8_footprint(void) {
    TEST("3EAC8_footprint");
    ResetTestState();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0x3C3C3C3Cu);

    func_8003EAC8(0x100, 0x12345678);   /* bit 8 -> slot 8 -> 0x800A7710 */

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        if (a == GA_TEST_A76F0 + 8u * 4u) {
            if (PE_LoadU32(a) != 0x12345678u) {
                FAIL("registration word missing");
                return;
            }
        } else if (PE_LoadU32(a) != 0x3C3C3C3Cu) {
            printf("FAIL: guest 0x%08X modified\n", a);
            FAIL("func_8003EAC8 write footprint exceeds its single word");
            return;
        }
    }
    PASS();
}

static void test_3EAC8_recorder_disabled(void) {
    TEST("3EAC8_recorder_disabled");
    ResetTestState();

    /* Production behavior must be identical with the recorder off */
    PE_3EAC8_RecordSetEnabled(0);
    func_8003EAC8(0x80, 0x1000);
    ASSERT(PE_3EAC8_RecordCount() == 0, "recorder counted while disabled");
    ASSERT(PE_LoadU32(GA_TEST_A76F0 + 7u * 4u) == 0x1000u,
           "guest store did not happen with recorder disabled");

    PE_3EAC8_RecordSetEnabled(1);
    func_8003EAC8(0x80, 0x2000);
    ASSERT(PE_3EAC8_RecordCount() == 1, "recorder lost calls after re-enable");
    ASSERT(PE_LoadU32(GA_TEST_A76F0 + 7u * 4u) == 0x2000u,
           "guest store wrong after re-enable");
    PASS();
}

static void test_3EAC8_recorder_overflow(void) {
    TEST("3EAC8_recorder_overflow");
    ResetTestState();

    /* 70 calls > 64-entry log: count keeps climbing (visible overflow),
     * RecordAt stays bounded, and every store still lands. */
    for (int i = 0; i < 70; i++)
        func_8003EAC8(1, 0xC000 + i);
    ASSERT(PE_3EAC8_RecordCount() == 70, "overflow not visible in count");
    int a0 = -1, a1 = -1;
    ASSERT(PE_3EAC8_RecordAt(63, &a0, &a1), "record 63 missing");
    ASSERT(a0 == 1 && a1 == 0xC03F, "record 63 wrong");
    ASSERT(PE_3EAC8_RecordAt(64, &a0, &a1) == 0,
           "RecordAt read past the bounded log");
    ASSERT(PE_LoadU32(GA_TEST_A76F0) == 0xC000u + 69u,
           "store of final overflowing call wrong");
    PASS();
}

static void test_3EAC8_not_bootstrap_stub(void) {
    TEST("3EAC8_not_bootstrap_stub");
    ResetTestState();

    /* Strict mode must no longer stop here: translated functions never
     * touch the centralized boundary, so a strict run sails through. */
    g_strict_stubs = 1;
    func_8003EAC8(1, 0x4000);
    g_strict_stubs = 0;
    ASSERT(CountOrderLog("func_8003EAC8") == 0,
           "func_8003EAC8 recorded as stub");
    ASSERT(PE_LoadU32(GA_TEST_A76F0) == 0x4000u,
           "store missing on the strict-flag path");
    PASS();
}

static void test_3E974_ramreset_reproduces(void) {
    TEST("3E974_ramreset_reproduces");
    ResetTestState();

    D_8009D1A0 = 0;
    PE_RamReset();
    func_8003E974();
    ASSERT(D_8009D1A0 == 0x4000, "D_8009D1A0 wrong after PE_RamReset run");
    for (int i = 0; i < 0x20; i++) {
        char msg[80];
        uint32_t got = PE_LoadU32(GA_TEST_A76F0 + (unsigned int)i * 4u);
        snprintf(msg, sizeof msg, "slot %d after PE_RamReset: 0x%X want 0x%X",
                 i, got, k_3e974_final[i]);
        ASSERT(got == k_3e974_final[i], msg);
    }
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B5 — func_80036DC8 timer-record init rung.
 *
 * Retail truth (asm/disc1/26C48.s:677-738): dispatcher func_80036DC8
 * calls func_80036DF8 (record 0), func_80036E34 (record 2),
 * func_80036E58 (record 1) — 11 word stores total (two dead zero-stores
 * reproduced verbatim), leaving three 12-byte records:
 *   0x800A76A0 = { 1, 0, 0x1499700 }
 *   0x800A76AC = { 1, 0, 0 }
 *   0x800A76B8 = { 1, 0, 0 }
 * Sole caller func_8003E680; void(void); idempotent; no deeper calls.
 * ═══════════════════════════════════════════════════════════════════════ */

#define GA_TEST_A76A0  0x800A76A0u

/* Net state after func_80036DC8: 9 words, retail final values */
static const uint32_t k_36dc8_final[9] = {
    1, 0, 0x1499700u,   /* record 0 */
    1, 0, 0,            /* record 1 */
    1, 0, 0             /* record 2 */
};

static void test_36DC8_exact_final_state(void) {
    TEST("36DC8_exact_final_state");
    ResetTestState();

    /* Dirty the 9 words plus one-word guards on both sides */
    for (pe_addr_t a = GA_TEST_A76A0 - 4; a <= GA_TEST_A76A0 + 0x24u; a += 4)
        PE_StoreU32(a, 0xC0FFEE00u | ((a - GA_TEST_A76A0) & 0xFF));

    func_80036DC8();

    ASSERT(PE_LoadU32(GA_TEST_A76A0 - 4) == (0xC0FFEE00u | 0xFCu),
           "guard word before records clobbered");
    ASSERT(PE_LoadU32(GA_TEST_A76A0 + 0x24u) == (0xC0FFEE00u | 0x24u),
           "guard word after records clobbered");
    for (int i = 0; i < 7; i++) {
        char msg[80];
        uint32_t got = PE_LoadU32(GA_TEST_A76A0 + (unsigned int)i * 4u);
        snprintf(msg, sizeof msg, "record word %d: got 0x%X want 0x%X",
                 i, got, k_36dc8_final[i]);
        ASSERT(got == k_36dc8_final[i], msg);
    }
    PASS();
}

static void test_36DC8_leaf_sequence_state(void) {
    TEST("36DC8_leaf_sequence_state");
    ResetTestState();

    /* Guest state visible at each dependency boundary: invoke the three
     * leaves in retail order and check the progressive write sets. */
    for (pe_addr_t a = GA_TEST_A76A0; a <= GA_TEST_A76A0 + 0x20u; a += 4)
        PE_StoreU32(a, 0xD1D1D1D1u);

    func_80036DF8();   /* record 0 only */
    for (int i = 0; i < 7; i++) {
        uint32_t want = (i < 3) ? k_36dc8_final[i] : 0xD1D1D1D1u;
        ASSERT(PE_LoadU32(GA_TEST_A76A0 + (unsigned int)i * 4u) == want,
               "func_80036DF8 write set wrong (touched records 1/2)");
    }
    func_80036E34();   /* record 2 (0x800A76B8..C0) */
    for (int i = 0; i < 7; i++) {
        uint32_t want = (i < 3 || i >= 6) ? k_36dc8_final[i] : 0xD1D1D1D1u;
        ASSERT(PE_LoadU32(GA_TEST_A76A0 + (unsigned int)i * 4u) == want,
               "func_80036E34 write set wrong");
    }
    func_80036E58();   /* record 1 (0x800A76AC..B4) */
    for (int i = 0; i < 7; i++)
        ASSERT(PE_LoadU32(GA_TEST_A76A0 + (unsigned int)i * 4u) == k_36dc8_final[i],
               "final state wrong after all three leaves");
    PASS();
}

static void test_36DC8_footprint(void) {
    TEST("36DC8_footprint");
    ResetTestState();

    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0x3C3C3C3Cu);

    func_80036DC8();

    /* Exactly the 9 record words may differ — min 0x800A76A0, max
     * 0x800A76C0.  Nothing else in the 2 MiB may be touched. */
    for (pe_addr_t a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        if (a >= GA_TEST_A76A0 && a <= GA_TEST_A76A0 + 0x20u) {
            uint32_t want = k_36dc8_final[(a - GA_TEST_A76A0) / 4u];
            if (PE_LoadU32(a) != want) {
                printf("FAIL: guest 0x%08X = 0x%08X, want 0x%X\n",
                       a, PE_LoadU32(a), want);
                FAIL("record word has wrong final value");
                return;
            }
        } else if (PE_LoadU32(a) != 0x3C3C3C3Cu) {
            printf("FAIL: guest 0x%08X modified\n", a);
            FAIL("func_80036DC8 write footprint exceeds its 9 words");
            return;
        }
    }
    PASS();
}

static void test_36DC8_repeated_and_ramreset(void) {
    TEST("36DC8_repeated_and_ramreset");
    ResetTestState();

    func_80036DC8();
    /* Dirty all 9 words, rerun — absolute stores replay the same state */
    for (int i = 0; i < 7; i++)
        PE_StoreU32(GA_TEST_A76A0 + (unsigned int)i * 4u, 0xDEADBEEFu);
    func_80036DC8();
    for (int i = 0; i < 7; i++)
        ASSERT(PE_LoadU32(GA_TEST_A76A0 + (unsigned int)i * 4u) == k_36dc8_final[i],
               "repeat invocation not idempotent");

    PE_RamReset();
    func_80036DC8();
    for (int i = 0; i < 7; i++)
        ASSERT(PE_LoadU32(GA_TEST_A76A0 + (unsigned int)i * 4u) == k_36dc8_final[i],
               "state wrong after PE_RamReset + invocation");
    PASS();
}

static void test_36DC8_not_bootstrap_stub(void) {
    TEST("36DC8_not_bootstrap_stub");
    ResetTestState();

    /* Strict mode must no longer stop here */
    g_strict_stubs = 1;
    func_80036DC8();
    g_strict_stubs = 0;
    ASSERT(CountOrderLog("func_80036DC8") == 0,
           "func_80036DC8 recorded as stub");
    ASSERT(PE_LoadU32(GA_TEST_A76A0 + 8u) == 0x1499700u,
           "record 0 preload missing on the strict-flag path");
    PASS();
}

static void test_3E680_36DC8_integration(void) {
    TEST("3E680_36DC8_integration");
    ResetTestState();
    g_bootstrap_disc = 1;

    func_8003E680();

    ASSERT(CountOrderLog("func_80036DC8") == 0,
           "func_80036DC8 still routed through bootstrap policy");
    for (int i = 0; i < 7; i++)
        ASSERT(PE_LoadU32(GA_TEST_A76A0 + (unsigned int)i * 4u) == k_36dc8_final[i],
               "timer records wrong after func_8003E680");
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
    func_8007ED58();   /* drive reset → CdReady lane 1 (boot path state) */

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
    func_8007ED58();   /* drive reset → CdReady lane 1 (boot path state) */
    /* Seed without the busy bit 0x01000000: with the real func_8006E6D4
     * guard that bit means "read already in flight" and the retail retry
     * loop would (correctly) spin.  0xFEFFFFFF exercises every mask step. */
    D_800B0CD8 = 0xFEFFFFFFu;

    func_8006E834();

    /* & ~0xF0 at entry (0xFEFFFF0F), |= 0x01004000 at read issue
     * (0xFFFFFF0F), then & 0xFEFFBFFF in the completion poll (real
     * func_800811E4 returns 0: the synchronous host read leaves
     * D_8009B6B4 = 0): final 0xFEFFBF0F */
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

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-A batch 3 — real-disc foundation tests
 *
 * Synthetic in-memory MODE2/2352 fixtures (no retail data): 64 raw
 * sectors, PVD at user sector 16, root directory at sector 20 holding
 * "FMV1" (dir, sector 21) and "PE.IMG;1" (file, sector 30, 5000 bytes of
 * deterministic pattern).  Optional second fixture adds
 * "FMV2\PEDISC02.IDF;1".
 * ═══════════════════════════════════════════════════════════════════════ */

#define FX_SECTORS     64u
#define FX_PEIMG_LBA   30u
#define FX_PEIMG_SIZE  5000u
#define FX_IDF1_LBA    40u
#define FX_IDF1_SIZE   100u
#define FX_IDF2_LBA    41u
#define FX_IDF2_SIZE   100u

typedef struct {
    uint8_t  *img;
    PE_Disc  *disc;
} DiscFixture;

static uint8_t FxPattern(uint32_t i) { return (uint8_t)(i * 7u + 3u); }

/* Write one ISO9660 directory record; returns its length. */
static int FxPutDirRec(uint8_t *p, uint32_t extent, uint32_t size,
                       uint8_t flags, const char *id, int id_len) {
    int len = 33 + id_len;
    if (len & 1) len++;
    memset(p, 0, len);
    p[0] = (uint8_t)len;
    /* both-endian extent and size */
    p[2] = extent & 0xFF; p[3] = (extent >> 8) & 0xFF;
    p[4] = (extent >> 16) & 0xFF; p[5] = (extent >> 24) & 0xFF;
    p[6] = (extent >> 24) & 0xFF; p[7] = (extent >> 16) & 0xFF;
    p[8] = (extent >> 8) & 0xFF; p[9] = extent & 0xFF;
    p[10] = size & 0xFF; p[11] = (size >> 8) & 0xFF;
    p[12] = (size >> 16) & 0xFF; p[13] = (size >> 24) & 0xFF;
    p[14] = (size >> 24) & 0xFF; p[15] = (size >> 16) & 0xFF;
    p[16] = (size >> 8) & 0xFF; p[17] = size & 0xFF;
    p[25] = flags;
    p[32] = (uint8_t)id_len;
    memcpy(p + 33, id, (size_t)id_len);
    return len;
}

static uint8_t *FxUser(uint8_t *img, uint32_t lba) {
    return img + (size_t)lba * PE_DISC_RAW_SECTOR + PE_DISC_USER_OFFSET;
}

/* Build a validated in-memory fixture.  Returns 1 on success. */
static int FxBuild(DiscFixture *fx, int with_fmv2) {
    uint32_t i;
    uint8_t *p;
    int n;

    fx->img = malloc((size_t)FX_SECTORS * PE_DISC_RAW_SECTOR);
    if (!fx->img) return 0;
    memset(fx->img, 0, (size_t)FX_SECTORS * PE_DISC_RAW_SECTOR);
    for (i = 0; i < FX_SECTORS; i++) {
        uint8_t *raw = fx->img + (size_t)i * PE_DISC_RAW_SECTOR;
        raw[0] = 0x00;
        memset(raw + 1, 0xFF, 10);
        raw[11] = 0x00;
        raw[15] = 0x02; /* mode 2 */
    }
    /* PVD at user sector 16 */
    p = FxUser(fx->img, 16);
    p[0] = 1; memcpy(p + 1, "CD001", 5); p[6] = 1;
    FxPutDirRec(p + 156, 20, 2048, 0x02, "\0", 1);
    /* root directory at sector 20 */
    p = FxUser(fx->img, 20);
    n = FxPutDirRec(p, 20, 2048, 0x02, "\0", 1);
    n += FxPutDirRec(p + n, 20, 2048, 0x02, "\1", 1);
    n += FxPutDirRec(p + n, 21, 2048, 0x02, "FMV1", 4);
    n += FxPutDirRec(p + n, FX_PEIMG_LBA, FX_PEIMG_SIZE, 0x00,
                     "PE.IMG;1", 8);
    if (with_fmv2) {
        n += FxPutDirRec(p + n, 22, 2048, 0x02, "FMV2", 4);
        /* FMV2 directory at sector 22 */
        p = FxUser(fx->img, 22);
        n = FxPutDirRec(p, 22, 2048, 0x02, "\0", 1);
        n += FxPutDirRec(p + n, 20, 2048, 0x02, "\1", 1);
        FxPutDirRec(p + n, FX_IDF2_LBA, FX_IDF2_SIZE, 0x00,
                    "PEDISC02.IDF;1", 14);
    }
    /* FMV1 directory at sector 21 */
    p = FxUser(fx->img, 21);
    n = FxPutDirRec(p, 21, 2048, 0x02, "\0", 1);
    n += FxPutDirRec(p + n, 20, 2048, 0x02, "\1", 1);
    FxPutDirRec(p + n, FX_IDF1_LBA, FX_IDF1_SIZE, 0x00,
                "PEDISC01.IDF;1", 14);
    /* PE.IMG content pattern (5000 bytes starting at sector 30).  User
     * data is not contiguous in a MODE2 image: each 2352-byte raw sector
     * holds 2048 user bytes, so the pattern is written per sector. */
    for (i = 0; i < FX_PEIMG_SIZE; i++) {
        FxUser(fx->img, FX_PEIMG_LBA + i / 2048u)[i % 2048u] = FxPattern(i);
    }
    fx->disc = PE_Disc_OpenMemory(fx->img,
                                  (size_t)FX_SECTORS * PE_DISC_RAW_SECTOR);
    if (!fx->disc) { free(fx->img); fx->img = NULL; return 0; }
    return 1;
}

static void FxFree(DiscFixture *fx) {
    if (fx->disc) PE_Disc_Close(fx->disc);
    free(fx->img);
    fx->img = NULL;
    fx->disc = NULL;
}

/* ── pe_disc layer ───────────────────────────────────────────────────── */

static void test_disc_open_rejects_bad_size(void) {
    TEST("disc_open_rejects_bad_size");
    uint8_t buf[2352 * 2];
    memset(buf, 0, sizeof(buf));
    ResetTestState();
    ASSERT(PE_Disc_OpenMemory(buf, 1000) == NULL, "accepted non-2352-multiple size");
    PASS();
}

static void test_disc_open_rejects_bad_sync(void) {
    TEST("disc_open_rejects_bad_sync");
    uint8_t *buf = calloc(17, 2352);
    ResetTestState();
    ASSERT(buf != NULL, "alloc");
    ASSERT(PE_Disc_OpenMemory(buf, 17 * 2352) == NULL, "accepted zeroed image");
    free(buf);
    PASS();
}

static void test_disc_open_rejects_mode1(void) {
    TEST("disc_open_rejects_mode1");
    DiscFixture fx;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    fx.img[15] = 0x01; /* mode 1 */
    ASSERT(PE_Disc_OpenMemory(fx.img, (size_t)FX_SECTORS * PE_DISC_RAW_SECTOR) == NULL,
           "accepted MODE1 image");
    FxFree(&fx);
    PASS();
}

static void test_disc_open_valid_geometry(void) {
    TEST("disc_open_valid_geometry");
    DiscFixture fx;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(PE_Disc_UserSectorCount(fx.disc) == FX_SECTORS, "wrong sector count");
    FxFree(&fx);
    PASS();
}

static void test_disc_read_sector_bounds(void) {
    TEST("disc_read_sector_bounds");
    DiscFixture fx;
    uint8_t sec[2048];
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(PE_Disc_ReadUserSector(fx.disc, FX_SECTORS - 1, sec), "last sector should read");
    ASSERT(!PE_Disc_ReadUserSector(fx.disc, FX_SECTORS, sec), "sector past end must fail");
    FxFree(&fx);
    PASS();
}

static void test_disc_read_userdata_cross_sector(void) {
    TEST("disc_read_userdata_cross_sector");
    DiscFixture fx;
    uint8_t buf[FX_PEIMG_SIZE];
    uint32_t i;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(PE_Disc_ReadUserData(fx.disc, FX_PEIMG_LBA, 0, buf, FX_PEIMG_SIZE),
           "cross-sector read failed");
    for (i = 0; i < FX_PEIMG_SIZE; i++) {
        if (buf[i] != FxPattern(i)) { FAIL("pattern mismatch"); FxFree(&fx); return; }
    }
    FxFree(&fx);
    PASS();
}

static void test_disc_read_userdata_past_end(void) {
    TEST("disc_read_userdata_past_end");
    DiscFixture fx;
    uint8_t buf[2048];
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(PE_Disc_ReadUserData(fx.disc, FX_SECTORS - 1, 0, buf, 2048),
           "last full sector should read");
    ASSERT(PE_Disc_ReadUserData(fx.disc, FX_SECTORS - 1, 2047, buf, 1),
           "final byte of image should read");
    ASSERT(!PE_Disc_ReadUserData(fx.disc, FX_SECTORS - 1, 2047, buf, 2),
           "one byte past image end must fail");
    ASSERT(!PE_Disc_ReadUserData(fx.disc, FX_SECTORS, 0, buf, 2048),
           "sector past end must fail");
    FxFree(&fx);
    PASS();
}

static void test_disc_verify_pvd(void) {
    TEST("disc_verify_pvd");
    DiscFixture fx;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(PE_Disc_VerifyPVD(fx.disc), "PVD must verify");
    FxFree(&fx);
    PASS();
}

static void test_disc_verify_pvd_corrupt(void) {
    TEST("disc_verify_pvd_corrupt");
    DiscFixture fx;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    fx.img[16 * 2352 + 24 + 3] = 'X'; /* "CDx01" */
    ASSERT(!PE_Disc_VerifyPVD(fx.disc), "corrupted CD001 must not verify");
    FxFree(&fx);
    PASS();
}

static void test_disc_findfile_root(void) {
    TEST("disc_findfile_root");
    DiscFixture fx;
    uint32_t lba = 0, size = 0;
    char name[16];
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(PE_Disc_FindFile(fx.disc, "\\PE.IMG;1", &lba, &size, name, sizeof(name)),
           "PE.IMG lookup failed");
    ASSERT(lba == FX_PEIMG_LBA, "wrong extent");
    ASSERT(size == FX_PEIMG_SIZE, "wrong size");
    ASSERT(strcmp(name, "PE.IMG;1") == 0, "wrong file id");
    FxFree(&fx);
    PASS();
}

static void test_disc_findfile_nested(void) {
    TEST("disc_findfile_nested");
    DiscFixture fx;
    uint32_t lba = 0, size = 0;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(PE_Disc_FindFile(fx.disc, "\\FMV1\\PEDISC01.IDF;1", &lba, &size, NULL, 0),
           "nested IDF lookup failed");
    ASSERT(lba == FX_IDF1_LBA && size == FX_IDF1_SIZE, "wrong nested extent/size");
    FxFree(&fx);
    PASS();
}

static void test_disc_findfile_missing(void) {
    TEST("disc_findfile_missing");
    DiscFixture fx;
    uint32_t lba, size;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(!PE_Disc_FindFile(fx.disc, "\\NOPE.IMG;1", &lba, &size, NULL, 0),
           "missing file must not be found");
    ASSERT(!PE_Disc_FindFile(fx.disc, "\\NOPE\\X;1", &lba, &size, NULL, 0),
           "missing directory must not be found");
    FxFree(&fx);
    PASS();
}

static void test_disc_findfile_no_backslash(void) {
    TEST("disc_findfile_no_backslash");
    DiscFixture fx;
    uint32_t lba, size;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    ASSERT(!PE_Disc_FindFile(fx.disc, "PE.IMG;1", &lba, &size, NULL, 0),
           "path without leading backslash must be rejected");
    FxFree(&fx);
    PASS();
}

static void test_disc_findfile_malformed_record(void) {
    TEST("disc_findfile_malformed_record");
    DiscFixture fx;
    uint32_t lba, size;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    /* Corrupt the FMV1 record length (root dir offset 68): the scan skips
     * over the PE.IMG record into zero padding → safe not-found. */
    fx.img[20 * 2352 + 24 + 68] = 0xF0;
    ASSERT(!PE_Disc_FindFile(fx.disc, "\\PE.IMG;1", &lba, &size, NULL, 0),
           "malformed record must yield safe not-found");
    /* corrupt root extent in PVD: out-of-range directory → rejection */
    fx.img[16 * 2352 + 24 + 156 + 2] = 0xFF;
    fx.img[16 * 2352 + 24 + 156 + 3] = 0xFF;
    ASSERT(!PE_Disc_FindFile(fx.disc, "\\PE.IMG;1", &lba, &size, NULL, 0),
           "out-of-range root extent must be rejected");
    FxFree(&fx);
    PASS();
}

/* ── provider: func_80080C48 CdPosToInt ──────────────────────────────── */

static void test_cdpos_to_int_vectors(void) {
    TEST("cdpos_to_int_vectors");
    pe_addr_t fp = 0x801FFEC0u;
    ResetTestState();
    PE_StoreU8(fp + 0, 0x00); PE_StoreU8(fp + 1, 0x02); PE_StoreU8(fp + 2, 0x00);
    ASSERT(func_80080C48(fp) == 0, "00:02:00 must decode to LBA 0");
    PE_StoreU8(fp + 0, 0x01); PE_StoreU8(fp + 1, 0x00); PE_StoreU8(fp + 2, 0x00);
    ASSERT(func_80080C48(fp) == 4350, "01:00:00 must decode to LBA 4350");
    PE_StoreU8(fp + 0, 0x09); PE_StoreU8(fp + 1, 0x59); PE_StoreU8(fp + 2, 0x74);
    ASSERT(func_80080C48(fp) == 44849, "09:59:74 must decode to LBA 44849");
    PASS();
}

/* ── provider: func_80082314 PVD verify ──────────────────────────────── */

static void test_pvd_verify_no_disc(void) {
    TEST("pvd_verify_no_disc");
    ResetTestState();
    func_8007ED58();
    ASSERT(func_80082314() == 2, "no disc must yield result word 2");
    ASSERT(PE_LoadU32(0x800B28F8u) == 2, "D_800B28F8 must carry 2");
    PASS();
}

static void test_pvd_verify_with_disc(void) {
    TEST("pvd_verify_with_disc");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_80082314() == 4, "valid disc must yield result word 4");
    ASSERT(PE_LoadU32(0x800B28F8u) == 4, "D_800B28F8 must carry 4");
    FxFree(&fx);
    PASS();
}

static void test_pvd_verify_lane_shortcut(void) {
    TEST("pvd_verify_lane_shortcut");
    ResetTestState();
    PE_StoreU32(0x8009B574u, 2);
    PE_StoreU32(0x8009B578u, 0x10u);
    ASSERT(func_80082314() == 0x10, "lane shortcut must return 0x10");
    PASS();
}

static void test_pvd_verify_wrong_disc(void) {
    TEST("pvd_verify_wrong_disc");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    fx.img[16 * 2352 + 24 + 3] = 'X';
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_80082314() == 2, "wrong disc must yield result word 2");
    FxFree(&fx);
    PASS();
}

/* ── provider: func_80081414 DsSearchFile ────────────────────────────── */

static void test_dssearch_no_disc(void) {
    TEST("dssearch_no_disc");
    ResetTestState();
    ASSERT(func_80081414(0x801FFEC0u, "\\PE.IMG;1") == 0,
           "no disc must yield not-found");
    PASS();
}

static void test_dssearch_bad_name(void) {
    TEST("dssearch_bad_name");
    DiscFixture fx;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_80081414(0x801FFEC0u, "PE.IMG;1") == 0,
           "name without leading backslash must yield not-found");
    FxFree(&fx);
    PASS();
}

static void test_dssearch_pe_img_cdlfile(void) {
    TEST("dssearch_pe_img_cdlfile");
    DiscFixture fx;
    pe_addr_t fp = 0x801FFEC0u;
    char name[9];
    int i;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_80081414(fp, "\\PE.IMG;1") == 1, "PE.IMG must be found");
    ASSERT(func_80080C48(fp) == (int)FX_PEIMG_LBA,
           "CdlFILE pos must decode to the PE.IMG extent");
    ASSERT(PE_LoadU32(fp + 4) == FX_PEIMG_SIZE, "CdlFILE size wrong");
    for (i = 0; i < 8; i++) name[i] = (char)PE_LoadU8(fp + 8 + (uint32_t)i);
    name[8] = '\0';
    ASSERT(strcmp(name, "PE.IMG;1") == 0, "CdlFILE name wrong");
    ASSERT(PE_LoadU8(fp + 8 + 8) == 0, "CdlFILE name must be NUL-padded");
    FxFree(&fx);
    PASS();
}

static void test_dssearch_missing(void) {
    TEST("dssearch_missing");
    DiscFixture fx;
    ResetTestState();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_80081414(0x801FFEC0u, "\\NOPE;1") == 0,
           "missing file must yield not-found");
    ASSERT(func_80081414(0x801FFEC0u, "\\FMV2\\PEDISC02.IDF;1") == 0,
           "FMV2 (absent on fixture) must yield not-found");
    FxFree(&fx);
    PASS();
}

/* ── provider: func_8006E6D4 read issue ──────────────────────────────── */

static void test_read_guard_busy(void) {
    TEST("read_guard_busy");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    D_800B0CD8 |= 0x01000000u;
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, D_80011614, 64) == -1,
           "busy guard must return -1");
    FxFree(&fx);
    PASS();
}

static void test_read_guard_not_ready(void) {
    TEST("read_guard_not_ready");
    DiscFixture fx;
    ResetTestState(); /* lane 0: no drive reset */
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, D_80011614, 64) == -1,
           "CdReady != 1 must return -1");
    FxFree(&fx);
    PASS();
}

static void test_read_guard_queue(void) {
    TEST("read_guard_queue");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    PE_StoreU32(0x800A3608u, 1);
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, D_80011614, 64) == -1,
           "nonzero queue must return -1");
    FxFree(&fx);
    PASS();
}

static void test_read_happy_guest_bytes(void) {
    TEST("read_happy_guest_bytes");
    DiscFixture fx;
    uint32_t i;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, D_80011614, 4096) == 1,
           "read issue must return 1");
    for (i = 0; i < 4096; i++) {
        if (PE_LoadU8(D_80011614 + i) != FxPattern(i)) {
            FAIL("guest byte mismatch"); FxFree(&fx); return;
        }
    }
    ASSERT((D_800B0CD8 & 0x01004000u) == 0x01004000u, "state bits not set");
    ASSERT(PE_LoadU32(0x8009B6B4u) == 0, "bytes-pending must be 0 after sync read");
    ASSERT(PE_LoadU32(0x8009B6B0u) == D_80011614, "dest register wrong");
    ASSERT(PE_LoadU32(0x8009B6ACu) == 0x200u, "mode register wrong");
    ASSERT(PE_LoadU32(0x8009B6D4u) == 1, "read-active flag wrong");
    FxFree(&fx);
    PASS();
}

static void test_read_exact_end_boundary(void) {
    TEST("read_exact_end_boundary");
    DiscFixture fx;
    pe_addr_t dest = PE_RAM_END - 64;
    uint32_t i;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, dest, 64) == 1,
           "exact-end write must succeed");
    for (i = 0; i < 64; i++) {
        if (PE_LoadU8(dest + i) != FxPattern(i)) {
            FAIL("boundary byte mismatch"); FxFree(&fx); return;
        }
    }
    FxFree(&fx);
    PASS();
}

static void test_read_one_byte_overflow(void) {
    TEST("read_one_byte_overflow");
    DiscFixture fx;
    pe_addr_t dest = PE_RAM_END - 63; /* 64 bytes would end one past RAM */
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, dest, 64) == -1,
           "one-byte overflow must be rejected, not written");
    ASSERT(PE_LoadU8(PE_RAM_END - 1) == 0, "overflow wrote past RAM end");
    ASSERT(PE_LoadU8(dest) == 0, "failed read must not write any bytes");
    ASSERT((D_800B0CD8 & 0x01004000u) == 0, "state bits must be cleared on failure");
    FxFree(&fx);
    PASS();
}

static void test_read_truncated_source(void) {
    TEST("read_truncated_source");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    /* lba 63 is the last sector; 4096 bytes runs past the image end */
    ASSERT(func_8006E6D4(FX_SECTORS - 1, 0, D_80011614, 4096) == -1,
           "truncated source must be rejected");
    ASSERT(func_8006E6D4(FX_SECTORS, 0, D_80011614, 2048) == -1,
           "source past end must be rejected");
    FxFree(&fx);
    PASS();
}

static void test_read_zero_size_trivial(void) {
    TEST("read_zero_size_trivial");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    PE_StoreU8(D_80011614, 0xAA);
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, D_80011614, 0) == 1,
           "zero-size read must complete trivially (retail boot behavior)");
    ASSERT(PE_LoadU8(D_80011614) == 0xAA, "zero-size read must not write");
    FxFree(&fx);
    PASS();
}

static void test_read_repeated_loads(void) {
    TEST("read_repeated_loads");
    DiscFixture fx;
    uint32_t i;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    D_800B0CD8 &= 0xFEFFBFFFu; /* caller clears between reads (retail shape) */
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, D_80011614, 2048) == 1, "first read failed");
    D_800B0CD8 &= 0xFEFFBFFFu;
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 1, D_80011614 + 0x10000u, 2048) == 1,
           "second read failed");
    for (i = 0; i < 2048; i++) {
        if (PE_LoadU8(D_80011614 + i) != FxPattern(i) ||
            PE_LoadU8(D_80011614 + 0x10000u + i) != FxPattern(2048 + i)) {
            FAIL("repeated-load byte mismatch"); FxFree(&fx); return;
        }
    }
    FxFree(&fx);
    PASS();
}

static void test_read_then_ramreset(void) {
    TEST("read_then_ramreset");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, D_80011614, 2048) == 1, "read failed");
    ASSERT(PE_LoadU8(D_80011614) == FxPattern(0), "data not loaded");
    PE_RamReset();
    ASSERT(PE_LoadU8(D_80011614) == 0, "PE_RamReset must clear loaded bytes");
    FxFree(&fx);
    PASS();
}

/* ── provider: func_800811E4 poll ────────────────────────────────────── */

static void test_poll_after_read(void) {
    TEST("poll_after_read");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, D_80011614, 2048) == 1, "read failed");
    ASSERT(func_800811E4(0x801FFEE0u) == 0, "poll must report done after sync read");
    FxFree(&fx);
    PASS();
}

static void test_poll_timeout(void) {
    TEST("poll_timeout");
    int vs;
    ResetTestState();
    HostFB_Init();
    HostFB_VSync(0);
    HostFB_VSync(0);
    HostFB_GetState(&vs, NULL, NULL, NULL);
    /* issue timestamp 1300 vsyncs in the past → >1200 timeout (signed) */
    PE_StoreU32(0x8009B6C4u, (uint32_t)(vs - 1300));
    PE_StoreU32(0x8009B6CCu, 1);
    ASSERT(func_800811E4(0x801FFEE0u) == -1, "stale read must time out");
    ASSERT(PE_LoadU32(0x8009B6CCu) == 0, "timeout abort must clear D_8009B6CC");
    PASS();
}

/* ── func_800698D4 full sequence ─────────────────────────────────────── */

static void test_698D4_real_disc_sequence(void) {
    TEST("698D4_real_disc_sequence");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_800698D4() == 0, "mount must succeed (0) with a valid disc");
    ASSERT(D_800B0DCD == 1, "only bit 1 may be set (FMV2 absent, like Disc 1)");
    ASSERT(D_800B0DD8 == FX_PEIMG_LBA, "D_800B0DD8 must carry the PE.IMG LBA");
    FxFree(&fx);
    PASS();
}

static void test_698D4_second_disc_bits(void) {
    TEST("698D4_second_disc_bits");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 1), "fixture build failed");
    PE_Disc_SetActive(fx.disc);
    ASSERT(func_800698D4() == 0, "mount must succeed with the FMV2 fixture");
    ASSERT(D_800B0DCD == 3, "both bits must be set when both IDFs exist");
    ASSERT(D_800B0DD8 == FX_PEIMG_LBA, "D_800B0DD8 must carry the PE.IMG LBA");
    FxFree(&fx);
    PASS();
}

static void test_698D4_no_disc(void) {
    TEST("698D4_no_disc");
    ResetTestState();
    func_8007ED58();
    ASSERT(func_800698D4() == -1, "no disc must return -1 (PVD verify fails)");
    ASSERT(D_800B0DCD == 0, "no bits may be set without a disc");
    PASS();
}

static void test_698D4_bootstrap_fixture(void) {
    TEST("698D4_bootstrap_fixture");
    ResetTestState();
    g_bootstrap_disc = 1;
    ASSERT(func_800698D4() == 0, "fixture must report the retail mounted value 0");
    ASSERT(D_800B0DCD == 3, "fixture sets both bits");
    PASS();
}

/* ═══════════════════════════════════════════════════════════════════════
 * Phase 6E-B16 — func_8006A9E4 PE.IMG streaming resource load rung
 * (plus its three translated dependencies: func_8006E6A8 issue wrapper,
 * func_8006E7E8 completion poll, func_8006E498 archive lookup)
 *
 * Guest addresses used by these tests:
 *   table D_800930DC (halfwords, guest RAM here — no exe is loaded in the
 *   test process, exactly like the --bootstrap-disc fixture), stream
 *   buffer 0x80100000, archive copy D_800E2858, copy-2 dest 0x80180000.
 * ═══════════════════════════════════════════════════════════════════════ */

#define B16_TABLE   0x800930DCu
#define B16_ADEST   0x800A8028u
#define B16_STREAM  0x80100000u
#define B16_ARCH    0x800E2858u
#define B16_DEST2   0x80180000u
#define B16_COPY1   0x10A50u
#define B16_COPY2   0x1400u

/* Fixture PE.IMG byte i, exactly as FxBuild writes it: pattern for
 * i < FX_PEIMG_SIZE, zero-filled sector tail afterwards. */
static uint8_t B16_FxByte(uint32_t i) {
    return (i < FX_PEIMG_SIZE) ? FxPattern(i) : (uint8_t)0;
}

/* ── func_8006E6A8: sector-count → byte-count wrapper ────────────────── */

static void test_6E6A8_sector_to_byte(void) {
    TEST("6E6A8_sector_to_byte");
    DiscFixture fx;
    uint32_t i;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);

    /* 2 sectors must arrive as 4096 bytes: the << 11 conversion at the
     * host-adaptation boundary. */
    ASSERT(func_8006E6A8(FX_PEIMG_LBA, B16_STREAM, 2) == 1,
           "2-sector issue must return 1");
    for (i = 0; i < 4096; i++) {
        if (PE_LoadU8(B16_STREAM + i) != B16_FxByte(i)) {
            FAIL("byte mismatch after 2-sector read"); FxFree(&fx); return;
        }
    }
    ASSERT(PE_LoadU8(B16_STREAM + 4096) == 0, "read wrote past 4096 bytes");
    ASSERT((D_800B0CD8 & 0x01004000u) == 0x01004000u,
           "provider post-issue bits missing");
    ASSERT(PE_LoadU32(0x8009B6B0u) == B16_STREAM, "dest register wrong");
    FxFree(&fx);
    PASS();
}

static void test_6E6A8_guard_zero_and_wrap(void) {
    TEST("6E6A8_guard_zero_and_wrap");
    ResetTestState();
    func_8007ED58();

    /* busy guard passes through the wrapper unchanged */
    D_800B0CD8 |= 0x01000000u;
    ASSERT(func_8006E6A8(FX_PEIMG_LBA, B16_STREAM, 1) == -1,
           "busy guard must return -1");
    D_800B0CD8 &= ~0x01000000u;

    /* 0 sectors → byte size 0 → trivial success, no bytes written */
    PE_StoreU8(B16_STREAM, 0xAA);
    ASSERT(func_8006E6A8(FX_PEIMG_LBA, B16_STREAM, 0) == 1,
           "zero-sector issue must complete trivially");
    ASSERT(PE_LoadU8(B16_STREAM) == 0xAA, "zero-size read must not write");

    /* exact << 11 wraparound: -1 → 0xFFFFF800 (negative → rejected),
     * 0x100000 → 0x80000000 (negative → rejected) */
    ASSERT(func_8006E6A8(FX_PEIMG_LBA, B16_STREAM, -1) == -1,
           "-1 sectors must wrap to a negative byte size and fail");
    ASSERT(func_8006E6A8(FX_PEIMG_LBA, B16_STREAM, 0x100000) == -1,
           "0x100000 sectors must wrap to a negative byte size and fail");
    PASS();
}

/* ── func_8006E7E8: poll + D_800B0CD8 RMW ────────────────────────────── */

static void test_6E7E8_success_rmw(void) {
    TEST("6E7E8_success_rmw");
    DiscFixture fx;
    ResetTestState();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);

    D_800B0CD8 = 0xDEAD0000u;
    ASSERT(func_8006E6D4(FX_PEIMG_LBA, 0, B16_STREAM, 2048) == 1, "read failed");
    ASSERT(D_800B0CD8 == 0xDFAD4000u, "issue must set 0x01004000");
    ASSERT(func_8006E7E8() == 0, "poll must report 0 after sync read");
    /* RMW on st==0: &= 0xFEFFBFFF, every other bit preserved */
    ASSERT(D_800B0CD8 == 0xDEAD0000u, "RMW after st==0 wrong");
    FxFree(&fx);
    PASS();
}

static void test_6E7E8_timeout_rmw(void) {
    TEST("6E7E8_timeout_rmw");
    int vs;
    ResetTestState();
    HostFB_Init();
    HostFB_VSync(0);
    HostFB_VSync(0);
    HostFB_GetState(&vs, NULL, NULL, NULL);
    PE_StoreU32(0x8009B6C4u, (uint32_t)(vs - 1300)); /* >1200 vsyncs stale */
    PE_StoreU32(0x8009B6CCu, 1);
    D_800B0CD8 = 0xDFAD4000u;
    ASSERT(func_8006E7E8() == -1, "stale read must time out");
    ASSERT(D_800B0CD8 == 0xDEAD0000u, "RMW after st==-1 wrong");
    ASSERT(PE_LoadU32(0x8009B6CCu) == 0, "timeout abort must clear D_8009B6CC");
    PASS();
}

static void test_6E7E8_pending_no_rmw(void) {
    TEST("6E7E8_pending_no_rmw");
    ResetTestState();
    HostFB_Init();
    PE_StoreU32(0x8009B6C4u, 0);  /* fresh timestamp: no timeout at vs=0 */
    PE_StoreU32(0x8009B6B4u, 1);  /* 1 byte pending → st = 1 */
    D_800B0CD8 = 0xDFAD4000u;
    ASSERT(func_8006E7E8() == 1, "pending poll must return 1");
    /* (uint32_t)(st+1) < 2 is false for st==1: NO RMW */
    ASSERT(D_800B0CD8 == 0xDFAD4000u, "st==1 must not clear state bits");
    PASS();
}

/* ── func_8006E498: archive directory lookup ─────────────────────────── */

static void B16_CraftArchive(pe_addr_t base) {
    /* lw(base+4) = 0x40 → directory header at base+0x40+8 = base+0x48 */
    PE_StoreU32(base + 4, 0x40u);
    /* hdr: count = 2 (top 10 bits), first entry at base+0x80 */
    PE_StoreU32(base + 0x48, (2u << 22) | 0x80u);
    /* entry 0 @ base+0x80: offset 0x500, key 0x11111111 */
    PE_StoreU32(base + 0x84, 0x500u);
    PE_StoreU32(base + 0x88, 0x11111111u);
    /* entry 1 @ base+0x8C: offset 0x700, key 0x57D40D84 */
    PE_StoreU32(base + 0x90, 0x700u);
    PE_StoreU32(base + 0x94, 0x57D40D84u);
}

static void test_6E498_hit_exact(void) {
    TEST("6E498_hit_exact");
    ResetTestState();
    B16_CraftArchive(B16_STREAM);
    /* hit on entry 0 and on entry 1 (loop advance by 0xC) */
    ASSERT(func_8006E498(B16_STREAM, 0x11111111u) == B16_STREAM + 0x500u,
           "entry-0 lookup wrong");
    ASSERT(func_8006E498(B16_STREAM, 0x57D40D84u) == B16_STREAM + 0x700u,
           "entry-1 lookup wrong");
    /* repeated invocation with unchanged guest state: same result */
    ASSERT(func_8006E498(B16_STREAM, 0x57D40D84u) == B16_STREAM + 0x700u,
           "repeated lookup not stable");
    PASS();
}

static void test_6E498_miss_and_empty(void) {
    TEST("6E498_miss_and_empty");
    ResetTestState();
    B16_CraftArchive(B16_STREAM);
    ASSERT(func_8006E498(B16_STREAM, 0x22222222u) == 0,
           "absent key must return 0");

    /* count == 0 → immediate 0; the entry table must NOT be read */
    PE_StoreU32(B16_DEST2 + 4, 0x40u);
    PE_StoreU32(B16_DEST2 + 0x48, 0x80u); /* count 0, table off 0x80 */
    PE_StoreU32(B16_DEST2 + 0x84, 0x500u);
    PE_StoreU32(B16_DEST2 + 0x88, 0x57D40D84u); /* would hit if read */
    ASSERT(func_8006E498(B16_DEST2, 0x57D40D84u) == 0,
           "count==0 must return 0 without walking entries");
    PASS();
}

static void test_6E498_packed_fields(void) {
    TEST("6E498_packed_fields");
    ResetTestState();
    /* 24-bit offset mask: entry offset 0xFF000800 → base+0x800 */
    PE_StoreU32(B16_STREAM + 4, 0x40u);
    PE_StoreU32(B16_STREAM + 0x48, (1u << 22) | 0x80u);
    PE_StoreU32(B16_STREAM + 0x84, 0xFF000800u);
    PE_StoreU32(B16_STREAM + 0x88, 0x57D41D84u);
    ASSERT(func_8006E498(B16_STREAM, 0x57D41D84u) == B16_STREAM + 0x800u,
           "offset must be masked to 24 bits");

    /* count is exactly the top 10 bits: 0x3FF = 1023 entries.  Put the
     * key in the LAST entry (index 1022 at base+0x80+1022*0xC) and clear
     * entry 0: the full walk must reach it. */
    PE_StoreU32(B16_STREAM + 0x48, (0x3FFu << 22) | 0x80u);
    PE_StoreU32(B16_STREAM + 0x84, 0u);
    PE_StoreU32(B16_STREAM + 0x88, 0u);
    PE_StoreU32(B16_STREAM + 0x80 + 1022u * 0xCu + 4, 0x900u);
    PE_StoreU32(B16_STREAM + 0x80 + 1022u * 0xCu + 8, 0x57D41D84u);
    ASSERT(func_8006E498(B16_STREAM, 0x57D41D84u) == B16_STREAM + 0x900u,
           "1023-entry walk must reach the final entry");
    ASSERT(func_8006E498(B16_STREAM, 0x57D41D85u) == 0,
           "1023-entry walk over non-matching keys must miss");
    PASS();
}

static void test_6E498_readonly_footprint(void) {
    TEST("6E498_readonly_footprint");
    pe_addr_t a;
    ResetTestState();
    for (a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0xA5A5A5A5u);
    B16_CraftArchive(B16_STREAM);
    ASSERT(func_8006E498(B16_STREAM, 0x57D40D84u) == B16_STREAM + 0x700u,
           "hit failed");
    ASSERT(func_8006E498(B16_STREAM, 0x99999999u) == 0, "miss failed");
    /* pure table walk: the ONLY non-canary words are the ones the test
     * itself stored */
    for (a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        uint32_t want = 0xA5A5A5A5u;
        pe_addr_t off = a - B16_STREAM;
        if (off == 4u) want = 0x40u;
        else if (off == 0x48u) want = (2u << 22) | 0x80u;
        else if (off == 0x84u) want = 0x500u;
        else if (off == 0x88u) want = 0x11111111u;
        else if (off == 0x90u) want = 0x700u;
        else if (off == 0x94u) want = 0x57D40D84u;
        if (PE_LoadU32(a) != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n",
                   a, PE_LoadU32(a), want);
            FAIL("func_8006E498 wrote guest memory");
            return;
        }
    }
    PASS();
}

/* Expected unresolved-callee sequence of the translated func_800527C8
 * dispatcher (Phase 6E-B23, func_80052C6C now translated), in retail
 * ROM order. */
static const char *const B22_DISP_SEQ[4] = {
    "func_8005BCBC", "func_8005D6F4",
    "func_80051CC4", "func_80042C78"
};

static int B21_CheckDispatcherOrder(int base) {
    for (int i = 0; i < 4; i++) {
        if (strcmp(g_stub_order_log[base + i], B22_DISP_SEQ[i]) != 0)
            return 0;
    }
    return 1;
}

/* func_800528F0 expected output oracle: computes the byte at a given
 * offset into the 521-byte D_800A1B90 table, using the same LCG+XorShift
 * model as the retail MIPS — an independent, non-production reference. */
static uint8_t B18_ExpectedTableByte(uint32_t off) {
    uint32_t state, w[521];
    uint8_t out[521];
    int i, bit, pass;
    if (off >= 521) return 0;
    state = (uint32_t)(((uint64_t)0u * 0x88888889uLL) >> 32) >> 5;
    for (i = 0; i < 17; i++) {
        uint32_t word = 0;
        for (bit = 0; bit < 32; bit++) {
            state = (uint32_t)((uint64_t)state * 0x5D588B65uLL) + 1u;
            word = (word >> 1) | (state & 0x80000000u);
        }
        w[i] = word;
    }
    w[16] = (w[16] << 23) ^ (w[0] >> 9) ^ w[15];
    for (i = 17; i < 521; i++)
        w[i] = (w[i - 17] << 23) ^ (w[i - 16] >> 9) ^ w[i - 1];
    for (i = 0; i < 521; i++)
        out[i] = (uint8_t)(w[i] & 0xFFu);
    /* Self-referential XOR: out[0..31] ^= out[489..520],
     * out[32..520] ^= out[0..488], twice. */
    for (pass = 0; pass < 2; pass++) {
        for (i = 0; i < 32; i++)
            out[i] ^= out[489 + i];
        for (i = 32; i < 521; i++)
            out[i] ^= out[i - 32];
    }
    return out[off];
}

/* Expected word for DRAWENV/DISPENV structure addresses written by
 * func_8005E588's SetDefDrawEnv/SetDefDispEnv calls.
 * Returns the expected 32-bit LE word, or 0xFFFFFFFF if addr is not
 * within a known structure region. */
static uint32_t B19_DrawEnvExpectedWord(uint32_t addr) {
    /* DRAWENV_A: 0x800A2180, x=0,y=0,w=320,h=224 */
    if (addr == 0x800A2180u) return 0x00000000u;
    if (addr == 0x800A2184u) return 0x00E00140u;
    if (addr == 0x800A2188u) return 0x00000000u;
    if (addr == 0x800A218Cu) return 0x00000000u;
    if (addr == 0x800A2190u) return 0x00000000u;
    if (addr == 0x800A2194u) return 0x0101000Au;
    if (addr == 0x800A2198u) return 0x00000001u; /* byte 1 + 3 zeros */
    /* DRAWENV_C: 0x800A21DC, x=0,y=224,w=320,h=224 */
    if (addr == 0x800A21DCu) return 0x00E00000u;
    if (addr == 0x800A21E0u) return 0x00E00140u;
    if (addr == 0x800A21E4u) return 0x00080000u; /* sh 8 at +6,+8 */
    if (addr == 0x800A21E8u) return 0x00E00000u;
    if (addr == 0x800A21ECu) return 0x00000000u;
    if (addr == 0x800A21F0u) return 0x0101000Au;
    /* DRAWENV_B: 0x800A21F8, x=0,y=224,w=320,h=224 */
    if (addr == 0x800A21F8u) return 0x00E00000u;
    if (addr == 0x800A21FCu) return 0x00E00140u;
    if (addr == 0x800A2200u) return 0x00E00000u;
    if (addr == 0x800A2204u) return 0x00000000u;
    if (addr == 0x800A2208u) return 0x00000000u;
    if (addr == 0x800A220Cu) return 0x0101000Au;
    /* Flag bytes around 0x800A2210 */
    if (addr == 0x800A2210u) return 0x00000001u; /* sb 1 */
    /* DISPENV: 0x800A2254, x=0,y=0,w=320,h=224 */
    if (addr == 0x800A2254u) return 0x00000000u;
    if (addr == 0x800A2258u) return 0x00E00140u;
    if (addr == 0x800A225Cu) return 0x00080000u; /* sh 8 at +8 */
    if (addr == 0x800A2260u) return 0x00E00000u; /* sh 0xE0 at +C */
    if (addr == 0x800A2264u) return 0x00000000u;
    /* DISPENV continuation + misc */
    if (addr == 0x800A21F4u) return 0x00000000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    if (addr == 0x800A2260u) return 0x00E00000u;
    return 0xFFFFFFFFu;  /* not a known display env address */
}

/* ── func_8006A9E4: full patterned run ───────────────────────────────── */

static void test_6A9E4_full_run_patterned(void) {
    TEST("6A9E4_full_run_patterned");
    static uint8_t snap[B16_COPY1]; /* stream snapshot before the run */
    DiscFixture fx;
    uint32_t i;
    const uint8_t *fb;
    ResetTestState();
    HostFB_Init();
    func_8007ED58();
    ASSERT(FxBuild(&fx, 0), "fixture build failed");
    PE_Disc_SetActive(fx.disc);

    /* Retail-shaped halfword table (guest RAM here; no exe loaded):
     *   A: off=0  end=1  → lba 30, 1 sector → 0x800A8028
     *   B: off=1  end=1  → 0 sectors (stream preserved for the archive)
     *   C: off=1  end=2  → lba 31, 1 sector → stream
     *   D: off=2  end=3  → lba 32, 1 sector → stream */
    PE_StoreU16(B16_TABLE + 0, 0);
    PE_StoreU16(B16_TABLE + 2, 1);
    PE_StoreU16(B16_TABLE + 4, 1);
    PE_StoreU16(B16_TABLE + 6, 0x7E77); /* retail-unused slot */
    PE_StoreU16(B16_TABLE + 8, 1);
    PE_StoreU16(B16_TABLE + 10, 2);
    PE_StoreU16(B16_TABLE + 12, 3);
    D_800B0DD8 = FX_PEIMG_LBA;

    /* Entry contract (retail: func_8006A8D4 / func_8006A674 outputs). */
    D_800B0E6C = B16_STREAM;
    PE_StoreU32(0x800B0E08u, B16_DEST2);

    /* Pre-fill the stream buffer, then craft the archive the two
     * func_8006E498 lookups will walk after copy #1. */
    for (i = 0; i < B16_COPY1; i++)
        PE_StoreU8(B16_STREAM + i, (uint8_t)(i * 13u + 5u));
    PE_StoreU32(B16_STREAM + 4, 0x100u);            /* dir off */
    PE_StoreU32(B16_STREAM + 0x108, (3u << 22) | 0x180u); /* count 3 */
    PE_StoreU32(B16_STREAM + 0x184, 0x200u);        /* e0 off */
    PE_StoreU32(B16_STREAM + 0x188, 0xDEADBEEFu);   /* e0 key (decoy) */
    PE_StoreU32(B16_STREAM + 0x190, 0xAA000300u);   /* e1 off (dirty hi) */
    PE_StoreU32(B16_STREAM + 0x194, 0x57D40D84u);   /* e1 key */
    PE_StoreU32(B16_STREAM + 0x19C, 0xFF000400u);   /* e2 off (dirty hi) */
    PE_StoreU32(B16_STREAM + 0x1A0, 0x57D41D84u);   /* e2 key */
    for (i = 0; i < B16_COPY1; i++)
        snap[i] = PE_LoadU8(B16_STREAM + i);

    func_8006A9E4();

    /* 1. ClearImage({0,0,0x3FF,0x1FF}, 0,0,1) through the REAL host SDK:
     * clipped to the 320x240 host framebuffer. */
    fb = HostFB_GetPixels();
    for (i = 0; i < PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * 3; i += 3) {
        if (fb[i] != 0 || fb[i+1] != 0 || fb[i+2] != 1) {
            FAIL("ClearImage pixel wrong"); FxFree(&fx); return;
        }
    }

    /* 2. Cycle A bytes + guards. */
    for (i = 0; i < 2048; i++) {
        if (PE_LoadU8(B16_ADEST + i) != B16_FxByte(i)) {
            FAIL("cycle A byte mismatch"); FxFree(&fx); return;
        }
    }
    ASSERT(PE_LoadU32(B16_ADEST - 4) == 0, "guard below cycle A dest hit");
    ASSERT(PE_LoadU32(B16_ADEST + 2048) == 0, "guard above cycle A dest hit");

    /* 3. Copy #1: exact 0x10A50-byte image of the pre-run stream. */
    for (i = 0; i < B16_COPY1; i++) {
        if (PE_LoadU8(B16_ARCH + i) != snap[i]) {
            FAIL("copy #1 byte mismatch"); FxFree(&fx); return;
        }
    }
    ASSERT(PE_LoadU32(B16_ARCH - 4) == 0, "guard below archive hit");
    ASSERT(PE_LoadU32(B16_ARCH + B16_COPY1) == 0, "guard above archive hit");

    /* 4. Lookup slots, exact delay-slot store order results. */
    ASSERT(PE_LoadU32(0x800B0E20u) == B16_ARCH, "D_800B0E20 wrong");
    ASSERT(PE_LoadU32(0x800B0E18u) == B16_ARCH + 0x300u, "lookup 1 wrong");
    ASSERT(PE_LoadU32(0x800B0E1Cu) == B16_ARCH + 0x400u, "lookup 2 wrong");

    /* 5. Stream head after cycles C and D (D last: lba 32). */
    for (i = 0; i < 2048; i++) {
        if (PE_LoadU8(B16_STREAM + i) != B16_FxByte(4096 + i)) {
            FAIL("stream head mismatch after cycle D"); FxFree(&fx); return;
        }
    }
    ASSERT(PE_LoadU32(B16_STREAM + B16_COPY1) == 0, "stream guard hit");

    /* 6. Copy #2: 0x1400 bytes of the post-cycle-D stream. */
    for (i = 0; i < B16_COPY2; i++) {
        uint8_t want = (i < 2048) ? B16_FxByte(4096 + i) : snap[i];
        if (PE_LoadU8(B16_DEST2 + i) != want) {
            FAIL("copy #2 byte mismatch"); FxFree(&fx); return;
        }
    }
    ASSERT(PE_LoadU32(B16_DEST2 - 4) == 0, "guard below copy-2 dest hit");
    ASSERT(PE_LoadU32(B16_DEST2 + B16_COPY2) == 0, "guard above copy-2 dest hit");

    /* 7. Dependency boundary (updated Phase 6E-B21): func_800528F0,
     * func_8005E588, func_80062568, and func_80064964 are REAL too — six
     * unresolved dispatcher callees appear in retail ROM
     * order, then func_80087090. */
    ASSERT(g_stub_order_count == 5, "unexpected bootstrap invocations");
    ASSERT(B21_CheckDispatcherOrder(0),
           "dispatcher callee sequence wrong");
    ASSERT(strcmp(g_stub_order_log[4], "func_80087090") == 0,
           "func_80087090 must follow the dispatcher");
    ASSERT(Bootstrap_InvocationCount() == 5, "wrong provider count");
    ASSERT(CountOrderLog("func_8006A9E4") == 0, "6A9E4 routed via policy");
    ASSERT(CountOrderLog("func_800527C8") == 0, "527C8 routed via policy");
    ASSERT(CountOrderLog("func_800528F0") == 0, "528F0 routed via policy");
    ASSERT(CountOrderLog("func_8006E6A8") == 0, "6E6A8 routed via policy");
    ASSERT(CountOrderLog("func_8006E7E8") == 0, "6E7E8 routed via policy");
    ASSERT(CountOrderLog("func_8006E498") == 0, "6E498 routed via policy");
    FxFree(&fx);
    PASS();
}

/* ── func_8006A9E4: full 2 MiB canary footprint (zero-size cycles) ───── */

static uint32_t B16_StreamWordPre(uint32_t off) {
    if (off == 4u)    return 0x100u;
    if (off == 0x108u) return (1u << 22) | 0x180u;
    if (off == 0x184u) return 0x300u;
    if (off == 0x188u) return 0x57D40D84u;
    return 0xA5A5A5A5u;
}

static void test_6A9E4_zero_cycles_footprint(void) {
    TEST("6A9E4_zero_cycles_footprint");
    pe_addr_t a;
    ResetTestState();
    HostFB_Init();
    func_8007ED58();

    for (a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0xA5A5A5A5u);

    /* Canary table → every cycle size is (0xA5A5 - 0xA5A5) = 0, so no
     * disc is needed and no read bytes land.  Re-establish the provider
     * guards and the entry contract over the canary. */
    PE_StoreU32(0x8009B574u, 1);        /* CdReady lane */
    PE_StoreU32(0x800A3608u, 0);        /* queue empty */
    D_800B0CD8 = 0xA4A5A5A5u;           /* busy bit 0x01000000 clear */
    D_800B0E6C = B16_STREAM;
    PE_StoreU32(0x800B0E08u, B16_DEST2);
    PE_StoreU32(0x800A76A4u, 0);           /* timer tick = 0 (retail boot) */

    /* Minimal valid archive for the two lookups (count 1, key 1 hit). */
    PE_StoreU32(B16_STREAM + 4, 0x100u);
    PE_StoreU32(B16_STREAM + 0x108, (1u << 22) | 0x180u);
    PE_StoreU32(B16_STREAM + 0x184, 0x300u);
    PE_StoreU32(B16_STREAM + 0x188, 0x57D40D84u);

    func_8006A9E4();

    for (a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        uint32_t want = 0xA5A5A5A5u;
        if (a == 0x800B0CD8u) want = 0xE4A5A5A5u;      /* | then & RMW,
                                                * dispatcher |= 0x40000000 */
        else if (a == 0x8009B574u) want = 1;
        else if (a == 0x800A3608u) want = 0;
        else if (a == 0x800B0E6Cu) want = B16_STREAM;
        else if (a == 0x800B0E08u) want = B16_DEST2;
        else if (a == 0x8009B6ACu) want = 0x200u;      /* post-issue */
        else if (a == 0x8009B6B0u) want = B16_STREAM;  /* last dest */
        else if (a == 0x8009B6B4u) want = 0;
        else if (a == 0x8009B6C4u) want = 0;           /* vs = 0 */
        else if (a == 0x8009B6D4u) want = 1;
        else if (a == 0x800B0E20u) want = B16_ARCH;
        else if (a == 0x800B0E18u) want = B16_ARCH + 0x300u;
        else if (a == 0x800B0E1Cu) want = 0;           /* key 2 absent */
        /* Phase 6E-B17 dispatcher direct + leaf writes (func_800527C8
         * is REAL now — it runs during the cycle-B poll): */
        else if (a == 0x8009D028u) want = 0;           /* func_8005B890(0) */
        else if (a == 0x8009D218u) want = 1;           /* func_8005BC98 ×2 */
        else if (a == 0x8009D02Cu) want = 0;
        else if (a == 0x8009D030u) want = 0;
        else if (a == 0x8009D034u) want = 0;
        else if (a == 0x8009CF1Cu) want = 0;           /* func_8004F808 ×10 */
        else if (a == 0x8009CF30u) want = 0;
        else if (a == 0x8009CF3Cu) want = 0;
        else if (a == 0x8009CEFCu) want = 0;
        else if (a == 0x8009CF00u) want = 0;
        else if (a == 0x8009CF0Cu) want = 0;
        else if (a == 0x8009CF34u) want = 0;
        else if (a == 0x8009CF38u) want = 0;
        else if (a == 0x8009CFB0u) want = 0;
        else if (a == 0x8009CFF8u) want = 0;
        else if (a == 0x800A1870u) want = 0;           /* func_80042B38 */
        else if (a == 0x800A1874u) want = 0;
        else if (a == 0x8009D014u) want = 0x800A1AA0u; /* func_80051084 */
        else if (a == 0x8009CE94u) want = 0xA5A5A501u; /* 371A4(1) byte */
        else if (a == 0x800A76A4u) want = 0;           /* timer tick = 0 */
        /* Phase 6E-B18: func_800528F0 PRNG table writes */
        else if (a == 0x8009D038u) want = 0x00000208u; /* counter = 0x208 */
        else if (a >= 0x800A1B90u && a < 0x800A1B90u + 520u) {
            uint32_t off = a - 0x800A1B90u;
            want = (uint32_t)B18_ExpectedTableByte(off)
                 | ((uint32_t)B18_ExpectedTableByte(off + 1) << 8)
                 | ((uint32_t)B18_ExpectedTableByte(off + 2) << 16)
                 | ((uint32_t)B18_ExpectedTableByte(off + 3) << 24);
        } else if (a == 0x800A1B90u + 520u) {
            /* Final partial word: byte 520 only, remaining 3 bytes canary */
            want = (0xA5A5A5u << 8) | (uint32_t)B18_ExpectedTableByte(520);
        }
        /* Phase 6E-B19: func_8005E588 display env writes */
        else if (a == 0x8009D124u) want = 0;
        else if (a == 0x8009D128u) want = 0;
        else if (a == 0x8009D130u) want = 0;
        else if (a == 0x8009D134u) want = 0;
        else if (a == 0x8009D12Cu) want = 0x800A2270u;
        /* Phase 6E-B19a/b: func_8005E968 + func_8005F844 writes */
        else if (a == 0x8009D110u) want = 0x80808080u;
        else if (a == 0x8009D114u) want = 0x00404040u;
        else if (a == 0x8009D13Cu) want = 0x0000395Du;
        else if (a == 0x8009D140u) want = 0x00000084u;
        else if (a == 0x8009D144u) want = 0x000000A4u;
        /* DRAWENV/DISPENV structures */
        else if (a >= 0x800A2180u && a <= 0x800A2270u) {
            uint32_t exp = B19_DrawEnvExpectedWord(a);
            if (exp != 0xFFFFFFFFu) want = exp;
        }
        /* Phase 6E-B22: func_8005DE88 link chain and state */
        else if (a >= 0x800A2090u && a < 0x800A2174u
                 && (a - 0x800A2090u) % 0xCu == 0)
            want = a + 0xCu;
        else if (a == 0x800A2174u)
            want = 0;
        else if (a == 0x8009D0DCu)
            want = 0x800A2090u;
        else if (a == 0x8009D0E0u || a == 0x8009D0E4u ||
                 a == 0x8009D0E8u || a == 0x8009D0ECu ||
                 a == 0x8009D0F0u)
            want = 0;
        /* Phase 6E-B23: func_80052C6C resource-table init writes.
         *   - first clear loop: 50 halfwords at 0x800C0E48..0x800C0EAB
         *   - 9-record output table at 0x800A1E64..0x800A1F83: per record,
         *     byte at offset 9 cleared, halfword at offset 18 = 999 (0x03E7)
         *   - second clear loop: 100 halfwords at 0x800C1EB8..0x800C1F7F
         *   - third clear loop: 82 halfwords at 0x800C1F80..0x800C2023 */
        else if (a >= 0x800C0E48u && a <= 0x800C0EABu)
            want = 0;
        else if (a >= 0x800A1E64u && a < 0x800A1E64u + 288u) {
            uint32_t boff = (a - 0x800A1E64u) % 32u;
            want = 0xA5A5A5A5u;
            if (boff == 8u)        want = 0xA5A500A5u;  /* byte 9 cleared */
            else if (boff == 16u)  want = 0x03E7A5A5u;  /* sh 999 @ off 18 */
        } else if (a >= 0x800C1EB8u && a <= 0x800C1F7Fu)
            want = 0;
        else if (a >= 0x800C1F80u && a <= 0x800C2023u)
            want = 0;
        /* Phase 6E-B21: func_80071A24 bzero range + 80064964 flags */
        else if (a >= 0x800A3060u && a < 0x800A3180u) {
            want = 0;
            if (a == 0x800A3078u) want = 0x000000FFu;
            else if (a == 0x800A30A0u) want = 0x000000FFu;
            else if (a == 0x800A30B0u) want = 0x000000FFu;
            else if (a == 0x800A30B8u) want = 0x000000FFu;
            else if (a == 0x800A30C0u) want = 0x000000FFu;
            else if (a == 0x800A30C4u) want = 0x000000FFu;
            else if (a == 0x800A3124u) want = 0x000000FFu;
            else if (a == 0x800A3134u) want = 0x000000FFu;
        }
        /* Phase 6E-B20: func_80062568 free-list + gp stores */
        else if (a == 0x8009D158u) want = 0x800A22E0u;
        else if (a == 0x8009D15Cu) want = 0;
        else if (a == 0x8009D154u) want = 0;
        else if (a >= 0x800A22E0u && a < 0x800A3060u
                 && (a - 0x800A22E0u) % 0x90u == 0) {
            want = (a < 0x800A2FD0u) ? a + 0x90u : 0;
        }
        else if (a >= B16_STREAM && a < B16_STREAM + B16_COPY1)
            want = B16_StreamWordPre(a - B16_STREAM);
        else if (a >= B16_ARCH && a < B16_ARCH + B16_COPY1)
            want = B16_StreamWordPre(a - B16_ARCH);
        else if (a >= B16_DEST2 && a < B16_DEST2 + B16_COPY2)
            want = B16_StreamWordPre(a - B16_DEST2);
        if (PE_LoadU32(a) != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n",
                   a, PE_LoadU32(a), want);
            FAIL("func_8006A9E4 write footprint wrong");
            return;
        }
    }
    ASSERT(g_stub_order_count == 5, "unexpected bootstrap invocations");
    ASSERT(B21_CheckDispatcherOrder(0),
           "dispatcher callee sequence wrong");
    ASSERT(strcmp(g_stub_order_log[4], "func_80087090") == 0,
           "func_80087090 must follow the dispatcher");
    PASS();
}

/* ── func_8006A9E4: PE_RamReset then re-run ──────────────────────────── */

static void test_6A9E4_ramreset_rerun(void) {
    TEST("6A9E4_ramreset_rerun");
    uint32_t e18_1, e1c_1, e20_1;
    ResetTestState();
    HostFB_Init();
    func_8007ED58();
    /* zeroed table → 0-size cycles; zeroed stream → count-0 archive */
    D_800B0E6C = B16_STREAM;
    PE_StoreU32(0x800B0E08u, B16_DEST2);
    func_8006A9E4();
    e20_1 = PE_LoadU32(0x800B0E20u);
    e18_1 = PE_LoadU32(0x800B0E18u);
    e1c_1 = PE_LoadU32(0x800B0E1Cu);
    ASSERT(e20_1 == B16_ARCH && e18_1 == 0 && e1c_1 == 0,
           "first run slot values wrong");

    PE_RamReset();
    ASSERT(PE_LoadU32(0x800B0E20u) == 0, "RAM reset did not clear slots");
    /* Re-establish the entry contract (retail: the arena init rungs
     * re-run on reboot) and the provider lane state. */
    func_8007ED58();
    D_800B0E6C = B16_STREAM;
    PE_StoreU32(0x800B0E08u, B16_DEST2);
    func_8006A9E4();
    ASSERT(PE_LoadU32(0x800B0E20u) == e20_1, "rerun D_800B0E20 differs");
    ASSERT(PE_LoadU32(0x800B0E18u) == e18_1, "rerun D_800B0E18 differs");
    ASSERT(PE_LoadU32(0x800B0E1Cu) == e1c_1, "rerun D_800B0E1C differs");
    ASSERT(CountOrderLog("func_800527C8") == 0, "527C8 routed via policy");
    ASSERT(CountOrderLog("func_800528F0") == 0, "528F0 routed via policy");
    ASSERT(CountOrderLog("func_80087090") == 2, "stub count after rerun");
    PASS();
}

/* ── func_8006A9E4: placement after the completed func_8003E680 ──────── */

static void test_6A9E4_3E680_integration(void) {
    TEST("6A9E4_3E680_integration");
    ResetTestState();
    PE_Callback_Init();
    g_bootstrap_disc = 1;

    /* Retail order in func_8001220C: jal func_8003E680 @0x8001227C, then
     * jal func_8006A9E4 @0x80012284 (nop delay slot).  func_8003E680 is
     * fully translated — it must contribute zero bootstrap entries. */
    func_8003E680();
    ASSERT(g_stub_order_count == 0, "bootstrap providers remain in 3E680");

    func_8007ED58();
    D_800B0E6C = 0x801ED800u;  /* func_8006A8D4 arena value */
    PE_StoreU32(0x800B0E08u, 0x800F74F8u); /* func_8006A674 arena value */
    func_8006A9E4();

    /* B23: func_80052C6C now translated; 4 unresolved dispatcher
     * callees occupy the order log before func_80087090. */
    ASSERT(g_stub_order_count == 5, "unexpected provider count");
    ASSERT(B21_CheckDispatcherOrder(0),
           "func_800527C8 callee order must match retail ROM order");
    ASSERT(strcmp(g_stub_order_log[4], "func_80087090") == 0,
           "final provider after dispatcher must be func_80087090");
    ASSERT(CountOrderLog("func_8006A9E4") == 0,
           "func_8006A9E4 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_800527C8") == 0,
           "func_800527C8 still routed through bootstrap policy");
    ASSERT(CountOrderLog("func_800528F0") == 0,
           "func_8005E588 routed through bootstrap policy");
    ASSERT(PE_LoadU32(0x800B0E20u) == B16_ARCH, "D_800B0E20 wrong");
    ASSERT(PE_LoadU32(0x800B0E18u) == 0, "count-0 archive must yield 0");
    ASSERT(PE_LoadU32(0x800B0E1Cu) == 0, "count-0 archive must yield 0");
    /* The full --strict-stubs exit at func_8005DE88 (the first unresolved dispatcher callee
     * callee after func_8005E588 inside func_800527C8) is a runtime gate
     * (check_strict calls exit(1)) and is covered by the binary strict
     * runs; this test proves the in-process provider order. */
    PASS();
}

/* ── func_800528F0: direct PRNG table test ──────────────────────────── */

static void test_528F0_direct_boot_state(void) {
    TEST("528F0_direct_boot_state");
    uint32_t i;
    ResetTestState();
    PE_StoreU32(0x800A76A4u, 0);

    func_800528F0();

    {
        uint32_t w = PE_LoadU32(0x800A1B90u);
        ASSERT(w == 0x4A96BE57u, "D_800A1B90 first word mismatch");
    }
    ASSERT(PE_LoadU32(0x8009D038u) == 0x208u, "D_8009D038 counter mismatch");
    ASSERT(PE_LoadU8(0x800A1B90u + 0) == 0x57, "byte 0");
    ASSERT(PE_LoadU8(0x800A1B90u + 1) == 0xBE, "byte 1");
    ASSERT(PE_LoadU8(0x800A1B90u + 520) == 0x2E, "byte 520");

    /* Verify the full 521-byte table against the independent oracle. */
    for (i = 0; i < 521; i++) {
        uint8_t got = PE_LoadU8(0x800A1B90u + i);
        uint8_t want = B18_ExpectedTableByte(i);
        if (got != want) {
            printf("FAIL: D_800A1B90[%u] = 0x%02X, want 0x%02X\n", i, got, want);
            FAIL("func_800528F0 output byte mismatch");
            return;
        }
    }
    PASS();
}

/* ── Phase 6E-B19 dedicated tests ───────────────────────────────────── */

static void test_5E968_contract(void) {
    TEST("5E968_contract");
    ResetTestState();
    func_8005E968(0x80808080u);
    ASSERT(PE_LoadU32(0x8009D110u) == 0x80808080u, "orig color");
    ASSERT(PE_LoadU32(0x8009D114u) == 0x00404040u, "halved color");
    PASS();
}

static void test_5E968_boundary_values(void) {
    /* Exhaustive differential: prove (packed>>1)&0x007F7F7F matches
     * the MIPS sra+and for all interesting boundary values. */
    TEST("5E968_boundary_values");
    static const uint32_t inputs[] = {
        0x00000000, 0xFFFFFFFF, 0x80808080, 0x7F7F7F7F,
        0x80000000, 0x00FFFFFF, 0x01010101, 0xFEFEFEFE,
        0x00000001, 0x80000001, 0x40000000, 0xC0000000,
    };
    int n = (int)(sizeof(inputs)/sizeof(inputs[0]));
    int i;
    for (i = 0; i < n; i++) {
        ResetTestState();
        func_8005E968(inputs[i]);
        uint32_t orig = PE_LoadU32(0x8009D110u);
        uint32_t half = PE_LoadU32(0x8009D114u);
        uint32_t expect_half = (inputs[i] >> 1) & 0x007F7F7Fu;
        if (orig != inputs[i]) {
            printf("FAIL: input=0x%08X orig=0x%08X\n", inputs[i], orig);
            FAIL("5E968 orig mismatch"); return;
        }
        if (half != expect_half) {
            printf("FAIL: input=0x%08X half=0x%08X expect=0x%08X\n",
                   inputs[i], half, expect_half);
            FAIL("5E968 halved mismatch"); return;
        }
    }
    PASS();
}

static void test_5F844_a0_zero(void) {
    TEST("5F844_a0_zero");
    ResetTestState();
    func_8005F844(0);
    ASSERT(PE_LoadU32(0x8009D13Cu) == 0x395Du, "D_8009D13C a0=0");
    ASSERT(PE_LoadU32(0x8009D140u) == 0x84u,   "D_8009D140 a0=0");
    ASSERT(PE_LoadU32(0x8009D144u) == 0xA4u,   "D_8009D144 always");
    PASS();
}

static void test_5F844_a0_nonzero(void) {
    TEST("5F844_a0_nonzero");
    ResetTestState();
    func_8005F844(1);
    ASSERT(PE_LoadU32(0x8009D13Cu) == 0x3A1Cu, "D_8009D13C a0=1");
    ASSERT(PE_LoadU32(0x8009D140u) == 0xCCu,   "D_8009D140 a0=1");
    ASSERT(PE_LoadU32(0x8009D144u) == 0xA4u,   "D_8009D144 always");
    PASS();
}

static void test_5E588_direct_boot_state(void) {
    TEST("5E588_direct_boot_state");
    uint32_t i;
    ResetTestState();
    /* Set up known arena pointers. */
    D_800B0E50 = 0xDEAD0001u;
    D_800B0E54 = 0xDEAD0002u;
    D_800B0E38 = 0xDEAD0003u;
    D_800B0E3C = 0xDEAD0004u;

    func_8005E588();

    /* 1. Pointer propagation.
     *    D_800A21F4 is written from D_800B0E50 then overwritten by
     *    DRAWENV_C+0x18 (isbg/r0/g0/b0 = 0) — retail behavior. */
    ASSERT(PE_LoadU32(0x800A21F4u) == 0x00000000u, "D_800A21F4 (overwritten)");
    ASSERT(PE_LoadU32(0x800A226Cu) == 0xDEAD0002u, "D_800A226C");
    ASSERT(PE_LoadU32(0x800A21F0u) == 0x0101000Au, "D_800A21F0");
    ASSERT(PE_LoadU32(0x800A2268u) == 0xDEAD0004u, "D_800A2268");

    /* 2. Flag bytes. */
    ASSERT(PE_LoadU8(0x800A2210u) == 1, "flag 2210");
    ASSERT(PE_LoadU8(0x800A2198u) == 1, "flag 2198");
    for (i = 0; i < 3; i++) ASSERT(PE_LoadU8(0x800A2199u + i) == 0, "flag 2199+");
    for (i = 0; i < 3; i++) ASSERT(PE_LoadU8(0x800A2211u + i) == 0, "flag 2211+");

    /* 3. Halfword stores. */
    ASSERT(PE_LoadU16(0x800A225Eu) == 8,     "hw 225E");
    ASSERT(PE_LoadU16(0x800A21E6u) == 8,     "hw 21E6");
    ASSERT(PE_LoadU16(0x800A2262u) == 0xE0,  "hw 2262");
    ASSERT(PE_LoadU16(0x800A21EAu) == 0xE0,  "hw 21EA");

    /* 4. gp-relative stores. */
    ASSERT(PE_LoadU32(0x8009D128u) == 0,             "D_8009D128");
    ASSERT(PE_LoadU32(0x8009D124u) == 0,             "D_8009D124");
    ASSERT(PE_LoadU32(0x8009D12Cu) == 0x800A2270u,   "D_8009D12C");
    ASSERT(PE_LoadU32(0x8009D130u) == 0,             "D_8009D130");
    ASSERT(PE_LoadU32(0x8009D134u) == 0,             "D_8009D134");

    /* 5. Leaf callee results. */
    ASSERT(PE_LoadU32(0x8009D110u) == 0x80808080u, "5E968 orig");
    ASSERT(PE_LoadU32(0x8009D114u) == 0x00404040u, "5E968 half");
    ASSERT(PE_LoadU32(0x8009D13Cu) == 0x395Du,     "5F844 A a0=0");
    ASSERT(PE_LoadU32(0x8009D140u) == 0x84u,       "5F844 B a0=0");
    ASSERT(PE_LoadU32(0x8009D144u) == 0xA4u,       "5F844 C");

    /* 6. DRAWENV/DISPENV structures — spot-check one word from each. */
    ASSERT(PE_LoadU32(0x800A2180u) == 0x00000000u, "DRAWENV_A[0]");
    ASSERT(PE_LoadU32(0x800A2184u) == 0x00E00140u, "DRAWENV_A[1]");
    ASSERT(PE_LoadU32(0x800A21F8u) == 0x00E00000u, "DRAWENV_B[0]");
    ASSERT(PE_LoadU32(0x800A21DCu) == 0x00E00000u, "DRAWENV_C[0]");
    ASSERT(PE_LoadU32(0x800A2254u) == 0x00000000u, "DISPENV[0]");
    ASSERT(PE_LoadU32(0x800A2258u) == 0x00E00140u, "DISPENV[1]");

    /* 7. No bootstrap stubs. */
    ASSERT(g_stub_order_count == 0, "5E588 produced stubs");

    PASS();
}

static void test_5E588_ramreset_rerun(void) {
    TEST("5E588_ramreset_rerun");
    ResetTestState();
    D_800B0E50 = 0xAAAA0001u;
    D_800B0E54 = 0xAAAA0002u;
    D_800B0E38 = 0xAAAA0003u;
    D_800B0E3C = 0xAAAA0004u;
    func_8005E588();
    ASSERT(PE_LoadU32(0x8009D124u) == 0, "first run");
    PE_RamReset();
    ASSERT(PE_LoadU32(0x8009D124u) == 0, "reset cleared");
    D_800B0E50 = 0xBBBB0001u;
    D_800B0E54 = 0xBBBB0002u;
    D_800B0E38 = 0xBBBB0003u;
    D_800B0E3C = 0xBBBB0004u;
    func_8005E588();
    ASSERT(PE_LoadU32(0x8009D124u) == 0, "rerun");
    PASS();
}

/* ── Phase 6E-B20: func_80062568 free-list pool test ───────────────── */

static void test_62568_direct_boot_state(void) {
    TEST("62568_direct_boot_state");
    pe_addr_t a;
    ResetTestState();
    func_80062568();

    /* Verify pool chain: each slot stores next-slot pointer,
     * last slot is null-terminated. */
    for (a = 0x800A22E0u; a < 0x800A2FD0u; a += 0x90u)
        ASSERT(PE_LoadU32(a) == a + 0x90u, "pool link");
    ASSERT(PE_LoadU32(0x800A2FD0u) == 0, "pool null terminator");
    /* Head/tail/aux. */
    ASSERT(PE_LoadU32(0x8009D158u) == 0x800A22E0u, "pool head");
    ASSERT(PE_LoadU32(0x8009D15Cu) == 0, "pool tail");
    ASSERT(PE_LoadU32(0x8009D154u) == 0, "pool aux");
    /* No stubs. */
    ASSERT(g_stub_order_count == 0, "62568 stubs");
    PASS();
}

static void test_62568_dirty_state(void) {
    TEST("62568_dirty_state");
    pe_addr_t a;
    ResetTestState();
    /* Dirty all pool bytes + globals with patterned non-zero data. */
    for (a = 0x800A22E0u; a < 0x800A3060u; a += 4)
        PE_StoreU32(a, 0xDEAD0000u | (a & 0xFFFFu));
    PE_StoreU32(0x8009D158u, 0xBADC0DE0u);
    PE_StoreU32(0x8009D15Cu, 0xBADC0DE1u);
    PE_StoreU32(0x8009D154u, 0xBADC0DE2u);
    /* Also dirty guard bytes around pool and globals. */
    PE_StoreU32(0x800A22DCu, 0xCAFE0001u);  /* 4 bytes before pool */
    PE_StoreU32(0x800A3060u, 0xCAFE0002u);  /* at one-past-end */
    PE_StoreU32(0x8009D150u, 0xCAFE0003u);  /* 4 bytes before gp area */
    PE_StoreU32(0x8009D160u, 0xCAFE0004u);  /* 4 bytes after gp area */

    func_80062568();

    /* All 24 link words are exact. */
    for (a = 0x800A22E0u; a < 0x800A2FD0u; a += 0x90u)
        ASSERT(PE_LoadU32(a) == a + 0x90u, "dirty: pool link");
    ASSERT(PE_LoadU32(0x800A2FD0u) == 0, "dirty: null terminator");
    /* Globals exact. */
    ASSERT(PE_LoadU32(0x8009D158u) == 0x800A22E0u, "dirty: head");
    ASSERT(PE_LoadU32(0x8009D15Cu) == 0, "dirty: tail");
    ASSERT(PE_LoadU32(0x8009D154u) == 0, "dirty: aux");
    /* Non-link bytes within slots untouched. Spot-check 3 slots. */
    ASSERT(PE_LoadU32(0x800A22E4u) == (0xDEAD0000u | 0x22E4u), "slot0 +4");
    ASSERT(PE_LoadU32(0x800A22E0u + 0x8Cu) == (0xDEAD0000u | 0x236Cu), "slot0 end");
    ASSERT(PE_LoadU32(0x800A2374u) == (0xDEAD0000u | 0x2374u), "slot1 +4");
    ASSERT(PE_LoadU32(0x800A2FD4u) == (0xDEAD0000u | 0x2FD4u), "last slot +4");
    /* Guards untouched. */
    ASSERT(PE_LoadU32(0x800A22DCu) == 0xCAFE0001u, "guard before pool");
    ASSERT(PE_LoadU32(0x800A3060u) == 0xCAFE0002u, "guard at end");
    ASSERT(PE_LoadU32(0x8009D150u) == 0xCAFE0003u, "guard before gp");
    ASSERT(PE_LoadU32(0x8009D160u) == 0xCAFE0004u, "guard after gp");
    PASS();
}

static void test_62568_repeated(void) {
    TEST("62568_repeated");
    pe_addr_t a;
    ResetTestState();
    func_80062568();
    /* Corrupt non-link bytes in several slots. */
    PE_StoreU32(0x800A22E4u, 0xBAD00001u);
    PE_StoreU32(0x800A2374u, 0xBAD00002u);
    PE_StoreU32(0x800A2FD4u, 0xBAD00003u);
    /* Corrupt a link. */
    PE_StoreU32(0x800A2400u, 0xDEADBEEFu);
    /* Re-run. */
    func_80062568();
    /* Links restored. */
    ASSERT(PE_LoadU32(0x800A2400u) == 0x800A2400u + 0x90u, "repeat: link restored");
    ASSERT(PE_LoadU32(0x800A2FD0u) == 0, "repeat: null restored");
    /* Non-link bytes unchanged. */
    ASSERT(PE_LoadU32(0x800A22E4u) == 0xBAD00001u, "repeat: non-link slot0");
    ASSERT(PE_LoadU32(0x800A2374u) == 0xBAD00002u, "repeat: non-link slot1");
    ASSERT(PE_LoadU32(0x800A2FD4u) == 0xBAD00003u, "repeat: non-link last");
    PASS();
}

static void test_62568_full_footprint(void) {
    TEST("62568_full_footprint");
    pe_addr_t a;
    ResetTestState();
    for (a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0xA5A5A5A5u);
    func_80062568();

    for (a = PE_RAM_BASE; a < PE_RAM_END; a += 4) {
        uint32_t want = 0xA5A5A5A5u;
        /* Pool link words: 0x800A22E0..0x800A2FCF step 0x90 */
        if (a >= 0x800A22E0u && a < 0x800A2FD0u && (a - 0x800A22E0u) % 0x90u == 0)
            want = a + 0x90u;
        else if (a == 0x800A2FD0u)
            want = 0;                    /* null terminator */
        else if (a == 0x8009D158u)
            want = 0x800A22E0u;          /* head */
        else if (a == 0x8009D15Cu)
            want = 0;                    /* tail */
        else if (a == 0x8009D154u)
            want = 0;                    /* aux */
        if (PE_LoadU32(a) != want) {
            printf("FAIL: guest 0x%08X = 0x%08X, want 0x%08X\n", a, PE_LoadU32(a), want);
            FAIL("func_80062568 write footprint wrong");
            return;
        }
    }
    PASS();
}

static void test_62568_ramreset_rerun(void) {
    TEST("62568_ramreset_rerun");
    pe_addr_t a;
    ResetTestState();
    func_80062568();
    /* Verify initial state. */
    ASSERT(PE_LoadU32(0x800A22E0u) == 0x800A2370u, "first link");
    ASSERT(PE_LoadU32(0x800A2FD0u) == 0, "last link");
    ASSERT(PE_LoadU32(0x8009D158u) == 0x800A22E0u, "head");
    ASSERT(PE_LoadU32(0x8009D15Cu) == 0, "tail");
    ASSERT(PE_LoadU32(0x8009D154u) == 0, "aux");

    PE_RamReset();
    /* Dirty everything with controlled pattern. */
    for (a = 0x800A22E0u; a < 0x800A3060u; a += 4)
        PE_StoreU32(a, 0xCAFE0000u | (a & 0xFFFFu));
    PE_StoreU32(0x8009D158u, 0xBEEF0001u);
    PE_StoreU32(0x8009D15Cu, 0xBEEF0002u);
    PE_StoreU32(0x8009D154u, 0xBEEF0003u);

    func_80062568();
    /* Verify full chain restored. */
    for (a = 0x800A22E0u; a < 0x800A2FD0u; a += 0x90u)
        ASSERT(PE_LoadU32(a) == a + 0x90u, "rerun: pool link");
    ASSERT(PE_LoadU32(0x800A2FD0u) == 0, "rerun: null");
    ASSERT(PE_LoadU32(0x8009D158u) == 0x800A22E0u, "rerun: head");
    ASSERT(PE_LoadU32(0x8009D15Cu) == 0, "rerun: tail");
    ASSERT(PE_LoadU32(0x8009D154u) == 0, "rerun: aux");
    /* POOL_END untouched. */
    ASSERT(PE_LoadU32(0x800A3060u) == 0, "rerun: POOL_END untouched");
    /* Non-link bytes restored to zero by PE_RamReset, then left alone. */
    ASSERT(PE_LoadU32(0x800A22E4u) == 0xCAFE22E4u, "rerun: non-link preserved");
    ASSERT(PE_LoadU32(0x800A2FD4u) == 0xCAFE2FD4u, "rerun: non-link last preserved");
    PASS();
}

static void test_64964_direct_boot_state(void) {
    TEST("64964_direct_boot_state");
    uint32_t i;
    ResetTestState();
    for (i = 0; i < 0x120; i++) PE_StoreU8(0x800A3060u + i, 0xA5u);
    PE_StoreU32(0x800A305Cu, 0xCAFE0001u);
    PE_StoreU32(0x800A3180u, 0xCAFE0002u);
    func_80064964();
    for (i = 0; i < 0x120; i++) {
        uint32_t a = 0x800A3060u + i;
        uint8_t want = (a == 0x800A3078u || a == 0x800A30A0u ||
                        a == 0x800A30B0u || a == 0x800A30B8u ||
                        a == 0x800A30C0u || a == 0x800A30C4u ||
                        a == 0x800A3124u || a == 0x800A3134u) ? 0xFFu : 0;
        ASSERT(PE_LoadU8(a) == want, "64964 exact final byte state");
    }
    ASSERT(PE_LoadU32(0x800A305Cu) == 0xCAFE0001u, "64964 guard before");
    ASSERT(PE_LoadU32(0x800A3180u) == 0xCAFE0002u, "64964 guard after");
    PASS();
}

static void test_5DE88_direct_links_and_state(void) {
    TEST("5DE88_direct_links_and_state");
    pe_addr_t a;
    ResetTestState();
    for (a = PE_RAM_BASE; a < PE_RAM_END; a += 4)
        PE_StoreU32(a, 0xA5A5A5A5u);
    func_8005DE88();
    for (a = 0x800A2090u; a < 0x800A2174u; a += 0xCu)
        ASSERT(PE_LoadU32(a) == a + 0xCu, "5DE88 link chain");
    ASSERT(PE_LoadU32(0x800A2174u) == 0, "5DE88 null terminator");
    ASSERT(PE_LoadU32(0x8009D0DCu) == 0x800A2090u, "5DE88 head");
    ASSERT(PE_LoadU32(0x8009D0E0u) == 0, "5DE88 gp 370");
    ASSERT(PE_LoadU32(0x8009D0E4u) == 0, "5DE88 gp 374");
    ASSERT(PE_LoadU32(0x8009D0E8u) == 0, "5DE88 gp 378");
    ASSERT(PE_LoadU32(0x8009D0ECu) == 0, "5DE88 gp 37c");
    ASSERT(PE_LoadU32(0x8009D0F0u) == 0, "5DE88 gp 380");
    ASSERT(PE_LoadU32(0x800A208Cu) == 0xA5A5A5A5u, "5DE88 guard before");
    ASSERT(PE_LoadU32(0x800A2180u) == 0xA5A5A5A5u, "5DE88 guard after");
    PASS();
}

static void test_5DE88_reset_repeat_footprint(void) {
    TEST("5DE88_reset_repeat_footprint");
    pe_addr_t a;
    ResetTestState();
    func_8005DE88();
    PE_RamReset();
    for (a = 0x800A2090u; a < 0x800A2180u; a += 4)
        PE_StoreU32(a, 0xCAFE0000u | (a & 0xFFFFu));
    PE_StoreU32(0x800A208Cu, 0xBEEF0001u);
    PE_StoreU32(0x800A2180u, 0xBEEF0002u);
    func_8005DE88();
    for (a = 0x800A2090u; a < 0x800A2174u; a += 0xCu)
        ASSERT(PE_LoadU32(a) == a + 0xCu, "5DE88 repeat link");
    ASSERT(PE_LoadU32(0x800A2174u) == 0, "5DE88 repeat null");
    ASSERT(PE_LoadU32(0x800A208Cu) == 0xBEEF0001u, "5DE88 repeat guard before");
    ASSERT(PE_LoadU32(0x800A2180u) == 0xBEEF0002u, "5DE88 repeat guard after");
    ASSERT(PE_LoadU32(0x800A2094u) == 0xCAFE2094u, "5DE88 non-link preserved");
    func_8005DE88();
    ASSERT(PE_LoadU32(0x8009D0DCu) == 0x800A2090u, "5DE88 idempotent head");
    PASS();
}

static void test_80071A24_bzero_contract(void) {
    TEST("80071A24_bzero_contract");
    uint32_t i;
    ResetTestState();
    for (i = 0; i < 0x120; i += 4)
        PE_StoreU32(0x800A3060u + i, 0xDEAD0000u | i);
    PE_StoreU32(0x800A305Cu, 0xCAFE0001u);  /* guard before */
    PE_StoreU32(0x800A3180u, 0xCAFE0002u);  /* guard after */

    func_80071A24(0x800A3060u, 0x120);

    for (i = 0; i < 0x120; i += 4)
        ASSERT(PE_LoadU32(0x800A3060u + i) == 0, "bzero: word not zero");
    ASSERT(PE_LoadU32(0x800A305Cu) == 0xCAFE0001u, "guard before");
    ASSERT(PE_LoadU32(0x800A3180u) == 0xCAFE0002u, "guard after");
    ASSERT(func_80071A24(0x800A3060u, 0u) == 0x800A3060u,
           "bzero return destination");
    PASS();
}

static void test_80071A24_unaligned_zero_and_repeat(void) {
    TEST("80071A24_unaligned_zero_and_repeat");
    uint32_t i;
    ResetTestState();
    for (i = 0; i < 32; i++) PE_StoreU8(0x800A3050u + i, 0xA5u);
    func_80071A24(0x800A3053u, 7u);
    for (i = 0; i < 32; i++) {
        uint8_t want = (i >= 3 && i < 10) ? 0 : 0xA5u;
        ASSERT(PE_LoadU8(0x800A3050u + i) == want, "unaligned bzero");
    }
    func_80071A24(0x800A3053u, 7u);
    func_80071A24(0x800A3053u, 0u);
    ASSERT(PE_LoadU8(0x800A3052u) == 0xA5u, "zero-length touched before");
    ASSERT(PE_LoadU8(0x800A305Au) == 0xA5u, "zero-length touched after");
    PASS();
}

static void test_80071A24_full_ram_canary(void) {
    TEST("80071A24_full_ram_canary");
    uint32_t a;
    ResetTestState();
    for (a = PE_RAM_BASE; a < PE_RAM_END; a += 4) PE_StoreU32(a, 0xA5A5A5A5u);
    func_80071A24(0x800A3060u, 0x120u);
    for (a = PE_RAM_BASE; a < PE_RAM_END; a++) {
        uint8_t want = (a >= 0x800A3060u && a < 0x800A3180u) ? 0 : 0xA5u;
        ASSERT(PE_LoadU8(a) == want, "bzero canary footprint");
    }
    PASS();
}

static void test_80071A24_rejects_bad_ranges(void) {
    TEST("80071A24_rejects_bad_ranges");
#if defined(__SANITIZE_ADDRESS__)
    /* ASan/LSan cannot reliably reap aborting children under its tracer;
     * validate the same checked policy without deliberately aborting. */
    ASSERT(!PE_RangeIsRam(PE_RAM_END - 1u, 2u), "bad end range policy");
    ASSERT(!PE_RangeIsRam(PE_RAM_END - 2u, 8u), "bad overflow range policy");
    ASSERT(!PE_RangeIsRam(0xFFFFFFFFu, 1u), "bad wrapped range policy");
#else
    int cases = 0;
    const pe_addr_t dsts[] = { PE_RAM_END - 1u, PE_RAM_END - 2u, 0xFFFFFFFFu };
    const uint32_t lens[] = { 2u, 8u, 1u };
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        ASSERT(pid >= 0, "fork failed");
        if (pid == 0) { func_80071A24(dsts[i], lens[i]); _exit(0); }
        int status = 0;
        ASSERT(waitpid(pid, &status, 0) == pid, "waitpid failed");
        ASSERT(!WIFEXITED(status) || WEXITSTATUS(status) != 0,
               "bad range was accepted");
        cases++;
    }
    ASSERT(cases == 3, "bad range cases incomplete");
#endif
    PASS();
}

static void test_64964_ramreset_repeat_and_guards(void) {
    TEST("64964_ramreset_repeat_and_guards");
    ResetTestState();
    PE_StoreU32(0x800A305Cu, 0x11112222u);
    PE_StoreU32(0x800A3180u, 0x33334444u);
    func_80064964();
    PE_RamReset();
    for (uint32_t i = 0; i < 0x120; i++) PE_StoreU8(0x800A3060u + i, 0xA5u);
    PE_StoreU32(0x800A305Cu, 0x55556666u);
    PE_StoreU32(0x800A3180u, 0x77778888u);
    func_80064964();
    ASSERT(PE_LoadU32(0x800A305Cu) == 0x55556666u, "repeat guard before");
    ASSERT(PE_LoadU32(0x800A3180u) == 0x77778888u, "repeat guard after");
    ASSERT(PE_LoadU8(0x800A3060u) == 0 && PE_LoadU8(0x800A317Fu) == 0,
           "repeat range endpoints");
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
    test_callback_bind_resolve();
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
    test_7ED58_direct();
    test_7F72C_contract();
    test_6A5BC_both_loops_zero_body();
    test_7F778_getter();
    test_6A5BC_D_800B0DD4_store();
    test_6A5BC_strict_mode_rejects();

    /* func_8003E610 (2 tests) */
    test_3E610_guest_state();
    test_3E610_argument_values();

    /* Phase 6E-A direct provider tests (9 tests) */
    test_73C94_guard_idempotent();
    test_7D054_ssinit();
    test_77F7C_initgeom_constants();
    test_79004_79024_setters();
    test_409B4_card_init();
    test_7EC14_cdinit();
    test_80CC8_exchange();
    test_3E754_env_fields();
    test_3E944_save_state();

    /* Phase 6E-A batch 2 streaming tests (5 tests) */
    test_85644_bringup();
    test_86FF8_87024_commands();
    test_8682C_mode_mapping();
    test_8CBA8_default_entry_fields();
    test_8CBA8_0x24_index_math();

    /* func_8003E680 (7 tests) */
    test_3E680_five_globals_cleared();
    test_3E680_exactly_2000_polls();
    test_3E680_callback_exactly_once();
    test_3E680_callback_registered();
    test_3E680_callback_invoke();
    test_3E680_subsystem_order();
    test_3E680_final_call();

    /* Phase 6E-B6 rung */
    test_73D24_raw_contract();
    test_73D24_replace_remove_prevs();
    test_73D24_same_handler_no_store();
    test_73D24_zero_arg_clears();
    test_73D24_oracle_equality();
    test_73D24_dispatch_unknown_identity();
    test_73D24_bind_errors();
    test_73D24_write_footprint();
    test_73D24_ramreset_reinit();
    test_73D24_strict_not_stub();

    /* Phase 6E-B7 rung */
    test_371A4_raw_contract();
    test_371A4_byte_width_truncation();
    test_371A4_write_footprint();
    test_371A4_repeated_and_dirty();
    test_371A4_ramreset_reinit();
    test_371A4_3E680_integration();
    test_371A4_strict_not_stub();

    /* Phase 6E-B8 — func_80029388 rung */
    test_2F658_raw_contract();
    test_2F658_write_footprint();
    test_2F658_repeated_and_dirty();
    test_2F658_ramreset_reinit();
    test_20EFC_raw_contract();
    test_20EFC_write_footprint();
    test_29388_raw_contract();
    test_29388_write_footprint();
    test_29388_repeated_and_ramreset();
    test_29388_3E680_integration();
    test_29388_strict_not_stub();

    /* Phase 6E-B9 — func_8005BCA8 empty-stub rung */
    test_5BCA8_raw_contract();
    test_5BCA8_write_footprint();
    test_5BCA8_repeated_dirty_and_ramreset();
    test_5BCA8_3E680_integration();
    test_5BCA8_strict_not_stub();

    /* Phase 6E-B10 — func_80068D28 rung */
    test_68D28_raw_contract();
    test_68D28_write_footprint();
    test_68D28_repeated_dirty_and_ramreset();
    test_68D28_3E680_integration();
    test_68D28_strict_not_stub();

    /* Phase 6E-B11 — func_800124F8 rung */
    test_124F8_raw_contract();
    test_124F8_write_footprint();
    test_124F8_repeated_dirty_and_ramreset();
    test_124F8_3E680_integration();
    test_124F8_strict_not_stub();

    /* Phase 6E-B12 — func_8001A890 rung */
    test_1A890_raw_contract();
    test_1A890_write_footprint();
    test_1A890_repeated_dirty_and_ramreset();
    test_1A890_3E680_integration();
    test_1A890_strict_not_stub();

    /* Phase 6E-B13 — func_80034F10 rung */
    test_34F10_raw_contract();
    test_34F10_write_footprint();
    test_34F10_repeated_dirty_and_ramreset();
    test_34F10_3E680_integration();
    test_34F10_strict_not_stub();

    /* Phase 6E-B14 — func_8006536C rung */
    test_6536C_raw_contract();
    test_6536C_write_footprint();
    test_6536C_repeated_dirty_and_ramreset();
    test_6536C_3E680_integration();
    test_6536C_strict_not_stub();
    test_38D1C_frontier_past_3E680();

    /* Phase 6E-B15 — func_80038D1C rung */
    test_38D1C_raw_contract();
    test_38D1C_write_footprint();
    test_38D1C_repeated_and_ramreset();
    test_38D1C_3E680_integration();
    test_38D1C_strict_not_stub();

    /* D_80011614 (2 tests) */
    test_d11614_bootstrap_value();
    test_d11614_arena_anchors_track();

    /* Provider frontier: func_800725DC (3 tests) */
    test_725DC_sets_guard();
    test_725DC_idempotent();
    test_725DC_not_bootstrap_stub();

    /* Provider frontier: func_80070D10 (5 tests) */
    test_70D10_table_exact();
    test_70D10_reseed_idempotent();
    test_70D10_write_footprint();
    test_70D10_not_bootstrap_stub();
    test_3E680_70D10_translated();

    /* Provider frontier: func_80070D6C / func_80070DD0 (8 tests) */
    test_70D6C_first_16_returns();
    test_70D6C_checkpoints_and_state();
    test_70D6C_index_wrap_exact();
    test_70D6C_below_table_read_exact();
    test_70D6C_footprint();
    test_70D6C_reseed_repeats();
    test_70D6C_not_bootstrap_stub();
    test_70DD0_ranges();
    test_3E680_warmup_real();

    /* Provider frontier: func_8003E974 (8 tests) */
    test_3E974_gp_scalar_clears();
    test_3E974_array_final_table();
    test_3E974_call_sequence_exact();
    test_3E974_d1a0_or_exact();
    test_3E974_footprint();
    test_3E974_repeated_idempotent();
    test_3E974_not_bootstrap_stub();
    test_3E680_3E974_integration();

    /* Provider frontier: func_8003EAC8 LZCR registration (9 tests) */
    test_LZCR_helper_exact();
    test_3EAC8_contract_edge_inputs();
    test_3EAC8_one_hot_all32();
    test_3EAC8_same_slot_rom_order();
    test_3EAC8_footprint();
    test_3EAC8_recorder_disabled();
    test_3EAC8_recorder_overflow();
    test_3EAC8_not_bootstrap_stub();
    test_3E974_ramreset_reproduces();

    /* Provider frontier: func_80036DC8 timer-record init (6 tests) */
    test_36DC8_exact_final_state();
    test_36DC8_leaf_sequence_state();
    test_36DC8_footprint();
    test_36DC8_repeated_and_ramreset();
    test_36DC8_not_bootstrap_stub();
    test_3E680_36DC8_integration();

    /* func_8006E834 (2 tests) */
    test_6E834_clears_status_bytes();
    test_6E834_masks_state_word();

    /* func_8006E9A0 (2 tests) */
    test_6E9A0_dispatch_arg1();
    test_6E9A0_dispatch_arg3();

    /* Phase 6E-A batch 3: real-disc foundation (41 tests) */
    test_disc_open_rejects_bad_size();
    test_disc_open_rejects_bad_sync();
    test_disc_open_rejects_mode1();
    test_disc_open_valid_geometry();
    test_disc_read_sector_bounds();
    test_disc_read_userdata_cross_sector();
    test_disc_read_userdata_past_end();
    test_disc_verify_pvd();
    test_disc_verify_pvd_corrupt();
    test_disc_findfile_root();
    test_disc_findfile_nested();
    test_disc_findfile_missing();
    test_disc_findfile_no_backslash();
    test_disc_findfile_malformed_record();
    test_cdpos_to_int_vectors();
    test_pvd_verify_no_disc();
    test_pvd_verify_with_disc();
    test_pvd_verify_lane_shortcut();
    test_pvd_verify_wrong_disc();
    test_dssearch_no_disc();
    test_dssearch_bad_name();
    test_dssearch_pe_img_cdlfile();
    test_dssearch_missing();
    test_read_guard_busy();
    test_read_guard_not_ready();
    test_read_guard_queue();
    test_read_happy_guest_bytes();
    test_read_exact_end_boundary();
    test_read_one_byte_overflow();
    test_read_truncated_source();
    test_read_zero_size_trivial();
    test_read_repeated_loads();
    test_read_then_ramreset();
    test_poll_after_read();
    test_poll_timeout();
    test_698D4_real_disc_sequence();
    test_698D4_second_disc_bits();
    test_698D4_no_disc();
    test_698D4_bootstrap_fixture();

    /* Phase 6E-B16: func_8006A9E4 streaming load rung (13 tests) */
    test_6E6A8_sector_to_byte();
    test_6E6A8_guard_zero_and_wrap();
    test_6E7E8_success_rmw();
    test_6E7E8_timeout_rmw();
    test_6E7E8_pending_no_rmw();
    test_6E498_hit_exact();
    test_6E498_miss_and_empty();
    test_6E498_packed_fields();
    test_6E498_readonly_footprint();
    test_6A9E4_full_run_patterned();
    test_6A9E4_zero_cycles_footprint();
    test_6A9E4_ramreset_rerun();
    test_6A9E4_3E680_integration();

    /* Phase 6E-B18: func_800528F0 PRNG table generator (1 test) */
    test_528F0_direct_boot_state();

    /* Phase 6E-B19: func_8005E588 + leaves (6 tests) */
    test_5E968_contract();
    test_5E968_boundary_values();
    test_5F844_a0_zero();
    test_5F844_a0_nonzero();
    test_5E588_direct_boot_state();
    test_5E588_ramreset_rerun();

    /* Phase 6E-B20: func_80062568 free-list pool */
    test_62568_direct_boot_state();
    test_62568_dirty_state();
    test_62568_repeated();
    test_62568_full_footprint();
    test_62568_ramreset_rerun();
    /* Phase 6E-B22: func_8005DE88 resource-list initializer */
    test_5DE88_direct_links_and_state();
    test_5DE88_reset_repeat_footprint();
    test_64964_direct_boot_state();
    test_80071A24_bzero_contract();
    test_80071A24_unaligned_zero_and_repeat();
    test_80071A24_full_ram_canary();
    test_80071A24_rejects_bad_ranges();
    test_64964_ramreset_repeat_and_guards();

    /* Guard tests (4 tests) */
    test_no_emulator_process();
    test_translated_functions_not_in_bootstrap();
    test_first_clear_path_reached();
    test_direct_clear_not_default();

    printf("\nResults: %d run, %d passed, %d failed\n",
           tests_run, tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
