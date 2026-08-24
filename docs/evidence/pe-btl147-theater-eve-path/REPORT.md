# PE-BTL147 — theater → Eve-entry dependency audit

This is a retail evidence rung. It does not modify persistent state, inject a
destination, or claim that the native runtime can enter the encounter.

## Retail result

The exhaustive Disc 1 field-script scan found eight `0x31` hops to `m0005i`,
including the two authentic `m0004i` module-0 paths:

```text
m0004i mod0 +0x06A0 -> 0xA80002C8 (m0005i), persist[0x4A]=0x18
m0004i mod0 +0x07B4 -> 0xA80002C8 (m0005i), persist[0x4A]=0x18
```

It found zero field-script `0x31` hops to `m0360i` (`0xA8066048`). The
scanner covered 414 scripts and 1,011 immediate `0x31` tokens. The packed
`m0360i` name and destination token were also absent from the executable's
destination-immediate scan.

## The missing persistent dependency

The retail `m0360i` module 2 body contains the unique persistent bit-4 write:

```text
+0x03E4 alu eq  imm=[0xB, 0x0, 0x4, 0x4]
+0x040C alu or  imm=[0x2, 0x1, 0x0, 0x4]
+0x0424 mov      imm=[0x0, 0x1]
```

The comprehensive persist scan identifies this as `persist[0] |= 4` and
finds no other retail writer. Therefore the native `m0005i` first-visit gate
cannot be satisfied by adding a convenience flag at the battle entry.

The authentic m0005i module-4 trigger remains the physical type-4 `0x77`
volume. Its gates are `persist[0x4A] < 0x28`, then `persist[0x4A] >= 0x11`,
followed by the type-0 payload `0x0D`. The m0004i route supplies
`persist[0x4A]=0x18`, so the unresolved dependency is the authentic route into
the m0360i event, not the m0005i volume geometry.

## Native boundary

The current native branch has translated field/battle leaf functions and
focused tests, but no generic scene scheduler/field-script runner that can
enter m0360i from the theater. Consequently this rung adds evidence only; it
does not implement a fake `persist[0]` write or a fake destination hop.

## Reproduction

```text
python3 tools/research/pe_btl147_theater_eve_path.py
cmake --build pc_port/build -j2
pc_port/build/pe-native-tests
Results: 928 run, 928 passed, 0 failed, 0 skipped
```

The machine-readable scan is `scan.json` in this directory.
