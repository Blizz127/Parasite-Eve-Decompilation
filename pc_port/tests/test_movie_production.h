/* DAY2-159b: production movie frame is delivered complete before decode.
 *
 * func_801924F8's E0 poll only exits when a stream record has been
 * published (state 2).  On a real drive that publish comes from
 * func_8007C214, which func_8007C564 calls exactly once per frame — after
 * it has assembled the *last* video chunk and set D_800B89F4.  The VLC
 * decoder func_8010C89C has no output bound except the bitstream's own
 * pad/terminator, so a partial body walks its arena cursor past the 2 MiB
 * guest window (the MV1d trap, docs/evidence/pe-mv1d-c89c/REPORT.md).
 *
 * This test calls the real production func_801924F8 on the real Disc 1
 * opening movie (FMV001, frame 1).  It pins the invariant end to end:
 *   1. the E0 poll advances the modeled CD device and the publish is a
 *      last-chunk publish (PE_Movie_LastPublishComplete);
 *   2. func_8010C89C was entered at the assembled body cursor and its RLE
 *      output matches the known frame-1 hash without exceeding its declared
 *      length (the guard bytes after it stay untouched).
 * Reference hashes come from retail_movie_complete_frame_cases.h, the same
 * frame the direct-decode test DAY2_movie_complete_frame uses. */
static void test_DAY2_movie_production_frame(void)
{
    char err[256] = {0};
    uint8_t raw[2352];
    const pe_addr_t suffix      = 0x801F0000u;   /* record[0] name      */
    const pe_addr_t record      = 0x801D0E00u;   /* record table entry 0 */
    const pe_addr_t record_base = 0x8014F800u;   /* D_800C0DC8          */
    const pe_addr_t body        = 0x80150000u;   /* base + 64*32        */
    const pe_addr_t table       = 0x80130000u;   /* D_801D0DF8          */
    const pe_addr_t out         = 0x80160000u;   /* got-frame a1        */
    PeC89CTelemetry tel;
    PE_Disc *disc;
    unsigned tick;

    TEST("DAY2_movie_production_frame");
    disc = BTL6_OpenDisc1(err, sizeof(err));
    ASSERT(disc != NULL, err[0] ? err : "production movie disc unavailable");

    ResetTestState();
    HostFB_Init();
    PE_GPU_Init();
    func_80073C94();
    B54KR_SeedGpuStatic();
    PE_Disc_SetActive(disc);
    CdDeviceSeed();
    ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14() == 1,
           "production CD startup");
    for (tick = 0; tick < 10u && PE_LoadU32(0x8009B574u) != 1u &&
         !PE_Port_ShouldStop(); tick++)
        HostFB_VSync(0);
    ASSERT(PE_LoadU32(0x8009B574u) == 1u, "production SDK ready");
    MOVAU_SeedRegisterPointers();

    /* libpress module -> VLC table, exactly as the frame oracle loads it. */
    for (unsigned sector = 0; sector < 38u; sector++) {
        ASSERT(PE_Disc_ReadRawSector(disc, 1940u + sector, raw),
               "libpress sector read");
        memcpy(PE_Translate(0x8010BCF8u + sector * 2048u, 2048u), raw + 24, 2048u);
    }

    /* Production record/stream state for FMV001 (movie table entry 0). */
    memcpy(PE_Translate(suffix, 16u), "\\FMV001.STR;1", 15u);
    PE_StoreU32(record, suffix);
    PE_StoreU8(record + 4u, 1u);        /* wide kind                    */
    PE_StoreU16(record + 6u, 1u);       /* stream start frame 1         */
    PE_StoreU16(record + 10u, 0u);      /* movie_x                      */
    PE_StoreU16(record + 12u, 0u);      /* movie_y                      */
    PE_StoreU8(0x801D0E18u, 1u);
    PE_StoreU32(0x801D0DE8u, 0x80165000u);
    PE_StoreU32(0x801D0DECu, out);
    PE_StoreU32(0x801D0DF0u, 0x80166000u);
    PE_StoreU32(0x801D0DF4u, 0x80167000u);
    PE_StoreU32(0x801D0DFCu, record_base);
    PE_StoreU32(0x801D0DF8u, table);
    PE_StoreU32(0x800ACDDCu, 0u);
    PE_StoreU8(0x800B0DBEu, 0x98u);
    PE_StoreU16(0x801D11B0u, 0xFFFFu);

    /* Run the real production frame worker.  Its E0 poll must advance the
     * modeled device and decode only a complete frame. */
    PE_Movie_ResetPublishState();
    ASSERT(func_801924F8(0) == 0 && !PE_Port_ShouldStop(),
           "production frame worker stopped");

    PE_C89C_GetTelemetry(&tel);
    ASSERT(tel.a0 == body, "decoder did not run on the assembled body");
    ASSERT(tel.ret == 0, "decoder did not terminate via the pad exit");
    ASSERT(B54K_MdecFnv1a64(body, MOVCOMPLETE_cases[0].chunks * 2016u) ==
           MOVCOMPLETE_cases[0].input_hash,
           "assembled production frame differs from the disc bytes");
    ASSERT(B54K_MdecFnv1a64(out, MOVCOMPLETE_cases[0].length) ==
           MOVCOMPLETE_cases[0].output_hash,
           "production decode RLE differs from original");
    ASSERT(PE_LoadU16(0x800B0DBCu) == 1u && PE_LoadU8(0x800B0DBAu) == 1u,
           "production frame exit state differs");
    for (unsigned i = 0; i < 16u; i++)
        ASSERT(PE_LoadU8(out + MOVCOMPLETE_cases[0].length + i) == 0u,
               "production decode exceeded its declared output length");

    /* Gate predicate: 7C214 reports a complete publish only when the
     * last-chunk flag D_800B89F4 is still set.  A surrogate (fixture)
     * publish must not be treated as a decodable complete frame. */
    PE_Movie_ResetPublishState();
    PE_StoreU32(0x800B89F4u, 0u);
    func_8007C214();
    ASSERT(!PE_Movie_LastPublishComplete(),
           "non-final publish was treated as a complete frame");
    PE_StoreU32(0x800B89F4u, 1u);
    func_8007C214();
    ASSERT(PE_Movie_LastPublishComplete(),
           "last-chunk publish was not treated as a complete frame");

    PE_Disc_Close(disc);
    PASS();
}

/* DAY2-159c: func_80192CE8's post-E08 media tail is present and runs.
 *
 * Index >= 47 makes func_801924F8 return at its record-range guard without
 * touching stream state, so func_80191FB8's D_800B0DBC = 0 survives and the
 * post-E08 loop's `(int16)D_800B0DBC <= 0` guard is taken on the first
 * check.  That exercises the tail's control flow and epilogue without the
 * movie/pad/DMA chain:
 *   - the call returns 0 rather than the removed named cut's -1,
 *   - no stop is requested,
 *   - overlay bit 0x200 is cleared.
 * Re-cutting the loop at 0x80192E08 (Bootstrap_ReturnVoid cut +
 * RequestStop) fails every assertion below: -1, stop requested, bit set. */
static void test_DAY2_92ce8_media_loop_tail(void)
{
    char err[256] = {0};
    PE_Disc *disc;

    TEST("DAY2_92ce8_media_loop_tail");
    disc = BTL6_OpenDisc1(err, sizeof(err));
    ASSERT(disc != NULL, err[0] ? err : "PE_Disc_Open failed");

    ResetTestState();
    DAY1_SeedDisplayDispatch();
    HostFB_Init();
    func_8007ED58();
    B558_PlantPointers();
    PE_Disc_SetActive(disc);

    /* B54KY: load the 133-sector PE.IMG overlay (the prefix authority). */
    D_800B0DD8 = 1013u;
    D_80011614 = 0x8018EFF0u;
    D_80093164[0] = 0x03D2u;
    D_80093164[1] = 0x0457u;
    ASSERT(func_8006E834() == 0, "retail 133-sector overlay load");
    B54KR_SeedGpuStatic();
    PE_StoreU32(0x800B0DD8u, 1013u);
    PE_StoreU32(0x8001160Cu, 0x8010BCF8u);
    PE_StoreU32(0x80011610u, 0x80120D00u);
    PE_StoreU16(0x8009315Eu, 0x039Fu);
    PE_StoreU16(0x80093160u, 0x03C5u);
    PE_StoreU16(0x80093162u, 0x03C9u);
    PE_StoreU32(0x800B0CD8u, 0x00100001u);
    PE_StoreU8(0x801D0E18u, 0xA5u);

    /* Entry sets 0x200, the tail's epilogue clears it. */
    ASSERT(func_80192CE8(47) == 0 && !PE_Port_ShouldStop(),
           "post-E08 media tail did not run to the epilogue");
    ASSERT((PE_LoadU32(0x800B0CD8u) & 0x200u) == 0u,
           "media-loop epilogue did not clear overlay bit 0x200");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE,
           "media tail requested a stop");

    PE_Disc_Close(disc);
    PASS();
}

/* Regression — func_8010C89C's CA7C zero-first-table escape arm.
 *
 * At CA7C the decoder looks the symbol up in the first table.  When that
 * word is zero it takes the CA98 escape: shift in eight more bits, look
 * up the *second* table at a3, and then shift v0 by the second word's low
 * byte.  CAD4 is the delay slot of the `b CADC` rejoin and re-reads
 * `at = t1 & 0xFF` from that second word; the pre-branch `at` belongs to
 * the zero first word and must not be reused.  The older transcription
 * kept the stale `at`, so those frames shifted v0 by 0 and desynchronised
 * the bitstream, emitting ~2x the declared RLE extent.
 *
 * FMV001 frame 319 (opening movie, LBA 192926..) drives this arm.  The
 * declared extent is 51076 bytes; the buggy decoder wrote 101510 bytes,
 * which overflowed the 0xFA00 RLE arena into the record pool at
 * A+0x1F400 and stalled the guest reader.  The canary past the declared
 * extent is the non-vacuous guard: the pre-fix decoder overwrites it. */
static void test_DAY2_c89c_second_table_escape(void)
{
    char err[256] = {0};
    uint8_t raw[2352];
    PE_Disc *disc;
    unsigned chunks = 0;
    uint32_t declared;

    TEST("DAY2_c89c_second_table_escape");
    disc = BTL6_OpenDisc1(err, sizeof(err));
    ASSERT(disc != NULL, err[0] ? err : "opening movie disc unavailable");

    ResetTestState();
    for (unsigned sector = 0; sector < 38u; sector++) {
        ASSERT(PE_Disc_ReadRawSector(disc, 1940u + sector, raw),
               "libpress sector read");
        memcpy(PE_Translate(0x8010BCF8u + sector * 2048u, 2048u),
               raw + 24, 2048u);
    }
    func_8010BD4C(0x80130000u, 0u);

    /* Assemble FMV001 frame 319 exactly as the production reader does:
     * the nine video chunks' 2016-byte bodies, skipping the interleaved
     * XA sector. */
    for (unsigned lba = 192926u; lba < 192926u + 32u && chunks < 9u; lba++) {
        ASSERT(PE_Disc_ReadRawSector(disc, lba, raw), "frame 319 sector read");
        if (!(raw[24] == 0x60u && raw[25] == 1u))
            continue;
        ASSERT((uint16_t)(raw[32] | (raw[33] << 8)) == 319u,
               "frame 319 chunk sequence");
        memcpy(PE_Translate(0x80150000u + chunks * 2016u, 2016u),
               raw + 56, 2016u);
        chunks++;
    }
    ASSERT(chunks == 9u, "frame 319 video chunk count");

    declared = ((PE_LoadU32(0x80150000u) & 0xFFFFu) << 2) + 4u;
    ASSERT(declared == 51076u, "frame 319 declared RLE extent");
    memset(PE_Translate(0x80160000u, declared + 64u), 0xCD, declared + 64u);
    ASSERT(func_8010C89C(0x80150000u, 0x80160000u, 0x80130000u, 0u) == 0 &&
           !PE_Port_ShouldStop(),
           "frame 319 VLC failed to terminate");
    ASSERT(PE_LoadU32(0x8011EBB4u) == 0x80160000u + declared,
           "frame 319 output bound differs");
    for (unsigned i = 0; i < 64u; i++)
        ASSERT(PE_LoadU8(0x80160000u + declared + i) == 0xCDu,
               "C89C wrote past the declared RLE extent");

    PE_Disc_Close(disc);
    PASS();
}

/* Regression — func_80191B64's C44 record-limit completion latch.
 *
 * Retail C44 is: if (w < D_801D11B0) latch D_801D0DBD; else if !(w <
 * rec[8]) fall through and latch too.  A stream frame number w therefore
 * sets the movie end-flag when it regresses below the last published
 * frame OR reaches/exceeds the record's frame limit.  The older
 * transcription only latched on the regression, so the opening movie
 * (record limit 2077) never completed and the reader ran past EOF.
 *
 * The second pass pins the true retail moment: w == record limit. */
static void test_DAY2_91b64_record_limit_latch(void)
{
    const pe_addr_t pool = 0x80150000u;
    const pe_addr_t rec  = 0x80151000u;

    TEST("DAY2_91b64_record_limit_latch");
    ResetTestState();
    DAY1_SeedDisplayDispatch();
    HostFB_Init();
    func_8007ED58();
    B558_PlantPointers();
    PE_GPU_Init();
    func_80073C94();
    B54KR_SeedGpuStatic();
    MOVAU_SeedRegisterPointers();

    PE_StoreU32(0x800C0DC8u, pool);
    PE_StoreU32(0x800C20C4u, 64u);
    PE_StoreU16(pool, 2u);          /* state 2 -> 7C484 got-frame      */
    PE_StoreU16(pool + 16u, 320u);
    PE_StoreU16(pool + 18u, 240u);
    PE_StoreU32(0x801D11ACu, rec);
    PE_StoreU16(rec + 8u, 200u);    /* rec[8] = record frame limit     */

    /* Below the limit: w >= lim (so the regression arm is skipped) but
     * w < rh, so retail leaves the latch clear. */
    PE_StoreU32(0x800BE9ECu, 0u);
    PE_StoreU32(0x800C0DBCu, 0u);
    PE_StoreU32(pool + 8u, 100u);   /* w                               */
    PE_StoreU16(0x801D11B0u, 99u);  /* lim                             */
    PE_StoreU8(0x801D0DBDu, 0u);
    (void)func_80191B64(0x801D1464u);
    ASSERT(!PE_Port_ShouldStop(), "91B64 stopped below the limit");
    ASSERT(PE_LoadU8(0x801D0DBDu) == 0u,
           "frame below the record limit latched the end flag");

    /* At the limit: the true retail completion moment. */
    PE_StoreU32(0x800BE9ECu, 0u);
    PE_StoreU32(0x800C0DBCu, 0u);
    PE_StoreU16(pool, 2u);
    PE_StoreU32(pool + 8u, 200u);   /* w == rh                         */
    PE_StoreU16(0x801D11B0u, 199u);
    PE_StoreU8(0x801D0DBDu, 0u);
    (void)func_80191B64(0x801D1464u);
    ASSERT(!PE_Port_ShouldStop(), "91B64 stopped on the limit pass");
    ASSERT(PE_LoadU8(0x801D0DBDu) == 1u,
           "record-limit frame did not latch the end flag");

    PASS();
}
