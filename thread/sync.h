#ifndef THREAD_SYNC_H_
#define THREAD_SYNC_H_

#include "lib/stdint.h"
#include "lib/kernel/list.h"


typedef struct _Semaphore {
    uint8_t value;
    List waiters;
} Semaphore;

typedef struct _Lock {
    struct _TaskStruct* holder;
    Semaphore semaphore;
    uint32_t holderRepeatNr;
} Lock;





void SemaInit(Semaphore* pSema, uint8_t value);
void SemaDown(Semaphore* pSema);
void SemaUp(Semaphore* pSema);

void LockInit(Lock* pLock);
void LockAcquire(Lock* pLock);
void LockRelease(Lock* pLock);

#endif // THREAD_SYNC_H_