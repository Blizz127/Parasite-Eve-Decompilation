# ASM function worklist slices (parent-generated)

Source: `build/asm_worklist.json` — 1680 non-matching functions / 611412 bytes.

Six disjoint, byte-balanced slices in rank order. Assign ONE slice per decompile agent;
never give two agents the same slice (the worklist is shared and reranked after each match, so
slice boundaries drift — treat the rank ranges as a guide and re-derive before each run).

| slice | funcs | bytes | rank range | first | last |
|---|---:|---:|---|---|---|
| 1 | 36 | 103000 | 1-36 | `func_8001D340` | `func_800D0728` |

| 2 | 73 | 102248 | 37-109 | `func_800D1384` | `func_80057094` |

| 3 | 124 | 102184 | 110-233 | `func_80038D74` | `func_800794C4` |

| 4 | 191 | 101924 | 234-424 | `func_80079754` | `func_80089D10` |

| 5 | 328 | 102092 | 425-752 | `func_800D8B6C` | `func_80085644` |

| 6 | 928 | 99964 | 753-1680 | `func_800C65E4` | `func_80089F50` |

