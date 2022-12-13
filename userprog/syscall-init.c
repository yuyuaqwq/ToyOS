#include "syscall-init.h"

#include "lib/stdint.h"
#include "lib/string.h"
#include "thread/thread.h"
#include "lib/user/syscall.h"
#include "device/console.h"

#define SYSCALL_NR 32

typedef void* syscall;
syscall gSyscallTable[SYSCALL_NR];

uint32_t SysGetPid(void) {
    return RunningThread()->pid;
}

uint32_t SysWrite(char* str) {
    ConsolePutStr(str);
    return strlen(str);
}

void SyscallInit(void) {
    PutStr("SyscallInit start\n");
    gSyscallTable[kSysGetPid] = SysGetPid;
    gSyscallTable[kSysWrite] = SysWrite;
    PutStr("SyscallInit done\n");
}