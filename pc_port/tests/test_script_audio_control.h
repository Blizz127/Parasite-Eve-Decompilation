#include "retail_script_audio_control_cases.h"
static void test_DAY2_script_audio_control(void)
{
    TEST("DAY2_script_audio_control");
    for (unsigned k=0;k<sizeof(SAC_cases)/sizeof(SAC_cases[0]);k++) {
        uint64_t hash;
        ResetTestState();
        for (unsigned i=0;i<sizeof(SAC_common)/sizeof(SAC_common[0]);i++)
            PE_StoreU32(0x80000000u+SAC_common[i][0],SAC_common[i][1]);
        for (unsigned i=SAC_cases[k].first;i<SAC_cases[k].end;i++)
            PE_StoreU32(0x80000000u+SAC_patches[i][0],SAC_patches[i][1]);
        ASSERT(func_80015DAC_default_cut(0x80140200u)==1,"EA305/314 control completes");
        hash=hit_camera_hash(SAC_ranges,sizeof(SAC_ranges)/sizeof(SAC_ranges[0]));
        ASSERT(hash==SAC_cases[k].hash,"EA305/314 audio control and FIFO differ from original");
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"audio control completes natively");
    }
    PASS();
}
