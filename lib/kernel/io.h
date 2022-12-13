#ifndef LIB_IO_H_
#define LIB_IO_H_

#include "lib\stdint.h"

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %b0, %w1" : : "a"(data), "Nd"(port));
}

static inline void outsw(uint16_t port, const void* addr, uint32_t wordCnt) {
    asm volatile("cld; rep outsw" : "+S"(addr), "+c"(wordCnt) : "d"(port));
}


static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void insw(int16_t port, void* addr, uint32_t wordCnt) {
    asm volatile("cld; rep insw" : "+D"(addr), "+c"(wordCnt) : "d"(port) : "memory");
}

#endif // LIB_IO_H_