/* Phase 5EF: event callback exchange.
 * Readers null-check then jalr with event/context in a0/a1. */
typedef void (*func_8009AFB4_t)(int event, void *context);

extern func_8009AFB4_t D_8009AFB4;

func_8009AFB4_t func_8007A4A8(func_8009AFB4_t callback) {
    func_8009AFB4_t old = D_8009AFB4;
    D_8009AFB4 = callback;
    return old;
}
