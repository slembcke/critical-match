#include <stdbool.h>
#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>

typedef uintptr_t (*coro_func)(uintptr_t);

void coro_start(coro_func func);
uintptr_t coro_yield(uintptr_t);
uintptr_t coro_resume(uintptr_t);

static void func2(uintptr_t n){
	n = coro_yield(n);
	n = coro_yield(n);
}

static uintptr_t func(uintptr_t n){
	n = coro_yield(n);
	n = coro_yield(n);
	
	func2(n);
	
	n = coro_yield(n);
	n = coro_yield(n);
	
	return 0;
}

int main(void){
	static uint8_t n;
	
	coro_start(func);
	
	for(n = 1; n < 20; ++n){
		uintptr_t value = coro_resume(n);
		printf("main() n: %d, value: %d\n", n, value);
		
		if(value == 0) break;
	}
	
	return EXIT_SUCCESS;
}
