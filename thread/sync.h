#ifndef THREAD_SYNC_H_
#define THREAD_SYNC_H_

#include "lib/stdint.h"
#include "lib/kernel/list.h"

#include "thread/thread.h"

typedef struct _Semaphore {
    uint8 value;
    List waiters;
} Semaphore;

typedef struct _Lock {
    TaskStruct* holder;
    Semaphore semaphore;
    uint32 holderRepeatNr;
} Lock;


#endif // THREAD_SYNC_H_