/* Phase 5EF: callback exchange.
 * YAML-live readers null-check then jalr with event/context in a0/a1. */
typedef void (*func_8009B6D0_t)(int event, void *context);

extern func_8009B6D0_t D_8009B6D0;

func_8009B6D0_t func_80081254(func_8009B6D0_t callback) {
    func_8009B6D0_t old = D_8009B6D0;
    D_8009B6D0 = callback;
    return old;
}
