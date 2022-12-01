#ifndef LIB_IO_H_
#define LIB_IO_H_

#include "lib\stdint.h"

static inline void outb(uint16 port, uint8 data) {
    asm volatile("outb %b0, %w1" : : "a"(data), "Nd"(port));
}

static inline void outsw(uint16 port, const void* addr, uint32 wordCnt) {
    asm volatile("cld; rep outsw" : "+S"(addr), "+c"(wordCnt) : "d"(port));
}


static inline uint8 inb(uint16 port) {
    uint8 data;
    asm volatile("inb %w1 %b0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void insw(int16 port, void* addr, uint32 wordCnt) {
    asm volatile("cld; rep insw" : "+D"(addr), "+c"(wordCnt) : "d"(port) : "memory");
}

#endif // LIB_IO_H_