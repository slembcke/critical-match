// Slembcke's Utility Lib

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#if __cplusplus
	#define SLIB_EXTERN_C_BEGIN extern "C" {
	#define SLIB_EXTERN_C_END }
#else
	#define SLIB_EXTERN_C_BEGIN
	#define SLIB_EXTERN_C_END
#endif

// Not standard in C++, but seems to be universally supported?
#define SLIB_RESTRICT __restrict__

// C's sized int types are annoyingly long winded.

typedef unsigned uint;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// typedef sint8_t s8;
// typedef sint16_t s16;
// typedef sint32_t s32;
// typedef sint64_t s64;

SLIB_EXTERN_C_BEGIN
void _slib_log(const char *format, const char *file, unsigned line, const char *message, ...);
void _slib_assert_helper(const char *condition, const char *file, unsigned line, bool warn, const char *message, ...);
SLIB_EXTERN_C_END

#define SLIB_STR(_S_) #_S_
#define SLIB_XSTR(_S) SLIB_STR(_S)

#ifdef NDEBUG
	#define SLIB_DEBUG 0
	#define SLIB_LOG_DEBUG(...)
	#define	SLIB_ASSERT(__condition__, ...)
	#define	SLIB_ASSERT_WARN(__condition__, ...)
#else
	#define SLIB_DEBUG 1
	#define SLIB_LOG_DEBUG(...) _slib_log("[DEBUG] %s:%d: %s\n", __FILE__, __LINE__, __VA_ARGS__)
	#define SLIB_ASSERT(__condition__, ...) if(!(__condition__)){_slib_assert_helper(#__condition__, __FILE__, __LINE__, false, __VA_ARGS__); abort();}
	#define SLIB_ASSERT_WARN(__condition__, ...) if(!(__condition__)) _slib_assert_helper(#__condition__, __FILE__, __LINE__, true, __VA_ARGS__)
#endif

#define SLIB_LOG(...) _slib_log("[LOG] %s:%d: %s\n", __FILE__, __LINE__, __VA_ARGS__)
#define SLIB_ASSERT_HARD(__condition__, ...) if(!(__condition__)){_slib_assert_helper(#__condition__, __FILE__, __LINE__, false, __VA_ARGS__); abort();}
#define SLIB_ABORT(...) {_slib_log("[Abort] %s:%d\n\tReason: %s\n", __FILE__, __LINE__, __VA_ARGS__); abort();}
#define SLIB_NYI() {_slib_log("[Abort] %s:%d\n\tReason: Not yet implemented.\n", __FILE__, __LINE__, ""); abort();}
