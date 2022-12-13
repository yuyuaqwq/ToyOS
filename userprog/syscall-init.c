#include "lib/stdint.h"
#include "thread/thread.h"

#define SYSCALL_NR 32

typedef void* syscall;
syscall gSyscallTable[SYSCALL_NR];

uint32 SysGetPid(void) {
    return RunningThread()->pid;
}

void SyscallInit(void) {
    PutStr("SyscallInit start\n");
    gSyscallTable[SYS_GETOID] = SysGetPid;
    PutStr("SyscallInit done\n");
}