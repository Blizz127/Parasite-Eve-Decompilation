#include "retail_card_status_cases.h"
static void test_DAY1_card_status(void)
{
    TEST("DAY1_card_status");
    static const uint32_t ranges[][2]={{0xA0ED4,0xA00},{0x9D154,16}};
    static const uint8_t states[]={0,1,2,3,4,5,255,3},flags[]={0,1,4,255},operations[]={0,12,8};
    static const uint32_t timers[]={0,1,2,0xFFFFFFFFu,0x80000000u,0x7FFFFFFFu,0x80000001u,99};
    for(unsigned n=0;n<4096;n++) {
        ResetTestState();
        /* Register the real card events (mode 1000h) the way func_800409B4
         * does, so the empty-slot kernel completion executes its callback
         * and latches the status flag the machine polls. */
        PE_Card_OpenEvents();
        for(unsigned i=0;i<0xA00;i++)PE_StoreU8(0x800A0ED4u+i,0xA5);
        unsigned index=n/2048;pe_addr_t record=0x800A0ED4u+index*0x418u;
        PE_StoreU8(record,flags[n/512%4]);PE_StoreU8(record+8u,states[n%8]);PE_StoreU8(record+1u,operations[n/32%3]);
        unsigned mask=n/8%64;
        for(unsigned i=0;i<6;i++)PE_StoreU32(0x800A1820u+4u*i,mask&(1u<<i)?0x12345678u:0u);
        PE_StoreU32(0x800A1838u,n/128%2);PE_StoreU32(0x800A183Cu,n/64%3);PE_StoreU32(0x800A1840u,timers[n/256%8]);
        PE_StoreU32(0x8009D154u,n/16%2?0x80158000u:0u);PE_StoreU32(0x8009D158u,0x12345678u);
        PE_StoreU32(0x8009D15Cu,0x80159000u);PE_StoreU32(0x8009D160u,0xDEADBEEFu);
        PE_StoreU32(0x80158000u,0u);PE_StoreU32(0x80158020u,1u);PE_StoreU32(0x80158024u,36u);
        for(unsigned i=0;i<8;i++)PE_StoreU32(0x800BCDA8u+4u*i,0xF1000000u+i);
        func_800405A4(index);
        uint64_t hash=hit_camera_hash(ranges,2);
        if(hash!=DAY1_card_status_cases[n].hash)fprintf(stderr,"card status %u differs\n",n);
        ASSERT(hash==DAY1_card_status_cases[n].hash,"card status effects differ from original");
        ASSERT((unsigned)PE_Port_ShouldStop()==DAY1_card_status_cases[n].stopped,"card status stop differs");
        if(DAY1_card_status_cases[n].stopped) {
            ASSERT(g_bootstrap_arg4_call_count==1 &&
                   g_bootstrap_arg4_calls[0].target==DAY1_card_status_cases[n].target &&
                   g_bootstrap_arg4_calls[0].arg0==DAY1_card_status_cases[n].argument,"card status BIOS frontier differs");
        } else ASSERT(!g_stub_order_count,"returning card status path logged a stub");
    }
    PASS();
}

/* Card-present kernel model: a host-backed raw 128 KiB image, formatted to
 * the psx-spx layout on first use.  Verifies the header/directory checksums,
 * the three kernel operations' success events, the psx-spx sector validation,
 * and that _card_write persists the 128-byte frame to the image. */
static void test_DAY1_card_present_kernel(void)
{
    static const char *path = "/tmp/pe_card_present_test.mcr";
    uint8_t frame[0x80];
    FILE *f;
    uint8_t sum;
    unsigned i;

    TEST("DAY1_card_present_kernel");
    ResetTestState();
    remove(path);
    setenv("PE_CARD_IMAGE", path, 1);
    PE_Card_Reset();
    PE_Card_OpenEvents();

    ASSERT(PE_Card_IsPresent() == 1, "present card must resolve");

    f = fopen(path, "rb");
    ASSERT(f != NULL, "present model must write the image file");
    memset(frame, 0, sizeof frame);
    ASSERT(fread(frame, 1, sizeof frame, f) == sizeof frame, "header read");
    ASSERT(frame[0] == 'M' && frame[1] == 'C', "header id must be MC");
    sum = 0; for (i = 0; i < 0x7Fu; i++) sum ^= frame[i];
    ASSERT(frame[0x7F] == sum, "header checksum must be the frame XOR");
    ASSERT(fseek(f, 0x80, SEEK_SET) == 0, "seek to directory frame 1");
    ASSERT(fread(frame, 1, sizeof frame, f) == sizeof frame, "dir read");
    ASSERT(frame[0] == 0xA0u, "formatted directory entry must be free (A0h)");
    sum = 0; for (i = 0; i < 0x7Fu; i++) sum ^= frame[i];
    ASSERT(frame[0x7F] == sum, "directory checksum must be the frame XOR");
    fclose(f);

    /* _card_info / _card_load: done-okay event F4000001h,0004h -> A1820. */
    PE_StoreU32(0x800A1820u, 0);
    ASSERT(func_8007DD44(0) == 1 && PE_LoadU32(0x800A1820u) == 1u,
           "_card_info must report done-okay");
    PE_StoreU32(0x800A1820u, 0);
    ASSERT(func_8007DD54(0) == 1 && PE_LoadU32(0x800A1820u) == 1u,
           "_card_load must report done-okay");

    /* _card_write: finished-okay event F0000011h,0004h -> A182C, and the
     * 128-byte frame is persisted at sector*128. */
    for (i = 0; i < 0x80u; i++)
        PE_StoreU8(0x80150000u + i, (uint8_t)(0x40u + i));
    PE_StoreU32(0x800A182Cu, 0);
    ASSERT(func_8007DDB4(0, 0x3F, 0x80150000u) == 1 &&
           PE_LoadU32(0x800A182Cu) == 1u,
           "_card_write must report finished-okay");
    f = fopen(path, "rb");
    ASSERT(f != NULL, "image reopen");
    ASSERT(fseek(f, 0x3F * 0x80, SEEK_SET) == 0, "seek to written sector");
    ASSERT(fread(frame, 1, sizeof frame, f) == sizeof frame, "written read");
    fclose(f);
    for (i = 0; i < 0x80u; i++)
        if (frame[i] != (uint8_t)(0x40u + i)) break;
    ASSERT(i == 0x80u, "_card_write must copy the 128-byte frame to the image");

    /* psx-spx sector validation: 0..3FFh valid, 400h accepted quirk, else 0. */
    ASSERT(func_8007DDB4(0, 0x400, 0) == 1, "sector 400h accepted (quirk)");
    ASSERT(func_8007DDB4(0, 0x401, 0) == 0, "sector 401h rejected");

    remove(path);
    PASS();
}

/* libcard file API: create/write/close then firstfile/nextfile and
 * open(read)/read must round-trip the same bytes and size. */
static void test_DAY1_card_file_api(void)
{
    static const char *path = "/tmp/pe_card_file_api_test.mcr";
    static const char *fname = "BASLUS-006620000000A";
    uint8_t in[200];
    pe_addr_t name = 0x80150000u, buf = 0x80151000u, dirent = 0x80152000u;
    int fd, i;

    TEST("DAY1_card_file_api");
    ResetTestState();
    remove(path);
    setenv("PE_CARD_IMAGE", path, 1);
    PE_Card_Reset();
    PE_Card_SetPresent(1);

    ASSERT(func_80072784(0) == 1, "format must succeed on a present card");
    for (i = 0; i <= 20; i++)
        PE_StoreU8(name + (pe_addr_t)i, (uint8_t)fname[i]);

    fd = func_80072734(name, 0x200);
    ASSERT(fd >= 0, "open(create) must return a handle");
    for (i = 0; i < 200; i++) {
        in[i] = (uint8_t)(i * 7 + 3);
        PE_StoreU8(buf + (pe_addr_t)i, in[i]);
    }
    ASSERT(func_80072764(fd, buf, 200) == 200, "write must store all 200 bytes");
    ASSERT(func_80072774(fd) == 0, "close(write) must succeed");

    ASSERT(func_800727B4(0, dirent) == dirent,
           "firstfile must return the created file");
    for (i = 0; i < 20; i++)
        if (PE_LoadU8(dirent + (pe_addr_t)i) != (uint8_t)fname[i]) break;
    ASSERT(i == 20, "firstfile name must match");
    ASSERT(PE_LoadU32(dirent + 0x18u) == 200u, "firstfile size must be 200");
    ASSERT(func_80072794(dirent) == 0, "nextfile must end after one file");

    fd = func_80072734(name, 1);
    ASSERT(fd >= 0, "open(read) must return a handle");
    ASSERT(func_80072754(fd, buf, 200) == 200, "read must return all 200 bytes");
    for (i = 0; i < 200; i++)
        if (PE_LoadU8(buf + (pe_addr_t)i) != in[i]) break;
    ASSERT(i == 200, "read data must round-trip");
    ASSERT(func_80072774(fd) == 0, "close(read) must succeed");

    remove(path);
    PE_Card_Reset();   /* re-resolve from the default image for later tests */
    PASS();
}
