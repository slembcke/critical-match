#include <stdlib.h>
#include <stdio.h>

#include "coro.h"

#define CPU_STACK ((uint8_t *)0x0100)

void log_stack(const char *msg){
	static uint8_t s, i;
	
	asm("tsx");
	asm("txa");
	s = __AX__;
	
	printf("%s s: %d ->", msg, s);
	for(i = 255; i > s; --i){
		printf(" $%02X", CPU_STACK[i]);
	}
	
	printf("\n");
}

void func2(uintptr_t n){
	log_stack("in func2");
	n = coro_yield(n);
	n = coro_yield(n);
}

uintptr_t func(uintptr_t n){
	log_stack("in func");
	n = coro_yield(n);
	log_stack("after_yield");
	n = coro_yield(n);
	log_stack("after_yield");
	
	func2(n);
	
	n = coro_yield(n);
	n = coro_yield(n);
	
	return 0;
}

int main(void){
	static uint8_t n;
	
	log_stack("before start");
	coro_start(func);
	
	for(n = 1; n < 20; ++n){
		uintptr_t value = coro_resume(n);
		printf("main() n: %d, value: %d\n", n, value);
		log_stack("after resume");
		
		if(value == 0) break;
	}
	
	return EXIT_SUCCESS;
}
