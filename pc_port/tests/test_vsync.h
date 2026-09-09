#include "pe_vsync.h"
#include "retail_vsync_cases.h"
typedef struct {uint32_t next,end,repeats;int failed;} TestVSyncTrace;
static uint32_t test_vsync_event(TestVSyncTrace *t,uint32_t kind,uint32_t address,uint32_t value,uint32_t extra)
{
    if(t->next>=t->end)goto fail;
    const uint32_t *e=DAY1_vsync_events[t->next];
    if(e[1]!=kind || e[2]!=address || (kind && (e[3]!=value || e[4]!=extra)))goto fail;
    uint32_t result=e[3];
    if(++t->repeats==e[0]){t->next++;t->repeats=0;}
    return result;
 fail:
    if(!t->failed)fprintf(stderr,"VSync event%u kind%u address%X value%X\n",t->next,kind,address,value);
    t->failed=1;PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
}
static uint32_t test_vsync_read(void *p,pe_addr_t a){return test_vsync_event(p,0,a,0,0);}
static void test_vsync_write(void *p,pe_addr_t a,uint32_t v){(void)test_vsync_event(p,1,a,v,0);}
static void test_vsync_bios(void *p,uint32_t table,uint32_t service,uint32_t a,uint32_t b)
{(void)test_vsync_event(p,2,(table<<8)|service,a,table==0xC0u?b:0);}
static void test_DAY1_vsync(void)
{
    TEST("DAY1_vsync");
    for(unsigned i=0;i<sizeof(DAY1_vsync_cases)/sizeof(DAY1_vsync_cases[0]);i++) {
        ResetTestState();
        TestVSyncTrace trace={DAY1_vsync_cases[i].first,DAY1_vsync_cases[i].end,0,0};
        PeVSyncClock clock={&trace,test_vsync_read,test_vsync_write,test_vsync_bios};
        uint32_t result=PE_RetailVSync(DAY1_vsync_cases[i].mode,&clock);
        if(trace.failed || result!=DAY1_vsync_cases[i].result)fprintf(stderr,"VSync case%u mode%d result%X/%X\n",i,DAY1_vsync_cases[i].mode,result,DAY1_vsync_cases[i].result);
        ASSERT(!trace.failed && trace.next==trace.end && !trace.repeats,"VSync device/BIOS transcript differs from original");
        ASSERT(result==DAY1_vsync_cases[i].result,"VSync return differs from original");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"VSync stopped or hit stub");
    }
    PASS();
}
