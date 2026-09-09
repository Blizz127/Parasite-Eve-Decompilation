# Original SDK formatter and copy behavior

DAY1/DAY2-77. Source71A84..72308 is545 words in621E4.s, SHA256
3eaf80663eeaeaa09560a87c5468b74afad3fa6ea1177d7a81b6f15eae4331e4.
This contract decompiles the source behavior. Only its72334 copy dependency is
native at this checkpoint; the formatter/card call boundaries remain unresolved.

## Parsing and conversions

Formatter saves a1/a2/a3 to the caller's argument area, allocates250h bytes,
and consumes 32-bit argument words starting at originalSP+8. Local digit buffer
ends at frame+210h. Per-conversion defaults are three words copied from94528
(all zero in the pinned executable), including flags, width and precision.
Format bytes are read signed. Flags repeat until another byte appears:
minus=bit1,plus=bit2,hash=bit4,zero=bit8; space writes the sign byte at211h.
Width is decimal accumulation with32-bit wrap or a signed star argument.
Negative star width is negated modulo32-bit and sets left alignment. Precision
is decimal or star after a dot; only nonnegative precision sets bit10h. Left
alignment clears zero padding.

The45-entry dispatch table11644 covers characters4Ch..78h. Supported entries:

| Conversion/modifier | Entry | Behavior |
| --- | --- | --- |
|h|71D48|Set20h; integer narrowing and short count store.|
|l|71D54|Set40h; continue parsing, still consumes32-bit arguments.|
|L|71D60|Set80h; continue parsing, still consumes32-bit arguments.|
|d,i|71D8C|Signed decimal; h sign-extends low16; negative magnitude uses wrapping negation.|
|u|71DE4|Unsigned decimal; h masks low16; clears sign byte.|
|o|71F04|Unsigned octal; h masks low16.|
|p|72004|Set precision8 and flags50h; continue to uppercase hex.|
|X|72018|Uppercase digit table1161C.|
|x|72024|Lowercase digit table11630.|
|c|7212C|Consume one word, output its low byte, including embedded NUL.|
|s|7214C|String, including hash-prefixed counted-string extension.|
|n|721D8|Store emitted count to supplied pointer:16 bits for h, otherwise32.|

Other conversions terminate the entire formatter immediately, except percent
which emits one percent byte. Final NUL is always stored, and return is emitted
byte count excluding that final NUL. There is no destination-size parameter.
Do not introduce silent truncation or replace unknown conversions with literals.

Decimal/octal/hex digit generation is unsigned after signed magnitude selection.
Without explicit precision, zero padding derives precision from width (minus
sign byte for decimal, minus2 for alternate hex); precision is then at least1.
With explicit zero precision, zero values emit no digits. Octal alternate form
adds a leading zero only when a digit already exists and is not zero. Hex
alternate form adds zero and the original conversion byte even for zero value.
Numeric text is constructed backward in local storage. Signed prefixes precede
zero padding. Right spaces are emitted before copying, decrementing width;
left spaces follow copying. All width/count arithmetic follows32-bit operations.

Ordinary strings use72314 (BIOS A0/1B) for unbounded length, or72324
(BIOS A0/2E) to find NUL within precision. Those BIOS semantics still need
explicit native/original authority. Hash+s takes unsigned first byte as length,
advances pointer, and caps to precision if supplied; it does not call the BIOS.
%n bypasses padding/copy and proceeds to the next format byte.

## Original execution evidence

pe_formatter_contract.py pins executable SHA1 and formatter SHA256, decodes the
actual dispatch table, and executes432 original formatter calls (27 formats x16
integer edge values), including real card filename/device formats. It covers
signed/unsigned narrowing, modifiers, flags, width/precision stars, pointers,
characters, counted strings, count stores, invalid conversion and percent.
All432 calls finish with return count and NUL verified. Raw outputs, argument
words and count-store effects are inlocal/live/formatter-contract.json. This is
original behavior evidence, not a native formatter pass.

Notable observed outputs:

| Format/input | Exact output |
| --- | --- |
|p with0|00000000|
|p withFFFFFFFF|FFFFFFFF|
|hash08x with0|0x000000|
|hash.precision0.o with0|empty|
|A percent-q B|A (formatting stops)|
|hash+s with bytes03,'a','b','c'|abc|
|hash precision2 s with same data|ab|
|Ld withFFFFFFFF|-1|

A%nB outputsAB and stores1 as a word. A%hnB stores only the low halfword,
preserving following bytes. Card formats remain those inCARD_OPERATION_CONTRACT:
`bu%d0:BASLUS-00662000000%c%c`, `bu%ld0:`, and `bu%ld0:%s`.
The last ordinary-string format is source-traced, not executed by this432-case
set because its BIOS length dependency is unresolved in the reader.

## Native copy dependency

72334..723A0 (27words), SHA256
9b433f057891fb4d72c9ba2a09d9dc6d95e85cf1f7f8a2e82b40ce03d85f4732,
is now native in func_80072334_port.c. Signed count<=0 causes no access.
Unsigned destination>=source copies backward and returns the original destination.
Destination<source copies forward and returns destination+positivecount. Raw
pointer ordering is preserved even when KSEG0/KSEG1 addresses alias the same RAM;
therefore this is not generally interchangeable with host memmove. The formatter
ignores the helper's return, but the native helper retains its exact behavior.

The port memory API requires canonical RAM addresses. An initial native audit
aborted onA0140040; the helper now translates low physical/KSEG1 RAM addresses
only at each access, retaining raw addresses for ordering/arithmetic/return.
It does not broaden global RAM validation or map unrelated address regions.
pe_formatter_copy_oracle.py compares2048 original/native cases with overlap,
raw alias ordering, positive/zero/negative counts and return values. Original
RAM expected hashes are regenerated, not inferred from host memmove.

Remaining: native full formatter, guest argument/temporary-buffer binding,
ordinary string BIOS dependencies, oversized local-buffer behavior and actual
card I/O. This checkpoint does not establish full menu, Day1/Day2 or live acceptance.

Validation: original copy generation and exact header verification passed;
normal and sanitizer focused copy group passed (1277 unrelated groups skipped).
Full CTest passed all8 targets in72.62 seconds. Both builds were warning-free.

## DAY1/DAY2-78: explicit-frame native formatter core

`PE_FormatterFrame` in `func_80071A84_port.c` now implements parsing,
integer/pointer/character/counted-string/count-store conversions, live dispatch
and digit tables, padding, final NUL and return count. It accepts an established
caller stack address and incoming s0..s7/ra values explicitly. It writes the
original argument spill, saved-register and temporary-memory locations; numeric
text is built in guest memory and copied with72334. Arithmetic wraps at32 bits.
The original is not replaced by host printf, truncation, or host temporary text.

This is a memory/output kernel, not a complete machine-context adapter. Incoming
register values establish saved-register memory; eventual register restoration,
including overwritten saved slots, still belongs to a caller adapter. Card
callers do not yet have that established context and retain their71A84 boundary.
Ordinary strings stop at72314 or72324 before any callee effect, suffix padding,
final NUL, or subsequent conversion. Only the callee's semantic arguments are
recorded (pointer for72314; pointer,zero,precision for72324). A returned count
is usable only when no stop was requested. Unknown live dispatch targets stop.

`pe_formatter_frame_oracle.py` pins the545-word source and executable, executes
880 original calls and generates expected hashes for output/format/string memory
and the complete exercised stack region. Cases include the prior432 formats,
empty/trailing percent, repeated modifiers, large precision, explicit BIOS stops,
three stack placements, output overlap with temporary/count-string storage,
%n modifying a later argument, and nonzero live conversion defaults. Oversized
precision530 exercises writes before the allocated frame without a host buffer
or artificial cap. These cases do not prove arbitrary corrupted-frame behavior,
unknown BIOS semantics, register restoration, or live card integration.

Source correction: the initializer copies three words from94528, not four;
the previous contract overstated the copied range. The native core and oracle
include the three actual words, including their live nonzero values.

Stage78 validation: all880 original frame comparisons passed in normal and
sanitizer builds; focused output is1passed,1278skipped. Full CTest passed8/8 in
71.87 seconds. Both final builds were warning-free. Exact generated-header
verification, Python compilation and scoped whitespace checks passed.

Stage79 integrates this kernel into the explicit-context card dispatcher entry
`PE_CardOperationFrame`. Original1024 dispatcher calls verify nested frame and
formatted-output effects through the first BIOS call. The context-free live
entry still stops at71A84 until its parents establish the required context.
See `CARD_OPERATION_CONTRACT.md` stage79.

Stage80 also connects the supplied-context cleanup path: filename formatting
returns into a second formatter call for `bu%ld0:%s`, which explicitly stops at
72314. The expanded1280-case card audit verifies nested memory effects and the
partial output. No ordinary-string BIOS return or live card completion is claimed.
