#include "retail_cd_device_tables.h"
#include "retail_cd_startup_notify_cases.h"
static void CdDeviceSeed(void)
{
    HostFB_Init();PE_GPU_Init();PE_Callback_Init();B558_PlantPointers();
    PE_StoreU32(0x8009B290u,0x1F801C00u);
    for(unsigned j=0;j<6;j++) for(unsigned i=0;i<32;i++) PE_StoreU32(0x80000000u+CDDEV_tables[j].address+i*4u,CDDEV_tables[j].values[i]);
    for(unsigned i=0;i<26;i++) PE_StoreU32(0x80011D0Cu+i*4u,CDDEV_command_jumps[i]);
    for(unsigned i=0;i<5;i++) PE_StoreU32(0x80011B8Cu+i*4u,CDACK_jumps[i]);
}
static void test_DAY2_cd_device(void)
{
    TEST("DAY2_cd_device");
    for(unsigned k=0;k<sizeof(CDNOTIFY_cases)/sizeof(CDNOTIFY_cases[0]);k++) {
        ResetTestState();PE_StoreU32(0x800B8AB8u,CDNOTIFY_cases[k].target);
        func_8007F960(CDNOTIFY_cases[k].status,CDNOTIFY_cases[k].response);
        if(CDNOTIFY_cases[k].target) {
            ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_data_callback")==1 &&
                g_bootstrap_arg4_calls[0].target==CDNOTIFY_cases[k].target &&
                g_bootstrap_arg4_calls[0].arg0==CDNOTIFY_cases[k].arg0 &&
                g_bootstrap_arg4_calls[0].arg1==CDNOTIFY_cases[k].arg1,"startup notification differs from original callback prefix");
        } else ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"null startup notification must return");
    }
    DiscFixture fx={0};PeCdDeviceState device;
    ResetTestState();PE_Disc_SetActive(NULL);
    ASSERT(!PE_CdReg_EnableDevice(7u),"CD device accepted absent disc");
    ASSERT(FxBuild(&fx,0),"CD device fixture");PE_Disc_SetActive(fx.disc);
    CdDeviceSeed();ASSERT(PE_CdReg_EnableDevice(7u),"CD device attachment");
    ASSERT(func_8007EC14()==1 && !PE_Port_ShouldStop(),"real CD startup did not complete");
    PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==3u && device.responses==4u && device.command_log[0]==1u && device.command_log[1]==10u && device.command_log[2]==12u,"startup command sequence");
    ASSERT(device.response_log[0]==3u && device.response_log[1]==3u && device.response_log[2]==2u && device.response_log[3]==3u,"startup response ordering");
    ASSERT(PE_LoadU8(0x8009B294u)==2u && PE_LoadU32(0x800945F0u)==0x8007C13Cu && PE_LoadU32(0x8009568Cu)==0x8007FE24u,"startup callback/status publication");
    ASSERT(PE_LoadU32(0x800A36ACu)==0x8007F960u && PE_LoadU32(0x800A36A0u)==0x8007F7E8u,"public CD handlers");
    /* A masked VBlank advances time but cannot invoke the SDK updater. */
    uint16_t mask=PE_IRQ_GetMask();
    (void)PE_IRQ_ExchangeMask((uint16_t)(mask&~1u));
    uint32_t vblanks=PE_LoadU32(0x800956ACu);
    HostFB_VSync(0);PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==3u && PE_LoadU32(0x800956ACu)==vblanks && (PE_IRQ_ReadStatus()&1u),"masked VBlank dispatched startup");
    (void)PE_IRQ_ExchangeMask(mask);
    for(unsigned tick=0;tick<10u && PE_LoadU32(0x8009B574u)!=1u && !PE_Port_ShouldStop();tick++) HostFB_VSync(0);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x800956ACu)>vblanks,"host VBlank startup did not dispatch");
    ASSERT(PE_LoadU32(0x8009B574u)==1u && PE_LoadU32(0x8009B578u)==11u,"CD startup state machine did not become ready");
    PE_CdReg_GetDeviceState(&device);
    ASSERT(device.commands==7u && device.command_log[3]==1u && device.command_log[4]==1u && device.command_log[5]==19u && device.command_log[6]==1u,"startup VBlank command progression");
    /* Transport IRQ masking and Init's held second response. */
    PE_CdReg_Reset();(void)PE_IRQ_ExchangeMask(0u);PE_IRQ_WriteStatus(0u);
    ASSERT(PE_CdReg_EnableDevice(0u),"device masked attachment");
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,10u);
    PE_CdReg_ServiceDevice(0x13CCDu);
    ASSERT(!(PE_CdReg_ReadU8(0x1F801800u)&0x20u),"command responded before scheduled cycle");
    PE_CdReg_ServiceDevice(1u);ASSERT(!(PE_IRQ_ReadStatus()&4u),"CD IRQ ignored device mask");
    PE_CdReg_WriteU8(0x1F801800u,1u);ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==3u,"Init first tag");
    PE_CdReg_WriteU8(0x1F801802u,7u);PE_CdReg_ServiceDevice(0u);
    ASSERT(PE_IRQ_ReadStatus()&4u,"pending CD response did not assert after enabling mask");
    PE_CdReg_ServiceDevice(0x200000u);PE_CdReg_GetDeviceState(&device);
    ASSERT(device.responses==1u,"second response overwrote unacknowledged first");
    ASSERT(PE_CdReg_ReadU8(0x1F801801u)==2u,"Init first status byte");
    PE_CdReg_WriteU8(0x1F801803u,7u);PE_CdReg_ServiceDevice(0u);
    ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==2u && PE_CdReg_ReadU8(0x1F801801u)==2u,"Init completion tag/status");
    PE_CdReg_WriteU8(0x1F801803u,7u);
    /* SetMode and Getparam transfer parameters through actual FIFOs. */
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801802u,0xA0u);PE_CdReg_WriteU8(0x1F801801u,14u);PE_CdReg_ServiceDevice(0xC4E1u);
    PE_CdReg_WriteU8(0x1F801800u,1u);PE_CdReg_WriteU8(0x1F801803u,7u);
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,15u);PE_CdReg_ServiceDevice(0xC4E1u);
    ASSERT(PE_CdReg_ReadU8(0x1F801801u)==2u && PE_CdReg_ReadU8(0x1F801801u)==0xA0u,"mode parameter round trip");
    PE_CdReg_WriteU8(0x1F801800u,1u);PE_CdReg_WriteU8(0x1F801803u,7u);
    /* Bad parameter counts are device errors, unported commands are boundaries. */
    PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,14u);PE_CdReg_ServiceDevice(0xC4E1u);
    PE_CdReg_WriteU8(0x1F801800u,1u);
    ASSERT((PE_CdReg_ReadU8(0x1F801803u)&7u)==5u && PE_CdReg_ReadU8(0x1F801801u)==3u && PE_CdReg_ReadU8(0x1F801801u)==0x20u,"bad parameter error response");
    PE_CdReg_WriteU8(0x1F801803u,7u);PE_CdReg_WriteU8(0x1F801800u,0u);PE_CdReg_WriteU8(0x1F801801u,3u);
    ASSERT(PE_Port_ShouldStop() && CountOrderLog("CD_device_unported_command")==1,"unported Play must stop");
    PE_CdReg_Reset();PE_Disc_SetActive(NULL);FxFree(&fx);PASS();
}
