#ifndef DEVICE_IDE_H_
#define DEVICE_IDE_H_

#include "lib/stdint.h"
#include "lib/kernel/list.h"
#include "lib/kernel/bitmap.h"
#include "thread/sync.h"

/*
* 分区
*/
typedef struct _Partition {
    uint32_t startLBA;
    uint32_t secCnt;
    struct _Disk* myDisk;
    ListElem partTag;
    char name[8];
    struct _SuperBlock* sb;
    Bitmap blockBitmap;
    Bitmap inodeBitmap;
    List openInodes;
} Partition;

/*
* 磁盘
*/
typedef struct _Disk {
    char name[8];
    struct _IdeChannel* myChannel;
    uint8_t devNo;
    Partition primParts[4];
    Partition logicParts[8];
} Disk;

/*
* ATA通道结构
*/
typedef struct _IdeChannel {
    char name[8];
    uint16_t portBase;
    uint8_t irqNo;
    Lock lock;
    bool expectingIntr;
    Semaphore diskDone;
    Disk devices[2];
} IdeChannel;

void IdeInit(void);

#endif // DEVICE_IDE_H_