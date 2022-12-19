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
    uint32_t startLBA;      // 起始扇区
    uint32_t secCnt;        // 扇区数
    struct _Disk* myDisk;   // 分区所属硬盘
    ListElem partTag;       // 用于队列中的标记
    char name[8];       // 分区名称
    struct _SuperBlock* sb;     // 本分区的超级块
    Bitmap blockBitmap;     // 块位图
    Bitmap inodeBitmap;     // i结点位图
    List openInodes;        // 本分区打开的i结点队列
} Partition;

/*
* 硬盘
*/
typedef struct _Disk {
    char name[8];       // 硬盘名称
    struct _IdeChannel* myChannel;      // 硬盘所属ide通道
    uint8_t devNo;      // 主硬盘0，从硬盘1
    Partition primParts[4];     // 主分区最多是4个
    Partition logicParts[8];        // 限制最多8个逻辑分区
} Disk;

/*
* ATA通道结构
*/
typedef struct _IdeChannel {
    char name[8];       // 通道名称
    uint16_t portBase;      // 通道起始端口号
    uint8_t irqNo;      // 通道所有中断号
    Lock lock;      // 通道锁
    bool expectingIntr;     // 等待硬盘的中断
    Semaphore diskDone;     // 用于阻塞、唤醒驱动程序
    Disk devices[2];        // 一个通道连接两个硬盘，一主一从
} IdeChannel;

void IdeInit(void);

#endif // DEVICE_IDE_H_