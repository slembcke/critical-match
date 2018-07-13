#ifndef NACO_H
#define NACO_H

#include <stdint.h>
#include <stdlib.h>

typedef uintptr_t (*naco_func)(uintptr_t);

void naco_init(naco_func func, void *naco_buffer, size_t buffer_size);
uintptr_t naco_yield(uintptr_t value);
uintptr_t naco_resume(void *naco_buffer, uintptr_t value);

#endif
