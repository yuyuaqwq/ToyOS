#ifndef USERPROG_TSS_H_
#define USERPROG_TSS_H_

#include "thread/thread.h"

void TssInit(void);
void UpdateTssEsp(TaskStruct* pThread);

#endif // USERPROG_TSS_H_