#include "retail_pe_menu_cases.h"

static void test_INV19_retail_pe_menu(void)
{
    unsigned k,i,failures=0;
    TEST("INV19_retail_pe_menu");
    for (k=0;k<sizeof(INV19_pe_menu_cases)/sizeof(INV19_pe_menu_cases[0]);k++) {
        const uint32_t *args=INV19_pe_menu_cases[k].args;
        uint32_t a=args[0],result=0u;
        uint64_t hash;
        ResetTestState();func_80071A64(1u);
        for (i=0;i<sizeof(INV19_pe_menu_common)/sizeof(INV19_pe_menu_common[0]);i++)
            PE_StoreU32(0x80000000u+INV19_pe_menu_common[i][0],INV19_pe_menu_common[i][1]);
        for (i=INV19_pe_menu_cases[k].first;i<INV19_pe_menu_cases[k].end;i++)
            PE_StoreU32(0x80000000u+INV19_pe_menu_patches[i][0],INV19_pe_menu_patches[i][1]);
        D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
        D_8009D03C=PE_LoadU32(0x8009D03Cu);
        D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
        D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
        D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
        D_8009D250=PE_LoadU32(0x8009D250u);g_pe_gte.h=256;
        switch (INV19_pe_menu_cases[k].entry) {
        case 0:func_80055610();break;
        case 1:result=(uint32_t)func_8006346C(a);break;
        case 2:func_80046ABC(a);break;
        case 3:result=(uint32_t)func_80046B58(a,args[1]);break;
        case 4:func_80046C20();break;
        case 5:func_80057B70((int32_t)a);break;
        case 6:func_80046DBC(a,args[1]);break;
        case 7:result=(uint32_t)func_8004FC3C((int32_t)a);break;
        case 8:func_80050B48(a);break;
        case 9:func_80061044((int32_t)a,(int32_t)args[1]);break;
        case 10:func_8004FC80(a);break;
        case 11:func_800634D4(a,args[1],(int32_t)args[2],args[3]);break;
        case 12:func_80062FEC();break;
        case 13:result=(uint32_t)func_80044E98(a,args[1]);break;
        case 14:result=(uint32_t)func_80043DA4(a,args[1]);break;
        case 15:func_8005E30C();break;
        }
        PE_StoreU32(0x8009D018u,D_8009D018);
        PE_StoreU32(0x8009D1A0u,D_8009D1A0);
        PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
        PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
        hash=hit_camera_hash(INV19_pe_menu_ranges,sizeof(INV19_pe_menu_ranges)/sizeof(INV19_pe_menu_ranges[0]));
        if (hash!=INV19_pe_menu_cases[k].hash || result!=INV19_pe_menu_cases[k].result) {
            fprintf(stderr,"PE menu %u hash %016llX/%016llX result %08X/%08X\n",k,
                (unsigned long long)hash,(unsigned long long)INV19_pe_menu_cases[k].hash,
                result,INV19_pe_menu_cases[k].result);
            if (getenv("PE_INV19_DUMP")) {
                char path[100];FILE *out;
                snprintf(path,sizeof(path),"/tmp/pe-inv19-native-%u.bin",k);out=fopen(path,"wb");
                if (out) {for (i=0;i<0x200000u;i++) fputc(PE_LoadU8(0x80000000u+i),out);fclose(out);}
            }
        }
        if (hash!=INV19_pe_menu_cases[k].hash || result!=INV19_pe_menu_cases[k].result ||
            g_stub_order_count || PE_Port_ShouldStop()) failures++;
    }
    ASSERT(!failures,"PE menu original comparisons or native call graphs failed");
    PASS();
}
