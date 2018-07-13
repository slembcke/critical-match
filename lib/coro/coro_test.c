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
	// log_stack("In func(). n: %d\n");
	printf("In func(). n: %04X\n", n);
	n = coro_yield(n);
	printf("Back in func(). %n\n", n);
	n = coro_yield(n);
	
	func2(n);
	
	n = coro_yield(n);
	n = coro_yield(n);
	
	coro_yield(0);
	return 0;
}

uint8_t buff[32];

int main(void){
	static uintptr_t n;
	
	printf("func: $%04X\n", func);
	
	// log_stack("before start");
	coro_init(func, buff, sizeof(buff));
	printf("Coroutine intialized.\n");
	
	printf("buff: ");
	for(n = 0; n < sizeof(buff); ++n){
		printf(" $%02X", buff[n]);
	}
	printf("\n");
	
	n = coro_resume(0xABCD);
	printf("Back in main(). n: $%04X\n", n);
	
	for(n = 1; n < 20; ++n){
		uintptr_t value;
		value = coro_resume(n);
		printf("main() n: %d, value: %d\n", n, value);
		// log_stack("after resume");
		
		if(value == 0) break;
	}
	
	return EXIT_SUCCESS;
}
