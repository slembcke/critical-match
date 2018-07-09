#include <stdlib.h>
#include <stdio.h>

#include "coro.h"

static void func2(uintptr_t n){
	n = coro_yield(n);
	n = coro_yield(n);
}

static uintptr_t func(uintptr_t n){
	printf("func()\n");
	
	n = coro_yield(n);
	n = coro_yield(n);
	
	func2(n);
	
	n = coro_yield(n);
	n = coro_yield(n);
	
	coro_yield(0);
	return 0;
}

int main(void){
	static uint8_t n;
	
	coro_start((coro_func)func);
	
	for(n = 1; n < 20; ++n){
		uintptr_t value;
		value = coro_resume(n);
		printf("main() n: %d, value: %d\n", n, value);
		
		if(value == 0) break;
	}
	
	return EXIT_SUCCESS;
}
