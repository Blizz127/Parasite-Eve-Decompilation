/* Original complete STR-frame VLC output; COP0 status providers, no pixel oracle. */
static const uint64_t MOVCOMPLETE_table_hash=UINT64_C(0x62FD6458F3443095);
static const struct { uint32_t first,last,chunks,frame,size,width,height,command,length,consumed; uint64_t input_hash,output_hash; } MOVCOMPLETE_cases[]={
{189742u,189751u,9u,1u,2712u,320u,240u,939525920u,7300u,2712u,UINT64_C(0x0D17397B6FEAB122),UINT64_C(0xAAED494150FD7400)},
{189752u,189761u,9u,2u,3628u,320u,240u,939526880u,11140u,3628u,UINT64_C(0xF09B47B9B96C09B2),UINT64_C(0xFD0055D1719EFF3E)},
{189762u,189771u,9u,3u,4356u,320u,240u,939527296u,12804u,4356u,UINT64_C(0xAC1A0AC85CD193BB),UINT64_C(0x6735AC03221EDBB5)},
};
