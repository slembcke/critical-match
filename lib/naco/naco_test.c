#include <stdlib.h>
#include <stdio.h>

#include "naco.h"

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
	n = naco_yield(n);
	n = naco_yield(n);
}

uintptr_t func(uintptr_t n){
	// log_stack("In func(). n: %d\n");
	// printf("In func(). n: %04X\n", n);
	n = naco_yield(n);
	// printf("Back in func(). %n\n", n);
	n = naco_yield(n);
	
	func2(n);
	
	n = naco_yield(n);
	n = naco_yield(n);
	
	return 0;
}

uint8_t buff[64];

int main(void){
	static uintptr_t n;
	
	// log_stack("before start");
	naco_init(func, buff, sizeof(buff));
	
	for(n = 1; n < 20; ++n){
		uintptr_t value;
		value = naco_resume(buff, n);
		printf("main() n: %d, value: %d\n", n, value);
		
		if(value == 0) break;
	}
	
	return EXIT_SUCCESS;
}
