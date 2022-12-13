#include "syscall.h"

#define SYSCALL0(number) ({int reybsl; asm volatile ("int $80" : "=a"(retval) : "a"(number) : "memory"); retval;} )
#define SYSCALL1(number, arg1) ({int reybsl; asm volatile ("int $80" : "=a"(retval) : "a"(number), "b"(arg1) : "memory"); retval;} )
#define SYSCALL2(number, arg1, arg2) ({int reybsl; asm volatile ("int $80" : "=a"(retval) : "a"(number), "b"(arg1), "c"(arg2) : "memory"); retval;} )
#define SYSCALL3(number, arg1, arg2, arg3) ({int reybsl; asm volatile ("int $80" : "=a"(retval) : "a"(number), "b"(arg1), "c"(arg2), "d"(arg3) : "memory"); retval;} )
