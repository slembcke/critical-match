#include <stdbool.h>
#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>

// TODO Pass varargs?
typedef void (*coro_func)(void);

void coro_start(coro_func func);
uint8_t coro_yield(uint8_t);
uint8_t coro_resume(uint8_t);

void func(){
	static uint8_t n;
	
	while(true){
		n = coro_yield(2*n);
		printf("func() n: %d\n", n);
		
		if(n == 10) abort();
	}
}

int main(void){
	static uint8_t n;
	coro_start(func);
	
	while(true){
		uint8_t value = coro_resume(n);
		printf("main() n: %d, value: %d\n", n, value);
		
		++n;
	}
	
	return EXIT_SUCCESS;
}
