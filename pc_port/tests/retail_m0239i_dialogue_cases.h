/* Original M0239I first-page dialogue, requires original disc. */
#define M239D_LBA 63439u
#define M239D_SECTORS 183u
static const uint32_t M239D_ranges[][2]={
{0x9CE90u,72u},
{0x9EC70u,112u},
{0xBCEA8u,224u},
{0x150000u,9216u},
{0x155000u,16u},
};
static const struct { unsigned message,bank,background; uint64_t hash; } M239D_cases[]={
{124,0,0,UINT64_C(0xAD104A17252547CA)},
{124,0,1,UINT64_C(0xF7B1AD1EC5E04BBE)},
{124,1,0,UINT64_C(0xAD104A17252547CA)},
{124,1,1,UINT64_C(0x4655AC96DF10FE0E)},
{125,0,0,UINT64_C(0xE502DC63786676A5)},
{125,0,1,UINT64_C(0xAF263A6ADF60CA19)},
{125,1,0,UINT64_C(0xE502DC63786676A5)},
{125,1,1,UINT64_C(0xF45B28B350ED0D89)},
};
