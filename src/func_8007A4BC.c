/* Phase 5EF: event callback exchange.
 * Readers null-check then jalr with event/context in a0/a1. */
typedef void (*func_8009AFB8_t)(int event, void *context);

extern func_8009AFB8_t D_8009AFB8;

func_8009AFB8_t func_8007A4BC(func_8009AFB8_t callback) {
    func_8009AFB8_t old = D_8009AFB8;
    D_8009AFB8 = callback;
    return old;
}
