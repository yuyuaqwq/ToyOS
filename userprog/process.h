#ifndef USERPROG_PROCESS_H_
#define USERPROG_PROCESS_H_

#include "lib/stdint.h"
#include "thread/thread.h"

#define DEFAULT_PRIO 31

#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define USER_VADDR_START 0x8048000


void ProcessActivate(TaskStruct* pThread);
void ProcessExecute(void* filename, char* name);

#endif // USERPROG_PROCESS_H_