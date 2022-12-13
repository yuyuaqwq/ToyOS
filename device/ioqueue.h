#ifndef DEVICE_IOQUEUE_H_
#define DEVICE_IOQUEUE_H_

#include "lib/stdint.h"
#include "thread/thread.h"
#include "thread/sync.h"

#define BUF_SIZE 64

typedef struct _IoQueue {
    Lock lock;
    TaskStruct* producer;
    TaskStruct* consumer;
    char buf[BUF_SIZE];
    int32_t head;
    int32_t tail;
} IoQueue;

#include "ioqueue.h"

#include "kernel/interrupt.h"
#include "kernel/global.h"
#include "kernel/debug.h"

void IoQueueInit(IoQueue* pIoq);
bool IoQueueFull(IoQueue* pIoq);
bool IoQueueEmpty(IoQueue* pIoq);
char IoQueueGetChar(IoQueue* pIoq);
void IoQueuePutChar(IoQueue* pIoq, char byte);

#endif // DEVICE_IOQUEUE_H_