# CD audio setup, mode and VBlank installation helpers

Stage145 translates7BAC0,812F4 and73D58. Audio setup uses the existing SPU
register owner for physical addresses and preserves guest-RAM pointer behavior.
The VBlank updater test now installs its slot through73D58 instead of calling
the host slot helper directly.

Original executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 7BAC0..7BBB0 | 60 | 09ca2dbe6a1149d6028cc5cae7637f983db117ef812194f394749792702d11c0 |
| 73D58..73D88 | 12 | 2f36b0dd40395ec821cd3a2cd88ac08d7729d4eb777d4394c4b96b3e507faf5f |
| 74478..744A4 | 11 | 4285c503a12de6963046b674eb547659c920fae822184ad2b76329c32a58887b |
| 812F4..81310 | 7 | 648a55660691f99fcb11328b0f30e22f9205656cb1d7e3686dec74e41daf5cf9 |

7BAC0 tests both SPU current-volume halfwords. If both are zero it sets master
volumes3FFF; otherwise it preserves them. It always sets CD volumes3FFF and
controlC001, then programs CD banks2/3 with80,0,80,0 and applies the gains.
812F4 stores only unsigned modes0/1.73D58 delegates to the existing guest-backed
slot setter and returns the previous identity. Its canonical installed SDK
jump-table assumption matches73D24; arbitrary pre-install/dirty targets are
not newly supported.

The CD register owner now retains four pending and four applied audio gains.
ATV0/1 use bank2 registers2/3; ATV2/3 use bank3 registers1/2. Bank3 register3
bit5 applies all four pending gains. This is register state only; no XA/CD-DA
sample decoding or mixing is added. Bank meanings and the apply bit are
cross-checked against the hardware research documentation:
[PSX-SPX CDROM registers](https://psx-spx.consoledev.net/cdromdrive/).
The original executable independently supplies this game's write sequence.

`pe_cd_startup_helpers_oracle.py --check` verifies18 complete original audio
setup graphs,72 installed slot-wrapper graphs and8 mode-setter graphs. Native
compares the entire512-byte SPU register region and CD shadow bytes; each audio
case runs through guest RAM and three physical-address aliases. Slot cases
cover all8 valid slots, same/changed/null identities and untouched neighboring
slots/counter. Native device checks verify pending versus applied gains,
reset clearing and isolation from a queued CD response. The3360 updater cases
also exercise real73D58 slot installation before VBlank dispatch.

Outer7F994/7EC14 startup wiring remains incomplete, particularly successful
physical command responses. Sector FIFO/DMA, MDEC output, movie player/updater/
loader and full opening-through-Day2 acceptance remain unfinished.128 stays published.

Normal and ASan/UBSan focused runs each pass38 DAY2 groups with1297 skipped.
Both builds are warning-free; original fixture regeneration/check, Python
compilation and scoped whitespace pass.
Full CTest passes8/8 in115.53s, including1335/1335 native groups with0 skipped;
log `local/live/ctest-day2-145.log`.
