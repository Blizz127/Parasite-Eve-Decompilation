/* Phase 6D-S — PE_PORT compatibility layer.
 * All BOOTSTRAP_RET stubs now use centralized Bootstrap_* policy.
 * No (int)(uintptr_t) casts remain on the first-clear path. */
#ifndef PE_PORT_COMPAT_H
#define PE_PORT_COMPAT_H
#include "psx_compat.h"

/* ── Boot Rung globals (guest-address backed) ─────────────────────── */
/* D_800B0CD8..D_800B0CEB, D_800B0DD4, D_80094488/D_8009448C are guest-RAM
 * lvalue macros defined in psx_compat.h — no externs here. */
extern unsigned int D_8009D1A0, D_8009D250;

/* ── Arena pointers — guest-RAM lvalue macros (Phase 6E-B16) ────────
 * The 19-slot retail arena pointer table lives INSIDE the D_800B0CD8
 * struct region in guest RAM (0x800B0E24..0x800B0E6C); retail func_8006A8D4
 * stores to it with sw and retail readers (func_8006A9E4: lw 0x130(s3),
 * lw 0x194(s3)) load from guest RAM.  6D-S modeled these as host scalar
 * globals, splitting them from the guest struct — func_8006A9E4 then read
 * zeros.  They are now guest-RAM lvalue macros like D_800B0CD8. */
#define D_800B0E24  PE_GUEST_U32(0x800B0E24u)
#define D_800B0E28  PE_GUEST_U32(0x800B0E28u)
#define D_800B0E2C  PE_GUEST_U32(0x800B0E2Cu)
#define D_800B0E30  PE_GUEST_U32(0x800B0E30u)
#define D_800B0E34  PE_GUEST_U32(0x800B0E34u)
#define D_800B0E38  PE_GUEST_U32(0x800B0E38u)
#define D_800B0E3C  PE_GUEST_U32(0x800B0E3Cu)
#define D_800B0E40  PE_GUEST_U32(0x800B0E40u)
#define D_800B0E44  PE_GUEST_U32(0x800B0E44u)
#define D_800B0E48  PE_GUEST_U32(0x800B0E48u)
#define D_800B0E4C  PE_GUEST_U32(0x800B0E4Cu)
#define D_800B0E50  PE_GUEST_U32(0x800B0E50u)
#define D_800B0E54  PE_GUEST_U32(0x800B0E54u)
#define D_800B0E58  PE_GUEST_U32(0x800B0E58u)
#define D_800B0E5C  PE_GUEST_U32(0x800B0E5Cu)
#define D_800B0E60  PE_GUEST_U32(0x800B0E60u)
#define D_800B0E64  PE_GUEST_U32(0x800B0E64u)
#define D_800B0E68  PE_GUEST_U32(0x800B0E68u)
#define D_800B0E6C  PE_GUEST_U32(0x800B0E6Cu)

/* ── func_8003E610 callees ─────────────────────────────────────────── */
/* All ten callees are now REAL implementations (Phase 6E-A batch 1):
 * func_80073C94/func_80072714/func_80072724  — pe_libetc.c
 * func_80074924/749D8/74A44/74BB8            — pe_libgpu.c
 * func_8007D054                              — pe_libsnd.c
 * func_80077F7C/func_80079004/func_80079024  — pe_gte.c
 * func_800409B4                              — pe_libcard.c
 * func_800844E4/func_80082534                — pe_save.c
 * func_8007EC14/func_8007ED58/func_8007F72C/func_8007F778/
 * func_8007FBF0/func_80080CC8/func_8007F7A8  — pe_libcd.c
 * func_8003E754/func_8003E944                — game/boot/*.c          */
#include "pe_sdk.h"

/* ── func_8006A5BC callees ─────────────────────────────────────────── */
/* All eight callees are now real (Phase 6E-A batches 1-2):
 * func_80085644/func_80086FF8/func_80087024/func_8008682C — pe_stream.c
 * func_8007ED58/func_8007F72C/func_8007F7A8                     — pe_libcd.c */

/* ── func_8003E680 callees ─────────────────────────────────────────── */
/* func_80070D10/func_80070D6C are now REAL translations (Phase 6E-B1/B2):
 * game/boot/func_80070D10_port.c — lagged-Fibonacci RNG table init
 * game/boot/func_80070D6C_port.c — RNG advance (verbatim |= wrap)
 * game/boot/func_80070DD0_port.c — handwritten ranged-random wrapper
 * func_8003E974 is now a REAL translation (Phase 6E-B3):
 * game/boot/func_8003E974_port.c — bit-table init + 20 registrations;
 * func_8003EAC8 is now a REAL translation too (Phase 6E-B4):
 * game/boot/func_8003EAC8_port.c — GTE LZCS/LZCR-indexed table writer.
 * func_80036DC8 is now a REAL translation too (Phase 6E-B5):
 * game/boot/func_80036DC8_port.c — timer-record init dispatcher. */
static inline void func_8003E91C(void)    { Bootstrap_ReturnVoid("func_8003E91C", "func_8003E680"); }

/* func_80073D24 is now a REAL SDK implementation (Phase 6E-B6):
 * pe_libetc.c — VBlank callback slot-4 setter with previous-handler
 * return, guest-table backed (pe_callback.h).
 * func_800371A4 is now a REAL translation (Phase 6E-B7):
 * game/boot/func_800371A4_port.c — 3-word $gp-relative byte setter.
 * func_80029388 is now a REAL translation too (Phase 6E-B8):
 * game/boot/func_80029388_port.c — slot-table clear + default-record
 * init (with leaves func_8002F658 and func_80020EFC).
 * func_8005BCA8 is now a REAL translation too (Phase 6E-B9):
 * game/boot/func_8005BCA8_port.c — empty jr/nop stub (2 retail words).
 * func_80068D28 is now a REAL translation too (Phase 6E-B10):
 * game/boot/func_80068D28_port.c — double-buffered display-record
 * data initializer (63 retail words).
 * func_800124F8 is now a REAL translation too (Phase 6E-B11):
 * game/boot/func_800124F8_port.c — subsystem table/array/scalar clear
 * (31 retail words).
 * func_8001A890 is now a REAL translation too (Phase 6E-B12):
 * game/boot/func_8001A890_port.c — subsystem scalar/array clear
 * (34 retail words).
 * func_80034F10 is now a REAL translation too (Phase 6E-B13):
 * game/boot/func_80034F10_port.c — subsystem table clear +
 * D_800B0CD8 flag-bit clear (45 retail words).
 * func_8006536C is now a REAL translation too (Phase 6E-B14):
 * game/boot/func_8006536C_port.c — 28x3-word record-table clear +
 * index byte clear (19 retail words).
 * func_800653B8 is now a REAL translation too (PE-CH1):
 * game/boot/func_800653B8_port.c — 18-word mailbox append into
 * D_800A3180[count++] (12-byte record; no clamp at 28).
 * func_80017764 is now a REAL translation too (PE-CH1):
 * game/boot/func_80017764_port.c — opcode 0x1C send; jal 653B8
 * with extra 0; return 1.
 * func_80065400 is now a REAL translation too (PE-CH1):
 * game/boot/func_80065400_port.c — 117-word mailbox drain; jal
 * func_80012700.
 * func_80012700 is now a REAL translation too (PE-CH1):
 * game/boot/func_80012700_port.c — 29-word freelist pop + init.
 * func_800177AC is now a REAL translation too (PE-CH1):
 * game/boot/func_800177AC_port.c — 7-word opcode 0x1F poll;
 * copies current task+0x14 through *arg0; return 1; no ACK.
 * func_80019154 is now a REAL translation too (PE-CH1):
 * game/boot/func_80019154_port.c — 7-word opcode 0x94 mode
 * read; copies D_8009D28C through *arg0; return 1.
 * func_8002F7D8 is now a REAL translation too (PE-CH1):
 * game/boot/func_8002F7D8_port.c — 102-word opcode 0x6F slot
 * alloc; claims first free SlotRecord; jal 1A680 unresolved.
 * func_8002FA10 is now a REAL translation too (PE-CH1):
 * game/boot/func_8002FA10_port.c — 37-word opcode 0x70
 * formation write through *actor.
 * func_8002FAA4 is now a REAL translation too (PE-CH1):
 * game/boot/func_8002FAA4_port.c — 13-word opcode 0xB7
 * formation write through *actor (0x70 subset).
 * func_8002FF78 is now a REAL translation too (PE-CH1):
 * game/boot/func_8002FF78_port.c — 101-word opcode 0x5A
 * Aya tagged setter through *D_8009D254.
 * func_80030220 is now a REAL translation too (PE-CH1):
 * game/boot/func_80030220_port.c — 197-word opcode 0x5A
 * slot tagged setter via D_80010C90[tag-40].
 * func_8002FE78 is now a REAL translation too (PE-BTL46):
 * game/boot/func_8002FE78_port.c — 64-word opcode 0x59
 * Aya tagged reader through *(*D_8009D254).
 * func_8003010C is now a REAL translation too (PE-BTL46):
 * game/boot/func_8003010C_port.c — 69-word opcode 0x59
 * slot tagged reader via D_80010B28[tag-41].
 * func_800299CC_consume_cut is now a REAL translation too (PE-CH1):
 * game/boot/func_800299CC_port.c — 16-word named cut of the
 * battle tick consume edge; D_8009D28C 6→0 and sb gp+0x10C=6.
 * func_8002CF24_mode7_cut is now a REAL translation too (PE-BTL2):
 * game/boot/func_8002CF24_port.c — 2-word inlined D_8009D28C=7
 * store (addiu 7 / sw gp+0x51C).
 * func_800293F4_hp_cut is now a REAL translation too (PE-BTL2):
 * game/boot/func_800293F4_port.c — 21-word HP clamp/copy named cut
 * of func_800293F4 (record+0x0C/+0x0E/+0x1C). */

/* ── REAL translated Boot Rung functions ────────────────────────────── */
extern void func_8003E610(void);
extern void func_8003E680(void);
extern void func_8003E974(void);
extern void func_8003EAC8(int, int);
extern void func_80036DC8(void);
extern void func_80036DF8(void);
extern void func_80036E34(void);
extern void func_80036E58(void);
extern void func_800371A4(int);
extern void func_80029388(void);
extern void func_8002F658(void);
extern void func_80020EFC(void);
extern void func_80071A64(pe_addr_t str);
extern void func_8005BCA8(void);
extern void func_80068D28(void);
extern void func_800124F8(void);
extern void func_8001A890(void);
extern void func_80034F10(void);
extern void func_80034FC4(void);
extern void func_8006536C(void);
extern void func_800653B8(unsigned int payload, unsigned int dest_id,
                          unsigned int dest_type, unsigned int sender,
                          unsigned int extra);
extern int  func_80065954(unsigned int index, unsigned int enabled);
extern int  func_800659C8(unsigned int index, unsigned int value);
extern int  func_80066800(unsigned int index);
extern int  func_800661EC(int a0, int a1, unsigned int a2, unsigned int a3);
extern int  func_80017C54(pe_addr_t args);
extern int  func_80017764(pe_addr_t args);
extern void func_80065400(void);
extern int func_80068E24(void);
extern void func_8003F3C4(void);
extern pe_addr_t func_80012700(pe_addr_t entry, unsigned int a1);
extern int  func_80017BB4_btl1_cut(pe_addr_t args);
extern int  func_800177AC(pe_addr_t args);
extern int  func_80019154(pe_addr_t args);
extern void func_8002F7D8(pe_addr_t actor);
extern void func_8002FA10(pe_addr_t actor, unsigned int index,
                          unsigned int a2, unsigned int a3,
                          unsigned int b3, unsigned int half_c,
                          int b7c, int b7d, int b7e, int b7f,
                          unsigned int b_e, unsigned int b_f);
extern void func_8002FAA4(pe_addr_t actor, unsigned int index,
                          unsigned int a2, unsigned int a3,
                          unsigned int b3, unsigned int half_c);
extern void func_8002FF78(unsigned int tag, unsigned int value);
extern int  func_8002FE78(unsigned int tag);
extern int  func_8003010C(pe_addr_t actor, unsigned int tag);
extern void func_80030220(pe_addr_t actor, unsigned int tag,
                          unsigned int value);
extern void func_800299CC_consume_cut(void);
extern void func_800299CC_after_consume_cut(void);
extern int func_8005C498(void);
extern void func_800512AC_cmd10_cut(void);
extern void func_80062CB8(pe_addr_t obj);
extern pe_addr_t func_80062CC4(void);
extern void func_8004B90C(void);
extern void func_8004B70C(uint32_t a0, uint32_t a1, pe_addr_t a2);
extern int func_8004BB80(pe_addr_t obj, uint32_t a1);
extern void func_8005E30C(void);
extern void func_800299CC_mode_switch_cut(void);
extern void func_8002A7F8_join_cut(void);
extern void func_800299CC_damage_entry_cut(void);
extern void func_8002BC90_mode6_cut(void);
extern void func_8002A7F8_mode3_cut(void);
extern int func_8002AA98(void);
extern void func_8002B29C(void);
extern void func_8002B0E8(void);
extern void func_8002F300_mode2_cut(void);
extern void func_800292EC_victory_ready_cut(void);
extern void func_80027D14(pe_addr_t actor);
extern void func_80028E94(pe_addr_t actor);
extern int func_80021054(void);
extern void func_80021DE0(void);
extern void func_80022394(void);
extern int func_80024A3C(void);
extern void func_80024250(int index, pe_addr_t actor);
extern void func_80021F38(void);
extern int func_8002312C(pe_addr_t slot);
extern void func_80023008(void);
extern void func_800236E8(void);
extern void func_80028574(pe_addr_t actor);
extern void func_8002F970(pe_addr_t p);
extern int func_80053E6C(int id);
extern void func_8006A25C(void);
extern void func_80033A2C(void);
extern int func_80019D24(pe_addr_t args);
extern int func_80069594(void);
extern int func_8006F8EC(unsigned int index);
extern int func_800D4704(pe_addr_t slot);
extern void func_8002CF24_mode7_cut(void);
extern void func_800293F4_hp_cut(void);
extern void func_800209F0_cut(void);
extern void func_80030640_cut(void);
extern void func_800339A0_cut(unsigned int encounter);
extern void func_80029810_after_hp_cut(unsigned int encounter);
extern void func_80029810_remainder_cut(void);
extern void func_80029810_prologue_cut(void);
extern void func_80029810_cut(unsigned int encounter);
extern void func_8001A680_command_cut(pe_addr_t actor, unsigned int command);
extern void func_8001A4AC(pe_addr_t actor);
extern void func_8006C140_type0_clip_bind(pe_addr_t package);
extern int func_800144FC_state3B_cut(void);
extern void func_800144FC_state3A_d1a0_cut(void);
extern int func_800144FC_state3A_cut(pe_addr_t arg0);
extern int func_800144FC_state0_cut(void);
extern int func_800144FC_state37_cut(void);
extern int func_800144FC_state38_cut(void);
extern int func_800144FC_state39_cut(void);
extern void func_80042EDC(void);
extern void func_80042F20(void);
extern int func_8006914C(int a0);
extern void func_8006D60C_after_6d078_cut(void);
extern int func_8006D60C_state3F_cut(void);
extern int func_8006D60C_state2F_cut(void);
extern int func_8006D60C_state30_cut(void);
extern void func_80086464(pe_addr_t a0);
extern void func_80086C1C(int a0, int a1);
extern int func_80085084(pe_addr_t buffer);
extern void func_8006D60C_state0_a0eq1_cut(void);
extern void func_8006D60C_state2C_cut(void);
extern int func_8006D60C(int a0);
extern void func_8006D078_state0_cut(void);
extern int func_8006D078_state2A_cut(void);
extern int func_8006D078_state2B_cut(void);
extern int func_8006D078(void);
extern int func_80087198(void);
extern void func_8006CDA4_state0_a0eq1_cut(int a1);
extern int func_8006CDA4_state7_cut(pe_addr_t dest, int stack_len);
extern int func_8006CDA4_state8_cut(void);
extern int func_8006CDA4_state9_a0eq1_cut(pe_addr_t dest);
extern int func_8006CDA4_stateA_cut(void);
extern int func_8006CDA4(int a0, int a1, int a2, pe_addr_t a3, int stack_len,
                         int stack_flag);
extern int func_8006C4C4(int a0);
extern int func_8006C1CC(int a0);
extern int func_8006C5BC(void);
extern pe_addr_t func_8006C5BC_ee13_prefix_cut(void);
extern void func_8006C5BC_ee13_epilogue_cut(void);
extern int func_8003F074_6C4C4_6C5BC_cut(void);
extern int func_8003F074_poll_cut(void);
extern pe_addr_t func_8003F074_371b0_a0(void);
extern void func_8003F074_after_poll_cut(void);
extern void func_8001A918(void);
extern void func_800E0060(void);
extern void func_800371B0(pe_addr_t a0);
extern pe_addr_t func_80012574(pe_addr_t a0);
extern void func_8001266C(void);
extern void func_800125E0(void);
extern pe_addr_t func_80035038(pe_addr_t desc, pe_addr_t parent,
                               unsigned int a2);
extern void func_8003F074_pool_cut(void);
extern void func_800361F4(pe_addr_t actor);
extern void func_80035E04(pe_addr_t actor);
extern void func_80035C84(pe_addr_t actor);
extern void func_8003999C(pe_addr_t actor, pe_addr_t table, pe_addr_t codep);
extern void func_8007136C(pe_addr_t actor, pe_addr_t codep);
extern void func_800710A4(pe_addr_t actor, pe_addr_t codep);
extern void func_80078934(pe_addr_t matrix, pe_addr_t src, pe_addr_t dst);
extern void func_8002F76C(pe_addr_t actor);
extern void func_80035558_walk_cut(void);
extern void func_80017018(void);
extern int func_80017294(pe_addr_t args);
extern int func_800172BC(pe_addr_t args);
extern int func_800172E0(pe_addr_t args);
extern int func_800172FC(pe_addr_t args);
extern int func_800181CC(pe_addr_t args);
extern int func_80015DAC_default_cut(pe_addr_t args);
extern int func_80015DAC_key190_cut(pe_addr_t args);
extern int func_800173F4(pe_addr_t args);
extern int func_80017E20(pe_addr_t args);
extern uint32_t func_8003708C(uint32_t a, uint32_t b);
extern uint32_t func_800370A8(uint32_t a, uint32_t b);
extern int func_80012850(pe_addr_t args);
extern int func_8001731C(pe_addr_t args);
extern int func_80017588(pe_addr_t args);
extern int func_80017D7C(pe_addr_t args);
extern int func_80017D5C(pe_addr_t args);
extern int func_80016910_key2900_cut(pe_addr_t args);
extern int func_8001A374(pe_addr_t args);
extern int func_80018E84(pe_addr_t args);
extern int func_80018F54(pe_addr_t args);
extern int func_80066C7C(unsigned int a0);
extern void func_80066CE8(void);
extern void func_80065674(void);
extern void func_80067E1C(void);
extern int func_80067294(pe_addr_t rec);
extern void func_80067A78(void);
extern void func_80067B74(void);
extern void func_80067D18(void);
extern void func_80068CE0(void);
extern int32_t func_80077CF4(int32_t angle);
extern int32_t func_80077DC4(int32_t angle);
extern int func_80018EE0(pe_addr_t args);
extern int func_80017988(pe_addr_t args);
extern int func_80019618(pe_addr_t args);
extern int func_8001856C(pe_addr_t args);
extern int func_80018E58(pe_addr_t args);
extern int func_80019410(pe_addr_t args);
extern int func_80019638(pe_addr_t args);
extern int func_80019658(pe_addr_t args);
extern int func_80018BEC(pe_addr_t args);
extern int func_80019AC0(pe_addr_t args);
extern int func_80066B60(unsigned int a0);
extern int func_80018EB4(pe_addr_t args);
extern int func_8001A1F0(pe_addr_t args);
extern int func_800176FC(pe_addr_t args);
extern int func_80018954(pe_addr_t args);
extern int func_80018164(pe_addr_t args);
extern int func_80018A48(pe_addr_t args);
extern int func_8001897C(pe_addr_t args);
extern int func_80018004(pe_addr_t args);
extern int func_800131E8(pe_addr_t args);
extern int func_80018774(pe_addr_t args);
extern int func_80013C34(pe_addr_t args);
extern int func_80013514(pe_addr_t args);
extern int func_800143B0(pe_addr_t args);
extern int func_800187C0(pe_addr_t args);
extern int func_800184EC(pe_addr_t args);
extern int func_8002FAF8(pe_addr_t actor, unsigned int code);
extern int func_80014228(pe_addr_t args);
extern int func_80017410(pe_addr_t args);
extern int func_800177C8(pe_addr_t args);
extern int func_80037548(int needle);
extern void func_800375E0(int id, unsigned int mode, pe_addr_t list);
extern void func_80037870(void);
extern void func_8003EB04(void);
extern int func_80037864(void);
extern int func_80017DE4(pe_addr_t args);
extern int func_80017F88(pe_addr_t args);
extern int func_80017FB0(pe_addr_t args);
extern int func_80019484(pe_addr_t args);
extern int func_80017A50(pe_addr_t args);
extern int func_80066BD8(unsigned int a0, unsigned int a1, unsigned int a2,
                         unsigned int a3, unsigned int a4);
extern int func_80018F0C(pe_addr_t args);
extern int func_80019BE4(pe_addr_t args);
extern int func_80019748(pe_addr_t args);
extern int func_80019798(pe_addr_t args);
extern int func_800392EC(void);
extern int func_80019728(pe_addr_t args);
extern int func_80018080(pe_addr_t args);
extern int func_80017FF0(pe_addr_t args);
extern int func_800192B8(pe_addr_t args);
extern void func_8001D340(unsigned int a0);
extern void func_8001F4D4(pe_addr_t actor);
extern int32_t func_8001F814(pe_addr_t actor);
extern int32_t func_800305C8(pe_addr_t attacker, pe_addr_t target);
extern void func_8006DE80(int id, int a1, int x, int y, int z);
extern void func_8006DED4(pe_addr_t dest, int id, int a1, int x, int y, int z);
extern unsigned int func_80071A54(void);
extern int func_8006F6D4(unsigned int index, unsigned int mode,
                         unsigned int a2, pe_addr_t out0, pe_addr_t out1,
                         pe_addr_t out2);
extern int func_800D4698(pe_addr_t slot, unsigned int mode, unsigned int a2,
                         unsigned int a3, unsigned int extra0,
                         unsigned int extra1);
extern int func_8006F39C(unsigned int code, pe_addr_t userdata);
extern int func_800CE49C(pe_addr_t slot, unsigned int extra);
extern void func_800D4620(pe_addr_t slot);
extern int func_8001735C(pe_addr_t args);
extern void func_8001AA78(pe_addr_t actor);
extern int func_8001C614(pe_addr_t rec, int a1, int a2);
extern int func_80012C20(pe_addr_t args);
extern int func_80017D9C(pe_addr_t args);
extern int func_80017AE8(pe_addr_t args);
extern int func_80017EC4(pe_addr_t args);
extern int func_80017B34(pe_addr_t args);
extern int func_80017B74(pe_addr_t args);
extern int func_80014694(pe_addr_t args);
extern int func_8001CAB0(int a0, int a1, pe_addr_t poly, unsigned int n);
extern int func_80014DA0(pe_addr_t args);
extern int func_80012E7C(pe_addr_t args);
extern int32_t func_80079FB4(int32_t a0, int32_t a1);
extern int func_8001A15C(pe_addr_t args);
extern int func_8001784C(pe_addr_t args);
extern int func_800130B4(pe_addr_t args);
extern int func_80015240(pe_addr_t args);
extern void func_80039B74(pe_addr_t dest, pe_addr_t clip, int a2, int a3);
extern pe_addr_t func_800362B8(unsigned int size);
extern void func_8003A6A8(pe_addr_t dest, pe_addr_t unused);
extern void func_8003E188(pe_addr_t dest);
extern void func_8006E2D0(pe_addr_t dest, uint32_t token);
extern int func_8006E454(pe_addr_t name);
extern void func_8006B4F8_12574_publish_cut(void);
extern int func_8006B4F8_dest_load_cut(uint32_t token);
extern void func_8006B35C(void);
extern int func_8006BE4C(void);
extern int func_8006BECC(void);
extern int func_8003F074_dest_ready_cut(uint32_t token);
extern int func_8006CC68(void);
extern void func_800661A4(void);
extern void func_800661CC(void);
extern void func_8006C5BC_clear_wait_cut(void);
extern void func_8003D050_prefix_cut(pe_addr_t dest, pe_addr_t obj,
                                     pe_addr_t stream, unsigned int stack_ba);
extern void func_8003D050_ptr14_cut(pe_addr_t dest, pe_addr_t obj);
extern int func_8003D050_post_3d94c_skip_cut(pe_addr_t dest, pe_addr_t stream);
extern void func_8003D050_epilogue_cut(pe_addr_t dest, int skipped);
extern void func_8003C5D8(pe_addr_t dest, int a1);
extern int func_8003C818(pe_addr_t dest);
extern int func_8003AF14(pe_addr_t dest, pe_addr_t scratch);
extern void func_8006698C(pe_addr_t dest);
extern void func_8003DFD8(pe_addr_t src, pe_addr_t dst, int count);
extern void func_8003D834(pe_addr_t dest, pe_addr_t clip, int a2, pe_addr_t bea40);
extern void func_800794C4(pe_addr_t angles, pe_addr_t out);
extern void func_8003A088_mode0_empty_cut(pe_addr_t dest);
extern void func_8003A088_mode0_walk_cut(pe_addr_t dest);
extern void func_8003B97C_empty_cut(pe_addr_t dest, pe_addr_t bea40);
extern void func_8003B97C_lighting_cut(pe_addr_t dest, pe_addr_t bea40);
extern void func_8003BCE0(pe_addr_t dest, int a1, int a2);
extern void func_80070D10(void);
extern unsigned int func_80070D6C(void);
extern int func_80070DD0(int, int);

/* func_8003EAC8 deterministic argument recording (test instrumentation) */
void PE_3EAC8_RecordReset(void);
void PE_3EAC8_RecordSetEnabled(int enabled);
int  PE_3EAC8_RecordCount(void);
int  PE_3EAC8_RecordAt(int index, int *a0, int *a1);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A674(void);
extern void func_8006A8D4(void);

/* func_80077B64/B A4/BC4/C44/C64 — Phase 6E-PE-GPU1: REAL translated
 * GPU packet-header setters (SetPolyF3 / SetPolyFT4 / SetPolyG4 / SetTile /
 * SetSprt).  game/boot/func_80077B{64,BA4,BC4,C44,C64}_port.c — each a
 * 5-word real outlined header-inline writing the primitive packet's byte
 * offset 3 (length) and byte offset 7 (code).  Called via jal from
 * func_80030894; B54K-A now exercises the SET leaves via the L2/L3
 * sprite-array prefix. */
extern void func_80077B64(pe_addr_t p);
extern void func_80077BA4(pe_addr_t p);
extern void func_80077BC4(pe_addr_t p);
extern void func_80077C44(pe_addr_t p);
extern void func_80077C64(pe_addr_t p);

/* Phase 6E-B54I: GPU primitive-builder leaves of func_80030894 and the
 * two add/sort wrappers, all REAL translations with full word-decode
 * headers (game/boot/func_80077*_port.c, func_8005DADC_port.c,
 * func_80037{0DC,140}_port.c, platform/func_800719E4_port.c):
 *   func_80077A64  GetTPage(tp, abr, x, y) value builder (pure)
 *   func_80077AA4  CLUT value builder, (y<<6)|((x>>a4)&0x3F), &0xFFFF
 *   func_80077B04  SetSemiTrans — code byte bit 1 set/clear
 *   func_80077B34  SetShadeTex — code byte bit 0 set/clear
 *   func_80077C04  setSprt header — len 4, code 0x64
 *   func_80077C84  DR draw-mode word — p[3]=1; p+4 word; returns word
 *   func_80077CB4  length-budget append — cap 17, fail -1, tail.tag=0
 *   func_8005DADC  *(u32*)0x800A8030 + 0x800A8028 + (a0<<3)
 *   func_800370DC  add/sort wrapper (draw-mode + setSprt + append)
 *   func_80037140  add/sort wrapper (draw-mode + SetTile + append)
 *   func_800719E4  BIOS B(38h) CD-mode trampoline (fail path; collapsed
 *                  with recorded justification, pe_libcd.c precedent) */
extern uint32_t func_80077A64(uint32_t tp, uint32_t abr, uint32_t x,
                              uint32_t y);
extern uint32_t func_80077AA4(int32_t x, uint32_t y);
extern void     func_80077B04(pe_addr_t p, uint32_t abr);
extern void     func_80077B34(pe_addr_t p, uint32_t st);
extern void     func_80077C04(pe_addr_t p);
extern uint32_t func_80077C84(pe_addr_t p, uint32_t a1, uint32_t a2,
                              uint32_t a3);
extern int32_t  func_80077CB4(pe_addr_t head, pe_addr_t tail);
extern pe_addr_t func_8005DADC(uint32_t index);
extern void     func_800370DC(pe_addr_t p, uint32_t mode);
extern void     func_80037140(pe_addr_t p, uint32_t mode);
extern uint32_t func_800719E4(uint32_t mode);

#endif
