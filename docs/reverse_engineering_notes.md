# Reverse engineering notes

Running log of verified observations about the Parasite Eve (NTSC-U)
binaries and disc data. **Every entry must cite its evidence**: the tool,
command, address/offset, or checksum that backs it. Speculation goes in a
clearly marked "Hypotheses" section, never mixed with facts.

Nothing has been verified yet — this file starts empty by design (Phase 0).

## Template for entries

```markdown
### <topic> (YYYY-MM-DD)

- Observation: ...
- Evidence: <tool + command + output excerpt / address / hash>
- Confidence: verified | probable | hypothesis
```

## Known starting points (unverified, to confirm in Phase 1–2)

- Main executables: `SLUS_006.62` (disc 1), `SLUS_006.68` (disc 2). The two
  discs likely share most engine code; diffing the two EXEs early may pay
  off.
- Era/toolchain: 1998 Square PS1 title — expect Psy-Q SDK libraries linked
  into the EXE and a Psy-Q-era gcc for game code. Confirm via library
  signature matching before assuming compiler flags.
- Expect overlays and/or file-based code loading from disc; PE2 uses
  overlay-heavy structure and PE1 likely does something similar. Verify, do
  not assume.

## Reference projects (structure only — do not copy source)

- Parasite Eve 2 decomp: https://github.com/GabeRealB/parasite-eve-2-decomp
  (splat-based layout, ninja build, checksum verification flow)
- Xenogears decomp — comparable Square-era discipline and tooling.

## Hypotheses

(none yet)
