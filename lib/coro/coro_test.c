#include <stdbool.h>
#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>

typedef uintptr_t (*coro_func)(uintptr_t);

void coro_start(coro_func func);
uintptr_t coro_yield(uintptr_t);
uintptr_t coro_resume(uintptr_t);

uintptr_t func(uintptr_t n){
	while(true){
		printf("func() n: %d\n", n);
		n = coro_yield(2*n);
		
		if(n == 10) abort();
	}
	
	return 67;
}

int main(void){
	static uint8_t n;
	coro_start(func);
	printf("%d\n", coro_resume(55));
	
	puts("main() resume.\n");
	
	while(true){
		uint8_t value = coro_resume(n);
		printf("main() n: %d, value: %d\n", n, value);
		
		++n;
	}
	
	return EXIT_SUCCESS;
}
