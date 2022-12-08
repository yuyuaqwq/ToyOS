#ifndef LIB_STRING_H_
#define LIB_STRING_H_

#include "lib/stdint.h"

#include "string.h"

#include "kernel/global.h"
#include "kernel/debug.h"

void memset(void* dst_, uint8 value, uint32 size);

void memcpy(void* dst_, const void* src_, uint32 size);

int32 memcmp(const void* a_, const void* b_, uint32 size);

char* strcpy(char* dst_, const char* src_);

uint32 strlen(const char* str);

int32 strcmp(const char* a, const char* b);

char* strchr(const char* str, const uint8 ch);

char* strrchr(const char* str, const uint8 ch);

char* strcat(char* dst_, const char* src_);

uint32 strchrs(const char* str, uint8 ch) ;

#endif // LIB_STRING_H_