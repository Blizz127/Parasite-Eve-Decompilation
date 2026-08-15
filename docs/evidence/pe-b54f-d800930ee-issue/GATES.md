# Gates

Matching EXE SHA-1 is unchanged: this rung edits `pc_port/` and
docs only.

```text
native=576/576
asan_ubsan=576/576, zero sanitizer diagnostics (Fedora toolbox
           jk2026-dev, PE_PORT_SANITIZERS=ON)
focused_B54F=2/2
retained_B54E=2/2
retained_B54C=2/2
retained_B54D=2/2
retained_B54A=8/8
retained_B54B=8/8
retained_B54C_material=8/8
retained_B54C_718D0=8/8
retained_B54E_oracle=8/8
new_B54F_oracle=8/8
focused_D=8/8
focused_C=10/10
focused_B2=15/15
focused_B1=8/8
focused_H=8/8
focused_B53B=15/15
B49=PASS normal; PASS ASan+UBSan
determinism=3/3 byte-identical framebuffer and trace
exe_oracles=50/50

framebuffer_sha256=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
trace_sha256=42c1956e077a40fed5176653b6a18938a8a91e99e35fe9d7044f31581de785af
disc_load_trace_sha256=7b8724acf4d4787f58ca0068e68839f171e2d3f36f72a42f0a4ef03f0041672b
fnv=7D860391E1ED6C97
matching_exe=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
strict_frontier=func_8006AD40_prefix_cut from func_8006AD40
bootstrap_strict=func_8007F72C from func_800698D4
```

Independent oracles that take the executable all passed (50/50).
`b21_bzero_oracle.py` uses `DST LEN` and is not in that set.
B54F's own 8/8 is `pc_port/tools/b54f_6ad40_d800930ee_oracle.py`.

Framebuffer digest is unchanged and **blind to this upload**.
HostFB is a 320×240 host buffer, not VRAM; the atlas RECT starts
at x=320, outside that width. Coverage for the atlas is the
focused B54F LoadImage + pack assertions, plus the HostFB-unchanged
check in that test. Do not treat `fb28dc21…` as evidence the
atlas still lands.
