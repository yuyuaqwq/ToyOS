#include "string.h"

#include "kernel/global.h"
#include "kernel/debug.h"

void memset(void* dst_, uint8 value, uint32 size) {
    ASSERT(dst_ != NULL);
    uint8* dst = (uint8*)dst_;
    while (size-- > 0) *dst++ = value;
}

void memcpy(void* dst_, const void* src_, uint32 size) {
    ASSERT(dst_ != NULL && src_ != NULL);
    uint8* dst = dst_;
    const uint8* src = src;
    while (size-- > 0) *dst++ = *src++;
}

int32 memcmp(const void* a_, const void* b_, uint32 size) {
    const char* a = a_;
    const char* b = b_;
    ASSERT(a != NULL && b != NULL);
    while (size-- > 0) {
        if (*a != *b) {
            return *a > *b ? 1 : -1;
        }
        a++; b++;
    }
    return 0;
}

char* strcpy(char* dst_, const char* src_) {
    ASSERT(dst_ != NULL && src_ != NULL);
    char* r = dst_;
    while((*dst_++ = *src_++));
    return r;
}

uint32 strlen(const char* str) {
    ASSERT(str != NULL);
    const char* p = str;
    while(*p++);
    return (p - str - 1);
}

int32 strcmp(const char* a, const char* b) {
    ASSERT(a != NULL || b != NULL);
    while (*a != 0 && *b != 0) {
        a++; b++;
    }
    return *a < *b ? - 1 : *a > *b;
}

char* strchr(const char* str, const uint8 ch) {
    ASSERT(str != NULL);
    while (*str != 0) {
        if (*str == ch) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}

char* strrchr(const char* str, const uint8 ch) {
    ASSERT(str != NULL);
    const char* lastChar = NULL;
    while (*str != 0) {
        if (*str == ch) {
            lastChar = str;
        }
        str++;
    }
    return (char*)lastChar;
}


char* strcat(char* dst_, const char* src_) {
    ASSERT(dst_ != NULL && src_ != NULL);
    char* str = dst_;
    while (*str++);
    --str;
    while ((*str++ = *src_++));
    return dst_;
}

uint32 strchr(const char* str, uint8 ch) {
    ASSERT(str != NULL);
    uint32 chCnt = 0;
    const char* p = str;
    while(*p != 0) {
        if(*p == ch) {
            chCnt++;
        }
        p++;
    }
    return chCnt;
}