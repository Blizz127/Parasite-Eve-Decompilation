# PE-BTL7 — 144FC 0x3B at 0x80014630 is overlay word 0

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`0x80014630` is JT[0x3B] inside `func_800144FC` (102 words),
not a separate function. Window `0x80014630..0x80014660` is
12 words, SHA-256 `3e531fb7…c1ea5bab`. Zero `jal`/`jalr`
through the 144FC epilogue. Not `6914C`. Not `0x800E086C`.

`$s0` = `D_800B0CD8`. 0x3B and state 0 `lw`/`sw` `0($s0)`
(`8E02`/`AE02`). `$s1` is 144FC a0; only 0x3A uses `lw 0($s1)`
(`8E22`) for `lbu(*binder)`. REJECTED: 0x3B `*s1` binder
(misdecode of `8E02` as `8E22`).

## 0x3B

```text
lbu +0xE($s0); andi 3
bnez → 0x80014660     # v0=0, rewind gp+0x90 by 12
lw 0($s0); and 0xFF7FFFFF; sb F4=0; sw 0($s0)
j 0x8001467C          # epilogue, delay li v0,1
```

Live `+0xE` bit1 keeps v0=0. 3F074 polls `6C5BC` while v0==1.
Poll exit is EE=0 idle `jal 6CC68` v0=0 after 6CC2C. EE=13
returning 1 is not `0x55` complete.

## Verify

```text
python3 pc_port/tools/pe_btl6_14630_oracle.py
python3 pc_port/tools/pe_btl7_3f074_poll_oracle.py
PE_TEST_FILTER=BTL6_144FC_0x3B ./pc_port/build/pe-native-tests
```
