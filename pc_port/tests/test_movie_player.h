/* DAY2-157: complete movie player 121C04.
 * Original control-flow authority: pe_movie_player_oracle.py (20 graphs:
 * 4 early returns and 16 full setup/search/decode graphs with explicit
 * SDK/search/frame providers). The native test executes the real
 * SDK/disc implementations. The disabled-device run stops at the recorded
 * movie_player_search_wait boundary. The enabled-device run really
 * searches the fixture ISO, completes Setloc through the stage155 queue,
 * installs the 1214D4/7C214/813E8 streaming callbacks, and then stops at
 * the recorded CD_device_read_mode device boundary when the mode-0x1E0
 * ReadS reaches the device model (read modes with 0x50 bits are the
 * recorded XA-mode frontier; the autonomous stream delivery they would
 * start is the next integration frontier). */
static void MOVPLY_SeedRecord(unsigned id,unsigned wide)
{
    pe_addr_t record=0x80122438u+id*20u;
    PE_StoreU8(0x80122FF0u,'\\');PE_StoreU8(0x80122FF1u,0u);   /* prefix "\\" */
    PE_StoreU32(0x80120FF4u,0x80122FF0u);
    PE_StoreU32(0x80120FFCu,0x80122FF0u);
    strcpy(PE_Translate(0x80130130u,16u),"PE.IMG;1");
    PE_StoreU32(0x80122438u+id*20u,0x80130130u);   /* record[0] is the name */
    PE_StoreU8(record+4u,(uint8_t)wide);
    PE_StoreU16(record+6u,0x1234u);                /* stream end LBA */
    PE_StoreU16(record+0xAu,0x5678u);              /* slice x */
    PE_StoreU16(record+0xCu,0x300u);               /* slice y */
    PE_StoreU32(0x80122420u,0x80160000u);          /* RLE arenas */
    PE_StoreU32(0x80122424u,0x80164000u);
    PE_StoreU32(0x80122428u,0x80170000u);          /* output buffers */
    PE_StoreU32(0x8012242Cu,0x80174000u);
    PE_StoreU32(0x80122430u,0x80130000u);          /* VLC table */
    PE_StoreU32(0x80122434u,0x80150000u);          /* record pool */
    /* The player's BD4C RLE decode reads the libpress module source at
     * 0x8010CBFC; the module image is absent from this fixture, so seed
     * the FF FF terminator for a deterministic empty decode (the real
     * table build is covered by DAY2_movie_complete_frame). */
    PE_StoreU16(0x8010CBFCu,0xFFFFu);
    PE_StoreU32(0x8009CDDCu,0x100u);               /* display bank low byte 0 */
    PE_StoreU8(0x800B0DBEu,0x98u);
    PE_StoreU8(0x800B0DBAu,3u);
    PE_StoreU16(0x800B0DBCu,0u);
}

static void test_DAY2_movie_player(void)
{
    PeMdecState mdec;
    unsigned wide,k;

    TEST("DAY2_movie_player");
    /* Early returns: halfword id >= 0x2F returns 0 without touching B0DBF. */
    for(k=0u;k<4u;k++) {
        unsigned id=k==0u?0x2Fu:k==1u?0x30u:k==2u?0x8000012Fu:0xFFFFu;
        ResetTestState();HostFB_Init();PE_GPU_Init();
        PE_StoreU8(0x800B0DBFu,0xA5u);
        ASSERT(func_80121C04((int32_t)id)==0,"player early return");
        ASSERT(PE_LoadU8(0x800B0DBFu)==0xA5u &&
            !PE_Port_ShouldStop()&&!g_stub_order_count,"player early state");
    }
    /* Disabled device: the search readiness loop keeps the recorded
     * boundary visible before any search. */
    ResetTestState();HostFB_Init();PE_GPU_Init();
    MOVPLY_SeedRecord(0u,1u);
    ASSERT(func_80121C04(0)==0 && PE_Port_ShouldStop(),"player search stop");
    ASSERT(g_stub_order_count==1u &&
        strcmp(g_stub_order_log[0],"movie_player_search_wait")==0,
        "player search boundary identity");
    /* The setup prefix still ran: both display envs, B0DBF/B0DBB and
     * the record pointer are in place. */
    ASSERT(PE_LoadU8(0x800B0DBFu)==0u && PE_LoadU8(0x800B0DBBu)==1u &&
        PE_LoadU32(0x801227E4u)==0x80122438u,"player setup prefix state");
    /* Enabled device: real ISO search, Setloc and ReadS issue, then the
     * decode retry stops at the recorded boundary because the modeled
     * physical stream does not deliver state-2 records yet. */
    {
        DiscFixture fx={0};
        ResetTestState();
        ASSERT(FxBuild(&fx,0),"player fixture");
        PE_Disc_SetActive(fx.disc);CdDeviceSeed();
        ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,
            "player startup");
        for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u &&
            !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
        ASSERT(PE_LoadU32(0x8009B574u)==1u,"player SDK ready");
        MOVPLY_SeedRecord(0u,1u);
        /* Retail's SDK DMA register pointer for the stream handler's
         * MDEC-output busy check (StreamOutputChcr routes the physical
         * identity through pe_mdec's DMA1 owner). */
        PE_StoreU32(0x8009B34Cu,0x1F801098u);
        ASSERT(func_80121C04(0)==0 && PE_Port_ShouldStop(),
            "player read-mode stop");
        ASSERT(g_stub_order_count>=1u &&
            strcmp(g_stub_order_log[0],"CD_device_read_mode")==0,
            "player read-mode boundary identity");
        /* The search really resolved the fixture file and the start
         * Setloc really completed through the queue. */
        ASSERT(PE_LoadU32(0x801223FCu)!=0u && PE_LoadU32(0x80122414u)!=0u,
            "player search result missing");
        /* 81314 installed the streaming callbacks before the device
         * boundary. */
        ASSERT(PE_LoadU32(0x800B8AB4u)==0x800813E8u &&
            func_800824F0(0u)==0x8007C214u,
            "player stream callbacks");
        ASSERT(PE_LoadU8(0x800B0DBAu)==3u && PE_LoadU16(0x800B0DBCu)==0u,
            "player no early handoff");
        ASSERT(PE_LoadU8(0x801228D4u)==0u && PE_LoadU8(0x801228E0u)==0u &&
            PE_LoadU16(0x801228E2u)==0x5678u &&
            PE_LoadU16(0x801228E4u)==0x3F0u &&
            PE_LoadU16(0x801228F8u)==0x18u,"player slice geometry");
        PE_MDEC_GetState(&mdec);
        ASSERT(mdec.upload_count>=1u,"player libpress tables");
        FxFree(&fx);
    }
    (void)wide;
    PASS();
}
