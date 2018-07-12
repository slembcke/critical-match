#ifndef CORO_H
#define CORO_H

#include <stdint.h>
#include <stdlib.h>

typedef uintptr_t (*coro_func)(uintptr_t);

void coro_init(coro_func func, void *coro_buffer, size_t buffer_size);
uintptr_t coro_yield(uintptr_t value);
uintptr_t coro_resume(uintptr_t value);

#endif
