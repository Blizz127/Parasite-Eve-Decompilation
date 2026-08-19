# persist[0] Comprehensive Scan Report

## Executive Summary

Scanned **all 438 field table entries** (414 with valid scripts) for persist[0] accesses. Found:

- **173 total persist[0] accesses** (141 reads, 32 writes) across **25 scenes**
- **31 scenes** test bit 4 (0x4) via AND 0x4
- **1 scene** writes bit 4: **m0360i** (table_index 359, map_id 360)
- **0 scenes** explicitly clear bit 4 via AND 0xFFFFFFFB

## The Bit 4 (0x4) Writer: m0360i

**Scene**: m0360i, table_index 359, module 2

**Instruction sequence**:
```
+0x06CC: opcode 0x09 (ALU), subop 0x02 (OR)
         modes=[0, 3, 2, 0], args=[0x2, 0x1, 0x0, 0x4]
         Operation: cond[1] = persist[0] | 0x4
         
+0x06E4: opcode 0x0A (assign)
         modes=[2, 3], args=[0x0, 0x1]
         Operation: persist[0] = cond[1]
```

**Effect**: `persist[0] |= 0x4` — sets bit 4 while preserving all other bits.

**This is the ONLY scene in the entire game that sets bit 4 on persist[0].**

## persist[0] Bit Layout

| Bit | Mask | Set By | Cleared By | Tested By |
|-----|------|--------|------------|-----------|
| 0 | 0x1 | m0290i, m0431i | (indirect via AND ~0x1 in some scenes) | 30+ scenes |
| 1 | 0x2 | 18 scenes | 11 scenes (AND 0xFFFFFFFD) | 20+ scenes |
| 2 | 0x4 | **m0360i only** | Never explicitly cleared | 31 scenes |
| 3+ | 0x8+ | Unknown | Unknown | Unknown |

## persist[0] Writers (32 total)

All writes follow the pattern: `persist[0] = cond[X]` (opcode 0x0A, modes [2, 3]).

### By ALU Operation Feeding the Write:

**OR 0x2 (set bit 1)** — 18 scenes:
- m0001i, m0005i, m0010i, m0017i, m0031i, m0033i, m0041i, m0042i, m0044i, m0088i, m0117i, m0191i, m0260i, m0273i, m0319i, m0360i, m0372i, m0431i

**OR 0x4 (set bit 4)** — 1 scene:
- **m0360i** (the unique bit-4 writer)

**OR 0x1 (set bit 0)** — 2 scenes:
- m0290i, m0431i

**AND 0xFFFFFFFD (clear bit 1)** — 11 scenes:
- m0002i, m0004i, m0005i, m0017i, m0023i, m0041i, m0046i, m0092i, m0098i, m0319i

**AND 0xFFFFFFFE (clear bit 0)** — some scenes (included in AND ~0x1)

## Scenes That Test Bit 4 (0x4)

31 scenes use `AND 0x4` to test if bit 4 is set:
```
m0001i, m0005i, m0010i, m0017i, m0031i, m0033i, m0035i, m0037i, m0038i, 
m0039i, m0040i, m0041i, m0042i, m0043i, m0044i, m0047i, m0069i, m0088i, 
m0103i, m0108i, m0115i, m0117i, m0121i, m0191i, m0260i, m0273i, m0319i, 
m0359i, m0372i, m0374i, m0433i
```

## Why m0360i Was Missed

The original `pe_pst0_scan.py` only scanned 8 scenes (the "current route" + day-1 extras):
- m0001i, m0002i, m0003i, m0004i, m0005i, m0372i, m0377i, m0378i

m0360i is table_index 359 (map_id 360), which is NOT in that set. It was discovered by the comprehensive all-scene scan.

## Key Insight: Bit 4 is Write-Once

- Bit 4 is set by m0360i (OR 0x4)
- Bit 4 is NEVER explicitly cleared by any script (no AND 0xFFFFFFFB)
- Bit 4 is tested by 31 scenes (AND 0x4 reads)
- Once set, bit 4 persists until game reset (func_80034F10 zeroes all persist)

## Files

- `persist0_all_scenes.json` — full raw data from comprehensive scan
- `PERSIST0_COMPREHENSIVE_REPORT.md` — this report
- Original scanner: `tools/research/pe_pst0_scan.py`
- Comprehensive scanner: `tools/research/pe_pst0_all_scene_scan.py`
