#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "slib.h"

static size_t PATH_PREFIX_LENGTH = 0;

void _slib_log(const char *format, const char *file, uint line, const char *message, ...){
	char message_buffer[1024];
	
	va_list vargs;
	va_start(vargs, message); {
		vsnprintf(message_buffer, sizeof(message_buffer), message, vargs);
	} va_end(vargs);
	
	fprintf(stdout, format, file + PATH_PREFIX_LENGTH, line, message_buffer);
}

void _slib_assert_helper(const char *condition, const char *file, uint line, bool warn, const char *message, ...){
	char message_buffer[1024];

	va_list vargs;
	va_start(vargs, message); {
		vsnprintf(message_buffer, sizeof(message_buffer), message, vargs);
	} va_end(vargs);
	
	const char *message_type = (warn ? "Warning" : "Aborting due to error");
	fprintf(stderr,
		"%s: %s\n"
		"\tFailed condition: %s\n"
		"\tSource:%s:%d\n",
		message_type, message_buffer, condition, file + PATH_PREFIX_LENGTH, line
	);
}
