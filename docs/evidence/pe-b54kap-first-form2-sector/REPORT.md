# PE-B54K-AP — first retail multiplexed stream sectors

Status: **RETAIL SECTOR CONTRACT CORRECTED AND CLOSED; NO STREAM DELIVERY
IMPLEMENTED**.

This rung reads the user-supplied Disc 1 image without modifying it. The movie
path already proved `\FMV1\FMV001.STR;1` at LBA 189742. Its opening sectors
prove that one STR mixes Mode-2 Form-1 video sectors and Mode-2 Form-2 XA audio
sectors.

## Video authority: Mode-2 Form-1

Raw sector 189742 is 2352 bytes and has SHA-256:

```text
84210c5a23b682cf8972f477cd0065568a85482f8391c353c27c1cd5bf15c5a3
```

Its byte-level geometry is:

```text
raw +0x000..+0x00B  00 ff ff ff ff ff ff ff ff ff ff 00  (sync)
raw +0x00C..+0x00F  42 11 67 02                          (MSF + mode 2)
raw +0x010..+0x017  01 01 48 00 01 01 48 00              (duplicated subheader)
raw +0x018..+0x817  2048-byte Form-1 data
raw +0x818..+0x92F  280-byte Form-1 EDC/ECC tail
```

The Form-1 data SHA-256 is
`810cc410d14efad506d757aab639f09c4aeb3591391fada7afd9a4b67e821a56`.
Its first 24 bytes decode, little-endian, as:

```text
magic                   0x80010160
chunk index             0
chunk count             9
frame number            1
frame-data length field 0x00000A98
width                   320
height                  240
```

The XA submode Form bit is `0x20`. It is clear in video submode `0x48`, so
calling the 2324 bytes after offset 24 a Form-2 payload would be incorrect:
the final 280 of those bytes are the Form-1 EDC/ECC tail.

## Interleaved audio authority: Mode-2 Form-2

LBAs 189742..189748 carry video chunks 0..6 with subheader tuple
`(file=1, channel=1, submode=0x48, coding=0)`. LBA 189749 instead has raw
SHA-256
`7b4a69c55db31ddf2c61069793855ffb08d3181b64780c7a68dcd2e18bb6741c`
and tuple `(1,1,0x64,1)`. Here the Form bit is set, so bytes
`+0x18..+0x92B` are the complete 2324-byte Form-2 data region; its SHA-256 is
`8a6094b8f60717e897286181ad0c372e3d3dba83ae9679d87b28c108cf316187`.
The final four raw bytes are its EDC. LBAs 189750 and 189751 then resume video
chunks 7 and 8.

This proves that the duplicated subheader selects sector role and payload
geometry; it is not disposable framing.

## Native boundary

`PE_Disc_ReadUserSector` intentionally copies exactly 2048 bytes from raw
offset 24. `PE_Disc_ReadUserData` crosses sectors in the same 2048-byte unit,
and the only 2352-byte reader is file-local `PE_Disc_ReadRaw`.

That API correctly exposes the first video sector's Form-1 data for ISO-style
reads, but it cannot represent the multiplexed `CdlReadS` stream:

- it discards the eight-byte duplicated subheader, so video and XA cannot be
  classified;
- it truncates each Form-2 XA data region by 276 bytes;
- it provides no variable Form-1/Form-2 geometry contract.

Consequently `CdlReadS` must not be implemented solely through
`PE_Disc_ReadUserData`. The next rung first needs a separate, bounds-checked,
read-only Mode-2 stream-sector API that returns the duplicated subheader and
the correct 2048- or 2324-byte data region. The existing ISO API remains
unchanged. No sector was delivered, no callback was invoked, and the strict
frontier remains before `func_8007F0C8`.

The independent oracle is
`pc_port/tools/b54kap_first_form2_sector_oracle.py`.

```text
RETAIL_FIRST_STREAM_LBA=189742
RETAIL_VIDEO_SECTOR=MODE2_FORM1_2048
RETAIL_XA_SECTOR=MODE2_FORM2_2324
RETAIL_STREAM_IS_INTERLEAVED=YES
ISO_2048_API_SUFFICIENT_FOR_CDLREADS=NO
PRODUCTION_REACHABILITY=blocked_at_func_80081314_func_8007F0C8_cut
NEXT_RUNG=read_only_mode2_stream_sector_api_with_geometry_controls
```
