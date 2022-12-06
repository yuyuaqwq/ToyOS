#include "sync.h"

#include "kernel/interrupt.h"

void SemaInit(Semaphore* pSema, uint8 value) {
    pSema->value = value;
    ListInit(&pSema->waiters);
}

void LockInit(Lock* pLock) {
    pLock->holder = NULL;
    pLock->holderRepeatNr = 0;
    SemaInit(&pLock->semaphore, 1);
}

void SemaDown(Semaphore* pSema) {
    IntrStatus oldStatus = IntrDisable();
    while (pSema->value == 0) {
        ASSERT(!ElemFind(&pSema->waiters, &RunningThread()->generalTag));
        if (ElemFind(&pSema->waiters, &RunningThread()->generalTag)) {
            PANIC("SemaDown: thread blocked has been in waiters list\n");
        }
        ListAppend(&pSema->waiters, &RunningThread()->generalTag);
        ThreadBlock(kTaskBlocked);
    }
}

