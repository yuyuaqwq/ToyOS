#ifndef LIB_STDIO_H_
#define LIB_STDIO_H_

#include "lib/stdint.h"

typedef char* va_list;
uint32_t printf(const char* str, ...);
uint32_t vsprintf(char* str, const char* format, va_list ap);
// uint32_t sprintf(char* buf, const char* format, ...);

#endif // LIB_STDIO_H_