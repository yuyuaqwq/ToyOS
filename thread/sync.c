#include "sync.h"

#include "kernel/interrupt.h"
#include "kernel/debug.h"
#include "thread/thread.h"

/*
* 初始化信号量
*/
void SemaInit(Semaphore* pSema, uint8 value) {
    pSema->value = value;
    ListInit(&pSema->waiters);
}

/*
* 信号量减少，当前线程阻塞
*/
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

    pSema->value--;
    ASSERT(pSema->value == 0);
    IntrSetStatus(oldStatus);
}

/*
* 信号量增加，唤醒一个线程
*/
void SemaUp(Semaphore* pSema) {
    IntrStatus oldStatus = IntrDisable();
    ASSERT(pSema->value == 0);
    if (!ListEmpty(&pSema->waiters)) {
        TaskStatus* threadBlocked = ELEM_TO_ENTRY(TaskStruct, generalTag, ListPop(&pSema->waiters));
        ThreadUnblock(threadBlocked);
    }
    pSema->value++;
    ASSERT(pSema->value == 1);
    IntrSetStatus(oldStatus);
}


void LockInit(Lock* pLock) {
    pLock->holder = NULL;
    pLock->holderRepeatNr = 0;
    SemaInit(&pLock->semaphore, 1);
}

void LockAcquire(Lock* pLock) {
    if (pLock->holder != RunningThread()) {
        SemaDown(&pLock->semaphore);
        pLock->holder = RunningThread();
        ASSERT(pLock->holderRepeatNr == 0);
        pLock->holderRepeatNr = 1;
    } else {
        pLock->holderRepeatNr++;
    }
}

void LockRelease(Lock* pLock) {
    ASSERT(pLock->holder == RunningThread());
    if (pLock->holderRepeatNr > 1) {
        pLock->holderRepeatNr--;
        return;
    }
    ASSERT(pLock->holderRepeatNr == 1);
    pLock->holder = NULL;
    pLock->holderRepeatNr = 0;
    SemaUp(&pLock->semaphore);
}
