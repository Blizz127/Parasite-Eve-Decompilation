/* Phase 5EF: event callback setter.
 * YAML-live readers null-check then jalr with event/context in a0/a1. */
typedef void (*func_800A36A8_t)(int event, void *context);

extern func_800A36A8_t D_800A36A8;

void func_8007FBD8(func_800A36A8_t callback) {
    D_800A36A8 = callback;
}
