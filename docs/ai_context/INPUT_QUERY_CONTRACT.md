# Original script opcode11 input query

DAY1/DAY2-66 restores selector3 in func_800130B4_port.c. Original source is
77 words130B4..131E8, SHA256
41ee2a1659cb0f8d1905ad71fb9f8b99408d6647c1a55127e789122ae7e25bbd,
from the pinned Disc1 executable. The existing native VM already dispatches
opcode11 to this function. Previously selector3 silently returned with no store.

Each argument is a pointer in the resolved argument bank. Selectors0,1,2 test
whether held8009D26C, newly pressed8009D1F4 or released8009D1E4 respectively
contain every mask bit. They write boolean0/1 to the third argument. Other
selectors except3 return1 without writing. Every path returns1.

Selector3 reads held flags then mask and writes0 to the output if any requested
bit is absent. That failure path leaves the mask and GTE registers untouched.
On success, ordered behavior is:

1. Write mask to GTE data register30 (LZCS), updating data31 (LZCR).
2. For mask80000000, choose index31 and leave the mask argument unchanged.
3. Otherwise store LZCR through the second argument pointer, reload that pointer
   and its value, and compute wrapped32-bit index31 minus the reloaded value.
4. Reload the output pointer, read800A7770+(index<<2) with wrapped32-bit
   addressing, and store the counter there.

LZCR counts leading bits equal to the sign bit. For positive masks the index
selects the highest set bit. For negative masks other than80000000 it selects
the highest zero bit. Zero andFFFFFFFF produce indexFFFFFFFF, reading800A776C
(one word before the32-counter table). These effects are preserved, without
clamping input or treating all negative values as bit31. The LZCR store is an
observable write to the script argument, not transient stack scratch. Reloads
also matter when arguments alias the pointer bank or destination.

PE_GTE_SetLZCS retains LZCS/LZCR in PeGteState while the existing PE_GTE_LZCR
remains a pure arithmetic helper. The query does not modify the GTE control
FLAG register. The oracle runner now accepts optional initial data registers,
allowing preservation checks on paths that do not execute mtc2. This does not
claim a complete GTE instruction interface or audit other users of the pure
arithmetic helper.

Reproduce with `python3 pc_port/tools/pe_input_query_oracle.py`; use
`--write-header` only to regenerate checked-in expected results. It executes
all original instructions for2560 cases, comparing RAM hashes and final GTE
state. Cases cover every one-bit mask, zero, all ones, mixed positive/negative
masks, hit/miss flags, selectors0..7, and overlapping destinations including
argument words, held state and counters. Some cases use the already-consumed
first argument pointer as the mask word. Expected results are generated from
original instruction execution, not the native translation.

Native `DAY1_input_query` checks those results, verifies unrelated GTE state
is retained, and runs selector3 through the VM with an immediate100 mask. The
script word changes to23, the output receives counter8 and the next opcode
executes. No live playthrough or full Day1/Day2 acceptance follows from this
handler-level evidence. M0351I's previously proved entry gate uses selector1;
this fix completes the previously omitted selector in the shared handler.

DAY1/DAY2-66 final regression:8/8 CTest targets pass in66.52s, with1266 native
groups. Both builds have no compiler warnings/errors; Python compilation and
scoped whitespace checks pass. All changes remain local.
