/* Phase 5EF: event callback setter.
 * YAML-live readers null-check then jalr with event/context in a0/a1. */
typedef void (*func_800A36A4_t)(int event, void *context);

extern func_800A36A4_t D_800A36A4;

void func_8007FBCC(func_800A36A4_t callback) {
    D_800A36A4 = callback;
}
