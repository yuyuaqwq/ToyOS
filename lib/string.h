#ifndef LIB_STRING_H_
#define LIB_STRING_H_

#include "lib/stdint.h"

#include "string.h"

#include "kernel/global.h"
#include "kernel/debug.h"

void memset(void* dst_, uint8_t value, uint32_t size);
void memcpy(void* dst_, const void* src_, uint32_t size);
int32_t memcmp(const void* a_, const void* b_, uint32_t size);

char* strcpy(char* dst_, const char* src_);
uint32_t strlen(const char* str);
int32_t strcmp(const char* a, const char* b);
char* strchr(const char* str, const uint8_t ch);
char* strrchr(const char* str, const uint8_t ch);
char* strcat(char* dst_, const char* src_);
uint32_t strchrs(const char* str, uint8_t ch) ;

#endif // LIB_STRING_H_