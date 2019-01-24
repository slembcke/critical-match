#ifndef NACO_H
#define NACO_H

#include <stdint.h>
#include <stdlib.h>

// Coroutine body function.
// 'value' will be the parameter passed to the naco_resume() call that starts the coroutine.
typedef uintptr_t (*naco_func)(uintptr_t value);

// Initialize a coroutine's stack buffer and body function.
// Coroutine does not begin executing until naco_resume() is called.
void naco_init(naco_func func, void *naco_buffer, size_t buffer_size);

// Execute a coroutine until it calls naco_yield().
// The return value is the value the coroutine passes to naco_yield();
uintptr_t naco_resume(void *naco_buffer, uintptr_t value);

// Yield from a coroutine back to the main thread.
// 'value' will be returned by the most recent call to naco_resume();
uintptr_t naco_yield(uintptr_t value);

#endif
