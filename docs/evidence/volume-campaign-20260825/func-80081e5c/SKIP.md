# func_80081E5C — refreshed volume screen — address-retention twin

Outcome: SKIP-PARKED-ADDRESS-RETENTION-FAMILY. No matching-C attempt or leaf claim; count remains 289.

Retail span [0x7265C,0x72670), five words: lui/addiu forms the address of D_8009B708 in v1, lw returns the old value in v0, and sw a0 occupies the jr delay slot. The previous boundary at 0x80081E58 is the real stack teardown instruction ending func_80081DF8; the following boundary at 0x80081E70 is the real first instruction of func_80081E70. Exact-start direct callers are file 0x6F538 / VA 8007ED38 and file 0x6F63C / VA 8007EE3C.

This is the same proven scalar-global exchange/address-retention family as func_800824C8 and func_800824DC. The bounded 824C8 attempts already established that natural era -O2 -G0 C does not retain the symbolic address in v1 through the load and store-in-jr slot; retrying this identical shape would violate the two-iteration rule. It is screened without a separate attempt.

Disposition: remove from matching-C scheduling as PARKED-ADDRESS-RETENTION-FAMILY. No YAML/build/verifier integration.
