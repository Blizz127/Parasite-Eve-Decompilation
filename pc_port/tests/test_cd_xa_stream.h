/* DAY2-158: CdlModeRT device coverage + autonomous stream publication.
 * Provenance: D_8009B32C..D_8009B35C are EXE .data initializers (same
 * family as B27C), planted by B558_PlantPointers / CdDeviceSeed — not a
 * CdInit or movie-path runtime writer. With bit6 allowed, ReadS mode
 * 0xE0 delivers sectors through the real IRQ→7C13C→7F88C→813E8→7C564
 * chain and 7C214 finalizes status 2 without a manual pump. */
static void test_DAY2_cd_xa_stream(void)
{
    TEST("DAY2_cd_xa_stream");
    DiscFixture fx={0};
    ASSERT(FxBuild(&fx,0),"XA stream fixture");
    ResetTestState();PE_Disc_SetActive(fx.disc);CdDeviceSeed();
    uint8_t *raw=fx.img+20u*2352u;
    for(unsigned i=0;i<2352u;i++) raw[i]=(uint8_t)(i*13u+(i>>8u));
    for(unsigned i=0;i<32u;i++) raw[24u+i]=0;
    /* STR header at user-data start (Size1 FIFO offset 12 → DMA from +24). */
    raw[24]=0x60;raw[25]=1;raw[30]=1;raw[32]=1;
    ASSERT(PE_CdReg_EnableDevice(7u) && func_8007EC14()==1,"XA stream startup");
    for(unsigned i=0;i<10u && PE_LoadU32(0x8009B574u)!=1u && !PE_Port_ShouldStop();i++)
        HostFB_VSync(0);
    ASSERT(PE_LoadU32(0x8009B574u)==1u,"XA stream SDK ready");
    /* EXE-planted DMA/CD pointers must be live before the stream path. */
    ASSERT(PE_LoadU32(0x8009B32Cu)==0x1F801800u &&
        PE_LoadU32(0x8009B334u)==0x1F801802u &&
        PE_LoadU32(0x8009B338u)==0x1F801803u &&
        PE_LoadU32(0x8009B33Cu)==0x1F801018u &&
        PE_LoadU32(0x8009B340u)==0x1F801020u &&
        PE_LoadU32(0x8009B344u)==0x1F8010F0u &&
        PE_LoadU32(0x8009B348u)==0x1F8010F4u &&
        PE_LoadU32(0x8009B34Cu)==0x1F801098u &&
        PE_LoadU32(0x8009B35Cu)==0x1F8010B8u,
        "EXE stream pointer table missing after CdDeviceSeed");
    PE_StoreU32(0x80140000u,0x00200200u);
    ASSERT(func_8007FB44(2u,0x80140000u)==1,"XA Setloc");
    for(unsigned i=0;i<2048u && PE_LoadU32(0x8009B598u) && !PE_Port_ShouldStop();i++)
        HostFB_VSync(-1);
    ASSERT(!PE_Port_ShouldStop(),"XA Setloc completion");
    func_8007A214(0x80150000u,8u);
    if(PE_Port_ShouldStop()) {FxFree(&fx);ASSERT(0,"XA StSetRing");}
    PE_StoreU32(0x800A801Cu,1u);
    ASSERT(func_80081314(0x80140000u,0x1E0u)!=0,"XA ReadS stream open");
    ASSERT(PE_LoadU32(0x800B8AB4u)==0x800813E8u &&
        func_800824F0(0u)==0x8007C214u,"XA stream callbacks");
    /* Restore DMA slot after the probe read of 824F0. */
    (void)func_800824F0(0x8007C214u);
    for(unsigned i=0;i<4096u && PE_LoadU16(0x80150000u)!=2u && !PE_Port_ShouldStop();i++)
        HostFB_VSync(-1);
    ASSERT(!PE_Port_ShouldStop(),"XA autonomous stream stopped");
    ASSERT(PE_LoadU16(0x80150000u)==2u && PE_LoadU32(0x800BE9E4u)==1u &&
        !PE_LoadU32(0x800B89F4u),"XA autonomous status-2 publication");
    for(unsigned i=0;i<4u;i++)
        ASSERT(PE_LoadU8(0x8015001Cu+i)==raw[12u+i],"XA autonomous location");
    for(unsigned i=0;i<2016u;i++)
        ASSERT(PE_LoadU8(0x80150100u+i)==raw[56u+i],"XA autonomous payload");
    PeCdDeviceState state;PE_CdReg_GetDeviceState(&state);
    ASSERT(state.mode==0xE0u && state.sectors>=1u,"XA device mode/sectors");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}
