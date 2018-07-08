#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>

typedef uint8_t u8;

// TODO Pass varargs?
typedef void (*coro_func)(void);

extern void *resume_addr;
extern void *yield_addr;

void coro_start(coro_func func);
void coro_yield(void);
void coro_resume(void);

void func(void){
	printf("func calling coro_yield().\n");
	coro_yield();
	
	printf("func() exiting.\n");
}

int main(void){
	coro_start(func);
	
	printf("main() calling coro_resume().\n");
	coro_resume();
	
	printf("main() calling coro_resume().\n");
	coro_resume();
	
	printf("main() exiting.\n");
	return EXIT_SUCCESS;
}
