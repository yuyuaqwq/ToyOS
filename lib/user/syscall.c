#include "syscall.h"


#define SYSCALL0(number) ({int retval; asm volatile ("pushl %[NUMBER]; int $80; addl $4, %%esp" : "=a"(retval) : [NUMBER]"i"(number) : "memory"); retval;} )

#define SYSCALL1(number, arg0)({int retval; asm volatile ("pushl %[ARG0]; pushl %[NUMBER]; int $80; addl $8, %%esp" : "=a"(retval) : [NUMBER]"i"(number), [ARG0]"g"(arg0) : "memory"); retval;} )

#define SYSCALL2(number, arg0, arg1)({int retval; asm volatile ("pushl %[ARG1]; pushl %[ARG0]; pushl %[NUMBER]; int $80; addl $12, %%esp" : "=a"(retval) : [NUMBER]"i"(number), [ARG0]"g"(arg0), [ARG1]"g"(arg1) : "memory"); retval;} )

#define SYSCALL3(number, arg0, arg1, arg2) ({int retval; asm volatile ("pushl %[ARG2]; pushl %[ARG1]; pushl %[ARG0]; pushl %[NUMBER]; int $80; addl $16, %%esp" : "=a"(retval) : [NUMBER]"i"(number), [ARG0]"g"(arg0), [arg1]"g"(ARG1), [arg2]"g"(ARG2) : "memory"); retval;} )

uint32_t GetPid() {
    return SYSCALL0(kSysGetPid);
}


uint32_t Write(char* str) {
    return SYSCALL1(kSysWrite, str);
}

void* malloc(uint32_t size) {
    return SYSCALL1(kSysMalloc, size);
}

void free(void* ptr) {
    SYSCALL1(kSysFree, ptr);
}