# Park scene flags and dialogue background

Stage111 follows the six-script inventory in DAY2_EXTENDED_ROUTES.md.
M0239I contains repeated ED2300 (`0x8FC`) commands around dialogues, including
801D4D50/801D4D90 around message7C. It also uses ED2101 (`0x835`) before two
transfers. Both keys were silently ignored by the native16910 handler.

Original ED2300 at16ABC calls375D0 after converting arg1 to a boolean.
375D0 stores the boolean word at9CED4. Original37870 and the native renderer
use that word to include the dialogue background independently of active
message records. The native handler now preserves this behavior, including
nonzero values whose low byte is zero. Original ED2101 at16A44 ORs overlay
B0CD8 with800; its native handler now performs that store. The downstream
meaning of that scene flag is not inferred from this isolated write.

`pe_dialogue_background_oracle.py` executes original371B0 initialization,
ED2101/2300 and complete37870 with no active message records. Eighty cases
cover two packet banks, zero/nonzero/upper-bit arguments and existing overlay
flags. Two digests distinguish the flag change from renderer packet linkage.
Native expectations are in retail_dialogue_background_cases.h and consumed
by test_dialogue_background.h. This proves flag/background packet behavior,
not the entire renderer or visible GPU output.

Additional real-text coverage uses original M0239I script SHA-256
`f3037093dd6eff7a5edaf7678253abf354237e79057be341af9455e897b15030`.
The room's text-directory id1 publishes text base801E465C. The original
pe_m0239i_dialogue_oracle.py runs371B0,375E0 for messages7C/7D, ED2300 and37870
through their first-page state2, in both packet banks with background on/off.
Model/scene tasks and confirmation input are not part of this fixture. Original
text remains disc-backed and is not copied into expectation headers.
Native comparison is in test_m0239i_dialogue.h; status is in ACTIVE_HANDOFF.md.

All checks concern specified first pages and packet state. Page confirmation,
scene continuation, full text/control coverage, GPU output and Day1/Day2-wide
acceptance remain unfinished.

Validation:80 synthetic flag/background graphs and8 real first-page graphs
pass original/native comparisons and ASan/UBSan. Final full CTest passes8/8
in84.80s. Oracle regeneration, Python and whitespace checks pass. Native app
rebuilt; all builds warning-free.
