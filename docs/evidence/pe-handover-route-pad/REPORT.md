# PE-HANDOVER — `--route-pad` autopilot hand-over

Date: 2026-09-18

## Residual this closes

The port could autopilot Day 1 (`--route-pad`) or let you play from boot, but not
both. In `pc_port/src/port_main.c` the window path installs
`PE_Port_SetPadSource(HostWindow_PadRaw)` and the route path then overrides it
with `PE_Port_SetPadSource(RoutePadSource)`, and `RoutePadSource` returns the
route mask unconditionally — so the autopilot owned the pad for the whole run.
There was no "drive me to the fight, then give me the controller".

## Change

`pc_port/src/port_main.c` gains two options and a release gate:

```text
--hand-over-at <frame>   release the pad once the route frame reaches <frame>
--hand-over-key          release the pad when the player presses a button
                         (both together: wait for the frame gate, then the button)
```

`RouteHandOverReady()` is consulted first in `RoutePadSource`; from that point the
pad comes from `HostWindow_PadRaw()`. With no window open that is idle `0xFFFF`,
so a headless hand-over simply stops feeding route input instead of breaking.

## Verification (headless, retail disc)

```sh
./pc_port/build/parasite-eve-port --headless --route-pad --hand-over-at 52500 \
  --disc-image "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin" \
  --max-frames 52800 --screenshot /tmp/pe-2nd/handover.ppm
```

```text
[DISC] opened '...Parasite Eve (USA) (Disc 1).bin' (210685 user sectors, boot=SLUS_006.62)
[ROUTE] --route-pad: full Day-1/Day-2 pad sequence (3158 pairs) + sewer/M34 pilot
route: sewer victory room=1 frame=52111 HP=27
[ROUTE] hand-over at frame 52500 (--hand-over-at): pad is the player's
[FB] vsyncs=52803 drawsyncs=52942 presents=52800
[HOST] stop_reason=frame-limit
exit=0
```

The autopilot played Day 1 through the first Eve fight (m0023i) and the first
sewer fight, then released the pad at frame 52500 — token `m0027i` (`0xA80023C8`),
`victories=1`, `mode=9`. The captured 320x240 framebuffer is a live field frame:
the sewer corridor with Aya lit and the walls/ceiling light rendered.

## Route anchors used to choose the frame

Taken from the recorded pilot logs, so a hand-over frame can be picked without
guessing (`docs/generated/DISC1_GAMEPLAY_BASELINE.md`):

| beat | token / check | frame |
| --- | --- | --- |
| first sewer hallway | `m0027i 0xA80023C8` | ~52.3k |
| first sewer victory | `sewer victory room=1` | 52111 |
| second sewer hallway | `m0028i 0xA8002448` | ~53.0–53.5k |
| second sewer victory | `sewer victory room=2` | 53823 |

Suggested hand-over frames:

- `--hand-over-at 52500` — just after the first sewer fight (**verified**: token
  `m0027i`, `victories=1`; the frame shows the sewer corridor with Aya lit);
- `--hand-over-at 53400` — in `m0028i`, inside the second fight (**verified**:
  `route: sewer victory room=1 frame=52111` fired first, then
  `[ROUTE] hand-over at frame 53400 (--hand-over-at): pad is the player's`, with
  token `m0028i` `0xA8002448` at frames 53000/53500; the captured frame shows the
  live battle — the large sewer enemy plus two smaller ones, Aya's **HP 16/53**,
  the AT gauge and the PE bar);
- `--hand-over-key` — autopilot continuously and grab the pad whenever you like.

## Notes

- Windowed play needs a display: the port's backend is X11 (`dlopen` on libX11,
  `XOpenDisplay`). This verification box has no X11/Wayland server, hence headless.
- The hand-over does not touch guest state; it only changes which host source
  answers the pad poll.
- Run the native suite unchanged: `pc_port/src/port_main.c` is the CLI host and is
  not linked into `pe-native-tests`, so the 1404-case suite is unaffected.
