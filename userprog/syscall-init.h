#ifndef USERPROG_SYSCALL_INIT_H_
#define USERPROG_SYSCALL_INIT_H_

#include "lib/stdint.h"

uint32_t SysGetPid(void);
void SyscallInit(void);

#endif // USERPROG_SYSCALL_INIT_H_