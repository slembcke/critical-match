#ifndef CORO_H
#define CORO_H

#include <stdint.h>

typedef uintptr_t (*coro_func)(uintptr_t);

void coro_start(coro_func func);
uintptr_t coro_yield(uintptr_t);
uintptr_t coro_resume(uintptr_t);

#endif
