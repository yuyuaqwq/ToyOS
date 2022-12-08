#include "ioqueue.h"

#include "kernel/interrupt.h"
#include "kernel/global.h"
#include "kernel/debug.h"

void IoQueueInit(IoQueue* pIoq) {
    LockInit(&pIoq->lock);
    pIoq->producer = pIoq->consumer = NULL;
    pIoq->head = pIoq->tail = 0;
}

static int32 NextPos(int32 pos) {
    return (pos + 1) % BUF_SIZE;
}

bool IoQueueFull(IoQueue* pIoq) {
    ASSERT(IntrGetStatus() == kIntrOff);
    return NextPos(pIoq->head) == pIoq->tail;
}

bool IoQueueEmpty(IoQueue* pIoq) {
    ASSERT(IntrGetStatus() == kIntrOff);
    return pIoq->head == pIoq->tail;
}

static void IoQueueWait(TaskStatus** waiter) {
    ASSERT(*waiter == NULL && waiter != NULL);
    *waiter = RunningThread();
    ThreadBlock(kTaskBlocked);
}

static void Wakeup(TaskStatus** waiter) {
    ASSERT(*waiter != NULL);
    ThreadUnblock(*waiter);
    *waiter = NULL;
}

char IoQueueGetChar(IoQueue* pIoq) {
    ASSERT(IntrGetStatus() == kIntrOff);

    while(IoQueueEmpty(pIoq)) {
        LockAcquire(&pIoq->lock);
        IoQueueWait(&pIoq->consumer);
        LockRelease(&pIoq->lock);
    }

    char byte = pIoq->buf[pIoq->tail];
    pIoq->tail = NextPos(pIoq->tail);

    if (pIoq->producer != NULL) {
        Wakeup(&pIoq->producer);
    }

    return byte;
}


void IoQueuePutChar(IoQueue* pIoq, char byte) {
    ASSERT(IntrGetStatus() == kIntrOff);

    while (IoQueueFull(pIoq)) {
        LockAcquire(&pIoq->lock);
        IoQueueWait(&pIoq->producer);
        LockRelease(&pIoq->lock);
    }
    pIoq->buf[pIoq->head] = byte;
    pIoq->head = NextPos(pIoq->head);

    if (pIoq->consumer != NULL) {
        Wakeup(&pIoq->consumer);
    }
}