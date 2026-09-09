# Late park routes and movie-loader contract

Stage132 continues the verified130/131 frontier. Reproduce route evidence with
`python3 pc_port/tools/pe_day2_park_late_routes.py`:4030 original-handler paths,
19 pinned scripts and51 static transfers. Source executable SHA1 and per-script
SHA256 checks precede execution. JSON is in
`local/live/day2-park-late-routes-132.json`. This includes previously examined
58/351/367 and candidate90/91, so19 is not a count of newly decompiled rooms.

| Script | Immediate destinations, in decoded order |
| --- | --- |
| 71 | 69 |
| 77 | 76,80 |
| 78 | 80,79,76 |
| 79 | 76,78,81 |
| 80 | 77,78 |
| 81 | 82,79 |
| 82 | 81,83 |
| 83 | 82,84,84,84,85,85 |
| 84 | 83,83,83,85 |
| 85 | 83,83,84,87 |
| 86 | 87,88 |
| 87 | 85,86 |
| 88 | 89,86,89 |
| 89 | 92,367 |
| 58 | 41,0,59 |
| 351 | 42,92 |
| 367 | 5,319,239,58,35,136 |
| 90 (candidate) | 136 |
| 91 (candidate) | 351 |

Adjacency is not story reachability. The executable decoder also reads some
trailing data; its command counts are not semantic completion counts. Every
listed transfer is pinned by both relocated command address and destination.

Original-handler checks establish these closed portions:

- 71 module1 selects music18 when signed story>=F0, otherwise music-reset217.
 Its final block writes storyF0 and previous-room71 before requesting69.
 Module2 independently writesF0 before its fade/wait and component publication.
- 88 module2's camera gate requires story==118. Module3 publishes component2
 when signed story<118 and component3 when story==118. Gates stop before the
 component calls. False paths land at801CD320 (position query) and801CD3E4
 (delay), respectively, not at the nearby20 commands. Original execution
 rejected those initially mistaken stop addresses.
- 88 module0 writes118; module1 writes110 and previous-room88 before89. These
 are separate scene paths, not a demonstrated110→118 sequence.
- 89 module0 at801DE360 skips to component publication801DE560 when persist0
 bit2 is set; otherwise it enters the scene at801DE394. After the receiver,
 the same flag at801DE5C4 selects the music/fade path801DE5EC when set and
 the continued scene801DE66C otherwise. The former path's final block writes
 story140 and previous-room91 before requesting92. The latter has a separate
 module2 endpoint writing11B and previous-room89 before audio-stop and367.
- 367's previously verified11B selector reaches its11B scene. That scene ends
 with11C and destination58. Script58 later writes120 before fade/reset and41.
 These endpoints are individually executed; intervening scenes remain open.
- 90 has a signed story<160 gate and consecutive writes130 then134 at801C2E00
 and801C2E10. Both stores execute with no yield between them; final story is134.
- 91 writes138 and previous-room91 before351. The351 final selector converts
 exactly138 to140 and requests92. The connection from the traced park return
 through90/91 is not yet established. Do not call this a proven Day2 ending.

Movie requests on this graph include71:8 at801C25C8,88:9 at801CCBFC and89:10
at801E2CB4. Native17018 currently records a HOST_ADAPTED skip and returns1
when `--skip-movie` is enabled; otherwise14E30 is an explicit unresolved stop.
The skip path is not evidence of retail movie behavior or completed days.

## Original movie loader14E30

Authority: `asm/disc1/3420.s`,106 words14E30..14FD8; SHA256
`3e11c6977f606fbb2b3ecf6cbce00a02ba1c6721419914bb01a2e791f84cd4a5`.
`python3 pc_port/tools/pe_movie_loader_contract.py` verifies42 cases against
original instructions; JSON in `local/live/movie-loader-contract-132.json`.
This is a decompiled call contract, not a native implementation:

```text
stream_flags[B0CD8] |= 0x8200;
74DC0(0); 73A44(0); 74D28(0); 74A44(1);
for each (table,destination_global) in
    [(9315E,1160C), (93160,11610)]:
  restart:
    repeat:
      first = u16(table); end = u16(table+2);
      result = 6E6A8(u32(B0DD8)+first, u32(destination_global), end-first);
    until result != -1;
    repeat:
      status = 6E7E8();
      if status == -1: goto restart;
    until status == 0;
    72714(); 726C4(); 72724();
descriptor = [u32(11610) + ((u16(93162)-u16(93160)) << 11), 0];
1216C4(1, &descriptor);
121C04(s16(*args[0]));
1223A8(1);
return 1;
```

The loader re-reads table/destination/base values on every issue attempt.
Other nonzero completion statuses keep polling; only-1 reissues the package.
The selection operand is signed16-bit, not an unsigned movie index. Tests
check the flag prefix, both issue argument windows, both issue-retry and completion branches,
stack descriptor and signed selection. Unselected completion boundaries are
invalid-instruction guards; expected boundaries stop before executing providers.
SDK/disc functions, executable overlay loading, overlay1216C4/121C04/1223A8,
cache behavior, movie decoding, playback and display restoration are not run.
No claims about full original loader execution follow from these42 cases.

Next work: restore the movie overlay graph and real playback; establish the
post120 route through original scene/native callees; execute late-park battles,
optional routes and complete scenes. Day1 full retail acceptance, all Day2
semantics and native end-to-end play remain unfinished. Stage132 changes
research tools/docs only;128 remains the last full native regression/release.
