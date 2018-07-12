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
	
	printf("\n %p\n", *((void **)0));
}

void func2(uintptr_t n){
	n = coro_yield(n);
	n = coro_yield(n);
}

uintptr_t func(uintptr_t n){
	n = coro_yield(n);
	n = coro_yield(n);
	
	func2(n);
	
	n = coro_yield(n);
	n = coro_yield(n);
	
	coro_yield(0);
	return 0;
}

uint8_t buff[16];

int main(void){
	static uint8_t n;
	
	log_stack("before start");
	coro_init(func, buff, sizeof(buff));
	
	for(n = 1; n < 20; ++n){
		uintptr_t value;
		value = coro_resume(n);
		printf("main() n: %d, value: %d\n", n, value);
		// log_stack("after resume");
		
		if(value == 0) break;
	}
	
	return EXIT_SUCCESS;
}
