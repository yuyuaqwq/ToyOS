#ifndef LIB_USER_SYSCALL_H_
#define LIB_USER_SYSCALL_H_

#include "lib/stdint.h"

typedef enum SyscallNr {
    kSysGetPid,
    kSysWrite,
    kSysMalloc,
    kSysFree,
};

uint32_t GetPid(void);
uint32_t Write(char* str);

void* malloc(uint32_t size);
void free(void* ptr);

#endif