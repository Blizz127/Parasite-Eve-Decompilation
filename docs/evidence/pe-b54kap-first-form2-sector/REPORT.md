# PE-B54K-AP — first retail Form-2 stream sector

Status: **RETAIL SECTOR CONTRACT CLOSED; NO STREAM DELIVERY IMPLEMENTED**.

This rung reads the user-supplied Disc 1 image without modifying it.  The
movie path already proved `\FMV1\FMV001.STR;1` at LBA 189742.  The first
retail sector at that LBA is not an ISO9660-style 2048-byte payload.

## First-sector authority

Raw sector 189742 is 2352 bytes and has SHA-256:

```text
84210c5a23b682cf8972f477cd0065568a85482f8391c353c27c1cd5bf15c5a3
```

Its byte-level geometry is:

```text
raw +0x00..+0x0B  00 ff ff ff ff ff ff ff ff ff ff 00  (sync)
raw +0x0C..+0x0F  42 11 67 02                          (MSF + mode 2)
raw +0x10..+0x17  01 01 48 00 01 01 48 00              (duplicated subheader)
raw +0x18..+0x92B 2324-byte Form-2 payload
```

The payload SHA-256 is
`aeb95a2e68ba186f95bb1194520a4f615ce005356a827897a8dcbccf06e48191`.
Its first 24 bytes decode, little-endian, as:

```text
magic          0x80010160
chunk index    0
chunk count    9
frame number   1
frame-data length field  0x00000A98
width          320
height         240
```

The next sectors independently confirm that this is a multiplexed stream,
not a contiguous 2048-byte file read.  LBAs 189742..189748 carry video chunks
0..6 with subheader tuple `(file=1, channel=1, submode=0x48, coding=0)`.
LBA 189749 instead carries tuple `(1,1,0x64,1)` and no `0x80010160` header.
LBAs 189750 and 189751 resume video chunks 7 and 8.  Therefore the duplicated
subheader is routing information and the complete 2324-byte Form-2 payload is
part of the observable retail input contract.

## Native boundary

`PE_Disc_ReadUserSector` intentionally copies exactly 2048 bytes from raw
offset 24. `PE_Disc_ReadUserData` crosses sectors in the same 2048-byte unit,
and the only 2352-byte reader is file-local `PE_Disc_ReadRaw`.  That API is
correct for ISO9660 files, but it cannot represent this stream:

- it discards the eight-byte duplicated subheader;
- it drops the final 276 bytes of every Form-2 payload;
- it cannot distinguish the interleaved `0x48` video and `0x64` XA sectors.

Consequently `CdlReadS` must not be implemented through
`PE_Disc_ReadUserData`.  The next implementation rung first needs a separate,
bounds-checked, read-only raw/Form-2 API that preserves the subheader and all
2324 payload bytes.  The existing ISO API remains unchanged.  No sector was
delivered, no callback was invoked, and the strict frontier remains before
`func_8007F0C8`.

The independent oracle is
`pc_port/tools/b54kap_first_form2_sector_oracle.py`.

```text
RETAIL_FIRST_STREAM_LBA=189742
RETAIL_STREAM_SECTOR=MODE2_FORM2_2324
RETAIL_STREAM_IS_INTERLEAVED=YES
ISO_2048_API_SUFFICIENT_FOR_CDLREADS=NO
PRODUCTION_REACHABILITY=blocked_at_func_80081314_func_8007F0C8_cut
NEXT_RUNG=read_only_raw_form2_api_with_first_sector_and_negative_controls
```
