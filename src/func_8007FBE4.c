/* Phase 5EF: event callback setter.
 * YAML-live readers null-check then jalr with event/context in a0/a1. */
typedef void (*func_800A36AC_t)(int event, void *context);

extern func_800A36AC_t D_800A36AC;

void func_8007FBE4(func_800A36AC_t callback) {
    D_800A36AC = callback;
}
