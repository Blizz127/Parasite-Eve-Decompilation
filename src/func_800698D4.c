/*
 * PARKED — Phase 5FL: func_800698D4 (disc mount / file search, Boot Rung 1).
 *
 * Status: 140/141 words (search-#3 and search-#4 beqz delay-slot scheduling
 * residual).  Retail uses nop in the beqz $v1, .L80069ADC delay slots for
 * searches #3 (D_80011354) and #4 (second D_80011348), but era cc1 -O2 -G0
 * consistently steals addiu $v0,$zero,-1 into all four beqz delay slots.
 * Searches #1 and #2 match retail exactly (retail also steals addiu $v0,-1
 * there), producing 140 words vs retail's 141.
 *
 * Phase 5FK-style zero-code barriers (asm volatile("": "=r"(v1): "0"(v1))
 * and empty asm volatile("")) prevent the steal at #3/#4 but add scheduling-
 * boundary overhead; the resulting objects bloat to 144 words (+3 vs retail).
 * The empty barrier causes cc1 to leave the branch in .set reorder mode,
 * forcing an assembler-inserted nop + separate li instruction instead of the
 * noreorder beqz+li pair.  No variant in V0-V8 reaches 141/141.
 *
 * The residual is a pure instruction-scheduling decision (delay-slot fill
 * vs. nop), not register allocation or liveness.  The retail compiler
 * (Psy-Q ccpsx, same GCC 2.7.x family) made different choices at different
 * search sites despite identical C structure, suggesting context-sensitive
 * heuristics (branch distance, function-end proximity) that era cc1 does
 * not replicate.
 *
 * Proven sltiu fix: return (D_800B0DCD < 1) generates sltiu (not slt).
 *
 * ROM: asm/disc1/55430.s @ file 0x5A0D4, 141 words (0x234), frame 0x30.
 * Next function: func_80069B08 at 0x5A308.
 */

extern unsigned char D_800B0DCD;    /* mount-status flag: |= 1, |= 2 */
extern int D_800B0DD8;              /* func_80080C48's return (opaque word) */
extern char D_80011330[];           /* "\\FMV1\\PEDISC01.IDF;1" */
extern char D_80011348[];           /* "\\PE.IMG;1" */
extern char D_80011354[];           /* "\\FMV2\\PEDISC02.IDF;1" */
extern int  func_8007F72C(void);
extern int  func_8007F778(void);    /* C */
extern int  func_80082314(void);
extern int  func_80081414(void *fp, char *name);  /* DsSearchFile (SDK) */
extern int  func_80080C48(void *fp);
extern void func_80073A44(int a);   /* VSync (SDK) */

int func_800698D4(void) {
    char local[0x18];   /* CdlFILE-shaped: pos(4) + size(4) + name(16) */
    int s0, t, v1;

    D_800B0DCD = 0;
    s0 = func_8007F72C();
    if (s0 != 1) {
        return 1;
    }
    if (func_8007F778() != 0) {
        return 1;
    }
    t = func_80082314();
    if (t == 1) {
        return 1;
    }
    if (t != 4) {
        return -1;
    }
    while (!(func_8007F72C() == 1 && func_8007F778() == 0)) {
        func_80073A44(0);
    }
    v1 = func_80081414(local, D_80011330);
    if (v1 != 0) {
      if (v1 != -1) {
        while (!(func_8007F72C() == 1 && func_8007F778() == 0)) {
            func_80073A44(0);
        }
        v1 = func_80081414(local, D_80011348);
        if (v1 != 0) {
          if (v1 != -1) {
            D_800B0DD8 = func_80080C48(local);
            D_800B0DCD |= 1;
          }
        }
      }
    }
    while (!(func_8007F72C() == 1 && func_8007F778() == 0)) {
        func_80073A44(0);
    }
    v1 = func_80081414(local, D_80011354);
    if (v1 != 0) {
      if (v1 != -1) {
        while (!(func_8007F72C() == 1 && func_8007F778() == 0)) {
            func_80073A44(0);
        }
        v1 = func_80081414(local, D_80011348);
        if (v1 != 0) {
          if (v1 != -1) {
            D_800B0DD8 = func_80080C48(local);
            D_800B0DCD |= 2;
          }
        }
      }
    }
    return (D_800B0DCD < 1) ? -2 : 0;
}
