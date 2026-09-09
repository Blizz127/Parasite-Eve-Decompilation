# M0374I scene-start delivery

Stage 107 follows the M0039I E4 endpoint and map-selector result recorded in
DAY2_STATION_RETURN.md. This is a bounded original/native comparison, not
verification of the entire scene or its day/ending classification.

Original script SHA-256:
`01b08019e76adebf20aeee364cf042b503fa2e4472885824bde7b8098a2cf33d`.
`pc_port/tools/pe_m0374i_delivery_oracle.py` pins and loads the original field
chunk, uses the original executable VM and emits numeric digest fixtures for
`pc_port/tests/test_m0374i_delivery.h`. The original script is never rewritten.

The verified chain is:

1. Module 2 opcode14 at801C58BC installs the mailbox entry801C6354.
2. Module 0 sender tail801C54B8 queues `(type=2, subtype=0, payload=1)`;
   its task ends. Original65400 delivers to the supplied type2 actor.
3. Receiver6354 reads payload1 into local4; its predicate selects opcode12
   at6388, which forks module2 offset28A, entry801C5D54.
4. Mailbox task flags are4. Fork insertion checks only flags&3, so this
   child is inserted after the current task and executes during the same VM
   traversal. An optional pre-existing inactive task remains linked behind it.
5. Child5D54 writes storyE6 unconditionally, opens dialogue62, selects
   animationE, sets animation state via4E/2F and yields at30 at801C5D94.

The 64 original cases vary eight incoming story values (including E4/E6/E8
and signed extremes), four serial values (including FFFE/FFFF wrap), and the
presence of an inactive task behind the receiver. Three digests per case cover
supplied actors, task pool, queue/pool state, story, dialogue records, selected
animation globals, and the entire original chunk. This does not compare every
RAM byte or the original stack, nor prove all possible actor states.

Fixtures explicitly publish actors and task pool; the recipient is recordless
with no installed animation mesh. Its wait is a boundary, not evidence of
correct real-model animation duration or completion. Preceding dialogue and
movement that reach the sender tail are not executed. The model/mesh setup,
continuation beyond5D94, E8 alternatives, transfer toM0059I, and full scene/day
classification remain outstanding. Static inspection has not identified a
battle opcode in M0374I; adjacency does not establish combat or a day boundary.

Original artifact: `local/live/m0374i-delivery-107.json`. Native validation
normal and ASan/UBSan each pass all64 cases in the new group; full CTest
passes8/8 in78.57s. Oracle --check, Python and whitespace checks pass. No
runtime fix was required. Results are also recorded in ACTIVE_HANDOFF.md.
