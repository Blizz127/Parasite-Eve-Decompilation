# Rehearsal flashes with concurrent pistol effects

Run28 uses DAY1-11 binary SHA256
`3bfcd7f5928bfdf18130af9ccf4094b7ae6a47f71450e5538ce1940a9699ade0`.
Normal inputs reach the rehearsal fight atframe47020 withHP45. The run
passes the prior missing flash context, then stops atframe49014 on script36
(80013E84), HP4. Battle victory and retail pixel equality are not established.
Screenshots come from read-only captures of the presented framebuffer;
no live gameplay RAM was written. `verification.json` records provenance.

DAY1-11 original/native evidence:644 synthetic effect-pump frames and64
copied actual-room pumps match defined output. The later DAY1-12 actor-follow
implementation was not part of this live binary. See
[stack analysis](../../ai_context/M0023I_FLASH_STACK.md).
