# Cleanup audit — approved Phase A

Generated at the approved cleanup point. Base: `main` @ `fc2b2f4c19860ff2b5388094240a1434a4fb5be7`.

## Archived stash 3

Stash 3 contained 22 untracked evidence files. They were copied under `docs/evidence/archive-overnight-17exx/` with their original relative paths, then stash 3 is dropped after this report is staged. No source file was duplicated because all 22 paths were absent from HEAD.

## Approved stash drops — pre-drop excerpts

### stash@{6}

Command: `git stash show -p stash@{6} | head -40`

```diff
diff --git a/configs/USA/disc1.yaml b/configs/USA/disc1.yaml
index c21c1ea..3eb987a 100644
--- a/configs/USA/disc1.yaml
+++ b/configs/USA/disc1.yaml
@@ -550,6 +550,10 @@ segments:
       # stores with byte-address cursor arithmetic. Era -O2 -G0; no $gp.
       - [0x5B0D4, c, func_8006A8D4]
       - [0x5B1E4, asm]
+      # Phase 5FM: C leaf func_8006E7E8 — VRAM 0x8006E7E8, size 0x4C.
+      # VSync poll helper: calls func_800811E4, checks for -1/0 return,
+      # clears D_800B0CD8 bit 0x01004000 on match. Era -O2 -G0.
+      - [0x5EFE8, c, func_8006E7E8]
       # Phase 5FK: C leaf func_8006E834 — VRAM 0x8006E834, size 0x16C.
       # Post-mount image loader + display env setup. Era -O2 -G0; paired
       # $v1-backup/$v0-test register pins + two zero-code "=r":"0" barriers
diff --git a/scripts/build_us.sh b/scripts/build_us.sh
index 3f7cb74..adce9ad 100755
--- a/scripts/build_us.sh
+++ b/scripts/build_us.sh
@@ -538,7 +538,8 @@ SIZE_C_6A5BC=0x90
 SIZE_C_6A64C=0x28
 SIZE_C_6A674=0x260
 SIZE_C_6A8D4=0x110
-SIZE_5B1E4=0x3e50
+SIZE_5B1E4=0x3e10
+SIZE_C_6E7E8=0x4c
 SIZE_C_6E834=0x16c
 SIZE_C_6E9A0=0x234
 SIZE_C_6EBD4=0x10
@@ -1054,6 +1055,7 @@ OBJECTS=(
     "build/src/func_8006A674.c.o"
     "build/src/func_8006A8D4.c.o"
     "build/asm/disc1/5B1E4.s.o"
+    "build/src/func_8006E7E8.c.o"
     "build/src/func_8006E834.c.o"
     "build/src/func_8006E9A0.c.o"
     "build/src/func_8006EBD4.c.o"
@@ -2380,6 +2382,9 @@ era_compile src/func_800197F0.c build/src/func_800197F0.c.o -O2 -G0
 era_compile src/func_8007F7A8.c build/src/func_8007F7A8.c.o -O2 -G0
 # Phase 5EM: boot memory-region layout init; ordered absolute pointer stores.
```

### stash@{8}

Command: `git stash show -p stash@{8} | head -40`

```diff
(no tracked diff output; stash is untracked-only or empty)```

### stash@{9}

Command: `git stash show -p stash@{9} | head -40`

```diff
(no tracked diff output; stash is untracked-only or empty)```

### stash@{11}

Command: `git stash show -p stash@{11} | head -40`

```diff
(no tracked diff output; stash is untracked-only or empty)```

### stash@{13}

Command: `git stash show -p stash@{13} | head -40`

```diff
diff --git a/configs/USA/disc1.yaml b/configs/USA/disc1.yaml
index ef6b690..9ab5166 100644
--- a/configs/USA/disc1.yaml
+++ b/configs/USA/disc1.yaml
@@ -336,11 +336,16 @@ segments:
       - [0x55254, asm]
       # Phase 5CA: C leaf func_80064C20 — file 0x55420, size 0x10.
       - [0x55420, c, func_80064C20]
+      # Phase 5EO: C leaf candidate func_8006A674 — VRAM 0x8006A674,
+      # size 0x260. Five counting loops; era -O2 -G0 with the default-off,
+      # per-leaf three-word symbolic-store expansion.
+      # Mid-55430 carve: prefix 0x5A44, C 0x260, then prior 5EM leaf.
+      - [0x55430, asm]
+      - [0x5AE74, c, func_8006A674]
       # Phase 5EM: C leaf func_8006A8D4 — VRAM 0x8006A8D4, size 0x110.
       # Boot memory-region layout initializer: 19 ordered absolute pointer
       # stores with byte-address cursor arithmetic. Era -O2 -G0; no $gp.
-      # Mid-55430 carve: prefix 0x5CA4, C 0x110, resume 5B1E4.s 0x41F0.
-      - [0x55430, asm]
+      # Resume 5B1E4.s at 0x5B1E4; trailing asm size 0x41F0.
       - [0x5B0D4, c, func_8006A8D4]
       - [0x5B1E4, asm]
       # Phase 5BS: C leaf func_8006EBD4 — VRAM 0x8006EBD4, size 0x10.
diff --git a/scripts/build_us.sh b/scripts/build_us.sh
index c51a83c..937e737 100755
--- a/scripts/build_us.sh
+++ b/scripts/build_us.sh
@@ -1,6 +1,6 @@
 #!/usr/bin/env bash
-# Phase 5EM: Disc 1 rebuild with 212 C leaves (delay-slot sw family + era + proven call shapes
-# + func_8006A8D4 boot memory-region layout init on era -O2 -G0).
+# Phase 5EO: Disc 1 rebuild with 213 C leaves (delay-slot sw family + era + proven call shapes
+# + func_8006A674 five-loop boot initializer with a per-leaf maspsx gate).
 # (prior 98 + 5 memset/memcpy countdown leaves through func_8008D820).
 #
 # Assembles splat-generated .s → .o with MIPS LE binutils, compiles the
@@ -120,7 +120,8 @@ CFLAGS_LEAF="-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls -ffreestandin
 # C 64A48:  0x55248 → 0x55254 = 0xC
 # 55254:    0x55254 → 0x55420 = 0x1CC
 # C 64C20:  0x55420 → 0x55430 = 0x10
```

### stash@{14}

Command: `git stash show -p stash@{14} | head -40`

```diff
diff --git a/configs/USA/disc1.yaml b/configs/USA/disc1.yaml
index ef6b690..9ab5166 100644
--- a/configs/USA/disc1.yaml
+++ b/configs/USA/disc1.yaml
@@ -336,11 +336,16 @@ segments:
       - [0x55254, asm]
       # Phase 5CA: C leaf func_80064C20 — file 0x55420, size 0x10.
       - [0x55420, c, func_80064C20]
+      # Phase 5EO: C leaf candidate func_8006A674 — VRAM 0x8006A674,
+      # size 0x260. Five counting loops; era -O2 -G0 with the default-off,
+      # per-leaf three-word symbolic-store expansion.
+      # Mid-55430 carve: prefix 0x5A44, C 0x260, then prior 5EM leaf.
+      - [0x55430, asm]
+      - [0x5AE74, c, func_8006A674]
       # Phase 5EM: C leaf func_8006A8D4 — VRAM 0x8006A8D4, size 0x110.
       # Boot memory-region layout initializer: 19 ordered absolute pointer
       # stores with byte-address cursor arithmetic. Era -O2 -G0; no $gp.
-      # Mid-55430 carve: prefix 0x5CA4, C 0x110, resume 5B1E4.s 0x41F0.
-      - [0x55430, asm]
+      # Resume 5B1E4.s at 0x5B1E4; trailing asm size 0x41F0.
       - [0x5B0D4, c, func_8006A8D4]
       - [0x5B1E4, asm]
       # Phase 5BS: C leaf func_8006EBD4 — VRAM 0x8006EBD4, size 0x10.
diff --git a/scripts/build_us.sh b/scripts/build_us.sh
index c51a83c..937e737 100755
--- a/scripts/build_us.sh
+++ b/scripts/build_us.sh
@@ -1,6 +1,6 @@
 #!/usr/bin/env bash
-# Phase 5EM: Disc 1 rebuild with 212 C leaves (delay-slot sw family + era + proven call shapes
-# + func_8006A8D4 boot memory-region layout init on era -O2 -G0).
+# Phase 5EO: Disc 1 rebuild with 213 C leaves (delay-slot sw family + era + proven call shapes
+# + func_8006A674 five-loop boot initializer with a per-leaf maspsx gate).
 # (prior 98 + 5 memset/memcpy countdown leaves through func_8008D820).
 #
 # Assembles splat-generated .s → .o with MIPS LE binutils, compiles the
@@ -120,7 +120,8 @@ CFLAGS_LEAF="-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls -ffreestandin
 # C 64A48:  0x55248 → 0x55254 = 0xC
 # 55254:    0x55254 → 0x55420 = 0x1CC
 # C 64C20:  0x55420 → 0x55430 = 0x10
```

### stash@{15}

Command: `git stash show -p stash@{15} | head -40`

```diff
(no tracked diff output; stash is untracked-only or empty)```

## Branch deletion audit

The following local branches were fully merged into `main` (0 commits ahead) and approved for deletion:

- `leaves/from-grind-20260821` (remote exists)
- `merge/leaves-into-grind` (remote exists)
- `phase3-disc1-boundary-audit` (remote exists)
- `phase4-disc1-function-inventory` (remote exists)
- `phase4h-asm-only-rebuild` (remote exists)
- `phase4i-asm-rebuild-parity-audit` (remote exists)
- `phase4j-mipsel-gcc-provisioning` (remote exists)
- `phase5-disc1-first-c-leaf` (remote exists)
- `phase5ab-func-800CE3AC`
- `phase5ac-func-800CD5A4`
- `phase5ac-next-simple-leaf`
- `phase5ae-2a0c-hole-aware` (remote exists)
- `phase5b-integrate-first-c-leaf` (remote exists)
- `phase5c-next-c-leaf` (remote exists)
- `phase5d-next-c-leaf` (remote exists)
- `phase5e-next-c-leaf` (remote exists)
- `phase5eb-return0-twins`
- `phase5eg-first-branch` (remote exists)
- `phase5eh-arg-return` (remote exists)
- `phase5ei-first-nonleaf`
- `phase5ei-ready-from-reader`
- `phase5ej-d8009d28c-state`
- `phase5ej-outgoing-arg`
- `phase5ek-197f0`
- `phase5ek-d8009d270-bitwise`
- `phase5el-7f7a8`
- `phase5em-boot-6a8d4`
- `phase5en-loop-6a674`
- `phase5eo-maspsx-addiu-at`
- `phase5eq-6a64c`
- `phase5er-d1c-d48`
- `phase5es-loop-4bf08`
- `phase5eu-gp-loop-55724`
- `phase5ev-52bcc`
- `phase5ew-52bcc-o1`
- `phase5ex-6a674-o1`
- `phase5ey-boot-3e610`
- `phase5ez-boot-6a5bc`
- `phase5f-next-c-leaf` (remote exists)
- `phase5fa-boot-3e680`
- `phase5fb-boot-698d4`
- `phase5fc-boot-6e834`
- `phase5fd-2f9cc`
- `phase5fe-2f970`
- `phase5ff-374e8`
- `phase5fj-6e9a0`
- `phase5fk-6e834-pin-revisit`
- `phase5fl-698d4-barrier-revisit`
- `phase5i-next-gcc-friendly-leaf`
- `phase5j-func-80090A0C` (remote exists)
- `phase5k-func-8008F694` (remote exists)
- `phase5l-func-8008F868` (remote exists)
- `phase5m-func-8008F880` (remote exists)
- `phase5n-func-8008FCB4` (remote exists)
- `phase5o-func-800904A0` (remote exists)
- `phase5p-func-800904AC` (remote exists)
- `phase5q-func-800904B4` (remote exists)
- `phase5r-func-800904BC` (remote exists)
- `phase5s-func-800906B4` (remote exists)
- `phase5z-func-800CD59C`
- `phase6a-native-boot-black`
- `phase6b-visible-native-window`
- `phase6c-translated-main-black`
- `phase6d-r-verify-boot-rung`
- `phase6d-real-boot-rung`
- `phase6d-s-guest-memory-safety`

Protected branches left intact: `grind/continuous-decomp`, `feature/pe-native-visible-frontier-next`, `phase6e-b-provider-frontier`, `docs-github-wiki-mirror`, `phase5fm-main-barrier-revisit`, and `merge/lanes-20260821`.

Remote deletion targets among the approved list:

- `origin/tooling/maspsx-expand-div`
- `origin/phase4g-mipsel-toolchain-provisioning` (remote-only merged branch)

- `origin/leaves/from-grind-20260821`
- `origin/merge/leaves-into-grind`
- `origin/phase3-disc1-boundary-audit`
- `origin/phase4-disc1-function-inventory`
- `origin/phase4h-asm-only-rebuild`
- `origin/phase4i-asm-rebuild-parity-audit`
- `origin/phase4j-mipsel-gcc-provisioning`
- `origin/phase5-disc1-first-c-leaf`
- `origin/phase5ae-2a0c-hole-aware`
- `origin/phase5b-integrate-first-c-leaf`
- `origin/phase5c-next-c-leaf`
- `origin/phase5d-next-c-leaf`
- `origin/phase5e-next-c-leaf`
- `origin/phase5eg-first-branch`
- `origin/phase5eh-arg-return`
- `origin/phase5f-next-c-leaf`
- `origin/phase5j-func-80090A0C`
- `origin/phase5k-func-8008F694`
- `origin/phase5l-func-8008F868`
- `origin/phase5m-func-8008F880`
- `origin/phase5n-func-8008FCB4`
- `origin/phase5o-func-800904A0`
- `origin/phase5p-func-800904AC`
- `origin/phase5q-func-800904B4`
- `origin/phase5r-func-800904BC`
- `origin/phase5s-func-800906B4`

## Documentation changes

- Current handoff heading corrected from 275 to 287 and current HEAD updated to `fc2b2f4`.
- Cross-lane status pointer added to the handoff, pointing to `~/dev/pe-continuous-decomp/RUNTIME_LANES.md`.
- `CAMPAIGN_DASHBOARD.md` marked `SUPERSEDED_BY_RUNTIME_LANES`; historical rows remain intact.
