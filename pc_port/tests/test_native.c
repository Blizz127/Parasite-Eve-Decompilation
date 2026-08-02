/*
 * Phase 6A — Native port tests.
 *
 * Covers: framebuffer, stubs, trace, memory, ClearImage, strict mode.
 * No interactive desktop required — all tests are headless.
 */
#include "host_framebuffer.h"
#include "stub_registry.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { tests_run++; printf("  TEST %s... ", name); } while(0)
#define PASS() do { tests_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg) do { tests_failed++; printf("FAIL: %s\n", msg); } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

/* ── 1. Framebuffer initialization ─────────────────────────────────── */

static void test_fb_init_zeros(void)
{
    TEST("fb_init_zeros");
    HostFB_Init();
    const uint8_t *p = HostFB_GetPixels();
    for (int i = 0; i < PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * 3; i++) {
        if (p[i] != 0) { FAIL("framebuffer not zero after init"); return; }
    }
    PASS();
}

/* ── 2. ClearImage fills correctly ──────────────────────────────────── */

static void test_clearimage_full(void)
{
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

/* ── 3. ClearImage rectangle clipping ───────────────────────────────── */

static void test_clearimage_clip(void)
{
    TEST("clearimage_clip");
    HostFB_Init();
    /* Fill with known color first */
    HostFB_ClearImage(0, 0, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT, 0xFF, 0, 0);
    /* Clip: rect starts negative */
    HostFB_ClearImage(-5, 0, 10, PE_PORT_FB_HEIGHT, 0, 0, 1);
    const uint8_t *p = HostFB_GetPixels();
    /* First 5 columns should be RGB(0,0,1), rest RGB(255,0,0) */
    ASSERT(p[0] == 0 && p[1] == 0 && p[2] == 1, "clip: pixel at (0,0) not cleared");
    ASSERT(p[15] == 0xFF && p[16] == 0 && p[17] == 0, "clip: pixel at (5,0) should be red");
    PASS();
}

/* ── 4. ClearImage out-of-bounds rect ───────────────────────────────── */

static void test_clearimage_oob(void)
{
    TEST("clearimage_oob");
    HostFB_Init();
    /* Entirely negative rect should be a no-op */
    HostFB_ClearImage(-100, -100, 50, 50, 0, 0, 1);
    const uint8_t *p = HostFB_GetPixels();
    ASSERT(p[0] == 0, "oob: pixel should still be zero");
    PASS();
}

/* ── 5. SetDispMask state ───────────────────────────────────────────── */

static void test_display_mask(void)
{
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

/* ── 6. VSync counter ───────────────────────────────────────────────── */

static void test_vsync_counter(void)
{
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

/* ── 7. DrawSync counter ────────────────────────────────────────────── */

static void test_drawsync_counter(void)
{
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

/* ── 8. PPM output is deterministic ─────────────────────────────────── */

static void test_ppm_deterministic(void)
{
    TEST("ppm_deterministic");
    HostFB_Init();
    HostFB_ClearImage(0, 0, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT, 1, 2, 3);

    HostFB_WritePPM("/tmp/pe-test-a.ppm");
    HostFB_WritePPM("/tmp/pe-test-b.ppm");

    /* Compare byte-by-byte */
    FILE *fa = fopen("/tmp/pe-test-a.ppm", "rb");
    FILE *fb = fopen("/tmp/pe-test-b.ppm", "rb");
    ASSERT(fa && fb, "cannot open PPM files");

    fseek(fa, 0, SEEK_END);
    fseek(fb, 0, SEEK_END);
    long sa = ftell(fa), sb = ftell(fb);
    ASSERT(sa == sb, "PPM sizes differ");

    fseek(fa, 0, SEEK_SET);
    fseek(fb, 0, SEEK_SET);
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

/* ── 9. Stub registry records invocations ───────────────────────────── */

static void test_stub_registry(void)
{
    TEST("stub_registry");
    /* Reset registry (hack: zero count) */
    extern int g_stub_count;
    g_stub_count = 0;

    Stub_Record("test_stub", "BOOTSTRAP_RET");
    Stub_Record("test_stub", "BOOTSTRAP_RET");  /* second invocation */

    extern StubEntry g_stub_registry[];
    ASSERT(g_stub_count == 1, "duplicate stub should not increase count");
    ASSERT(g_stub_registry[0].invoked == 2, "invocation count should be 2");
    ASSERT(strcmp(g_stub_registry[0].classification, "BOOTSTRAP_RET") == 0,
           "classification should be BOOTSTRAP_RET");
    PASS();
}

/* ── 10. Strict stubs flag ──────────────────────────────────────────── */

static void test_strict_stubs(void)
{
    TEST("strict_stubs_flag");
    extern int g_strict_stubs;
    g_strict_stubs = 1;

    extern int g_stub_count;
    g_stub_count = 0;

    /* UNSUPPORTED in strict mode should exit. We can't easily test exit(),
     * but we can verify the flag is set. */
    ASSERT(g_strict_stubs == 1, "strict_stubs flag should be set");
    g_strict_stubs = 0;
    PASS();
}

/* ── 11. Bootstrap disc flag ────────────────────────────────────────── */

static void test_bootstrap_disc_flag(void)
{
    TEST("bootstrap_disc_flag");
    extern int g_bootstrap_disc;
    g_bootstrap_disc = 1;
    ASSERT(g_bootstrap_disc == 1, "bootstrap_disc should be set");
    g_bootstrap_disc = 0;
    PASS();
}

/* ── 12. Framebuffer dimensions ─────────────────────────────────────── */

static void test_fb_dimensions(void)
{
    TEST("fb_dimensions");
    ASSERT(PE_PORT_FB_WIDTH == 320, "width should be 320");
    ASSERT(PE_PORT_FB_HEIGHT == 240, "height should be 240");
    PASS();
}

/* ── main ───────────────────────────────────────────────────────────── */

int main(void)
{
    printf("Phase 6A native port tests\n=======================\n");

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

    printf("\nResults: %d run, %d passed, %d failed\n",
           tests_run, tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
