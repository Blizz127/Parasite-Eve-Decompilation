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
