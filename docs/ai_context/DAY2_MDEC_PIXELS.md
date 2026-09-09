# Streaming MDEC pixel decoder

Stage151 adds a bounded software pixel decoder to the existing MDEC owner.
Quantization and signed scale-table uploads now retain their actual contents
alongside existing upload telemetry. BeginDecode copies a complete run-length
command into owned storage; ReadPixels lazily decodes and drains one macroblock
at a time. This avoids a full-frame allocation and permits split output reads.
These APIs are not yet connected to libpress compressed-input/output DMA calls.

The decoder supports quantizer bypass, signed10-bit coefficients, run skips,
zigzag placement, dequantization and coefficient saturation, two programmable
IDCT passes, monochrome4/8-bit output, color24/15-bit output, signed samples
and the15-bit high-bit flag. Colored macroblocks are returned in16x16 raster
order for eventual DMA1 consumption. Malformed blocks and absent tables stop;
a malformed first block cannot fabricate pixels. Reset aborts pending decode
while retaining device tables; power initialization clears them.

Numerical behavior follows the documented low-level model in
[PSX-SPX MDEC](https://psx-spx.consoledev.net/macroblockdecodermdec/).
The documentation itself marks exact IDCT/color rounding as uncertain.
This implementation uses explicit floor division, the documented IDCT rounding
bias and integer color coefficients; it is **not verified hardware-pixel-exact**.
General retail-frame pixel comparison is still required. Timing, FIFO pressure,
DMA0/1 scheduling, interrupt completion and bitstream-to-RLE integration are
separate unfinished work. No third-party decoder implementation was imported.
The FPGA research inspected under local/live/mdec-research-151 also describes
precision validation as work in progress; it is not a hardware oracle here.

`test_mdec_pixels.h` exercises272 analytical/contract cases:252 scan-position,
quantizer-bypass and signed-coefficient impulses;16 output-format combinations;
three DC-only cases using authenticated retail matrices and the existing
libpress reset; and a malformed-block stop. A diagonal programmable scale
matrix makes impulse results exact without relying on a second copy of the
same IDCT algorithm. Color cases distinguish all four luma quadrants and both
chroma channels; split reads exercise retained macroblock output.

`pe_mdec_pixel_tables.py --check` authenticates256 bytes of original table
payloads, matching the previously verified libpress reset hashes. It extracts
fixtures, not golden decoded pixels or new original control-flow traces.

Next connect original BFA0/C1EC input and C01C/C27C output calls to decoding and
DMA1, preserving scheduled completion and existing movie callback order. The
original libpress I/O disassembly is local/live/libpress-io-151.asm. Movie
player121C04, updater122040 and loader14E30 remain unfinished. Full opening
through Day2 and100%Day1 acceptance are not established. Runtime128 stays
published.

Validation: normal and ASan/UBSan focused runs each pass43 DAY2 groups with
1297 skipped (1340 total). Full CTest passes8/8 in173.56s, including1340/1340
native groups with0 skipped. Final builds are warning-free; authenticated
table extraction/check, Python compilation and scoped whitespace pass.
Logs: `local/live/*day2-151*.log`. These checks do not establish hardware-exact
rounding or complete movie playback.
