#include "ide.h"
#include "device/timer.h"
#include "kernel/interrupt.h"
#include "kernel/memory.h"
#include "kernel/debug.h"
#include "lib/kernel/stdio-kernel.h"
#include "lib/stdio.h"
#include "lib/kernel/io.h"

// 硬盘各寄存器端口号
#define REG_DATA(channel) (channel->portBase + 0)
#define REG_ERROR(channel) (channel->portBase + 1)
#define REG_SECT_CNT(channel) (channel->portBase + 2)
#define REG_LBA_L(channel) (channel->portBase + 3)
#define REG_LBA_M(channel) (channel->portBase + 4)
#define REG_LBA_H(channel) (channel->portBase + 5)
#define REG_DEV(channel) (channel->portBase + 6)
#define REG_STATUS(channel) (channel->portBase + 7)
#define REG_CMD(channel) (REG_STATUS(channel))
#define REG_ALT_STATUS(channel) (channel->portBase + 0x206)
#define REG_CTL(channel) (REG_ALT_STATUS(channel))


// REG_ALT_STATUS寄存器的关键位
#define BIT_STAT_BSY 0x80
#define BIT_STAT_DRDY 0x40
#define BIT_STAT_DRQ 0x8

// device寄存器的关键位
#define BIT_DEV_MBS 0xa0
#define BIT_DEV_LBA 0x40
#define BIT_DEV_DEV 0x10

// 硬盘操作的指令
#define CMD_IDENTIFY 0xec
#define CMD_READ_SECTOR 0x20
#define CMD_WRITE_SECTOR 0x30

// 可读写最大的扇区数
#define MAX_LBA ((80*1024*1024/512) - 1)        // 80MB

uint8_t gChannelCnt;        // 按硬盘数计算的通道数
IdeChannel gChannels[2];     // 两个ide通道

/*
* 选择读写的硬盘
*/
static void SelectDisk(Disk* hd) {
    uint32_t regDevice = BIT_DEV_MBS | BIT_DEV_LBA;
    if (hd->devNo == 1) {       // 从盘就置DEV位为1
        regDevice |= BIT_DEV_DEV;
    }
    outb(REG_DEV(hd->myChannel), regDevice);
}

/*
* 向硬盘控制器写入起始扇区地址及要读写的扇区数
*/
static void SelectSector(Disk* hd, uint32_t lba, uint32_t secCnt) {
    ASSERT(lba <= MAX_LBA);
    IdeChannel* channel = hd->myChannel;

    // 写入要读写的扇区数
    outb(REG_SECT_CNT(channel), secCnt);        // SecCnt为0则表示256扇区

    // 写入lba地址，即扇区号
    outb(REG_LBA_L(channel), lba);      // 低8位

    outb(REG_LBA_M(channel), lba >> 8);     // 8 ~ 15位
    outb(REG_LBA_H(channel), lba >> 16);        // 16~23位

    // lba的24~27位要存储在device寄存器的0~3位，无法单独写入4位，故重写整个寄存器
    outb(REG_DEV(channel), BIT_DEV_MBS | BIT_DEV_LBA | (hd->devNo == 1 ? BIT_DEV_DEV : 0) | lba >> 24);
}

/*
* 向通道channel发送cmd命令
*/
static void CmdOut(IdeChannel* channel, uint8_t cmd) {
    channel->expectingIntr = true;
    outb(REG_CMD(channel), cmd);
}

/*
* 从硬盘读取secCnt个扇区到buf
*/
static void ReadFromSector(Disk* hd, void* buf, uint8_t secCnt) {
    uint32_t sizeInByte;
    if (secCnt == 0) {
        // 0就是256
        sizeInByte = 256 * 512;
    } else {
        sizeInByte = secCnt * 512;
    }
    insw(REG_DATA(hd->myChannel), buf, sizeInByte / 2);
}

/*
* 将buf中secCnt个扇区的数据写入硬盘
*/
static void WriteToSector(Disk* hd, void* buf, uint8_t secCnt) {
    uint32_t sizeInByte;
    if (secCnt == 0) {
        // 0就是256
        sizeInByte = 256 * 512;
    } else {
        sizeInByte = secCnt * 512;
    }
    outsw(REG_DATA(hd->myChannel), buf, sizeInByte / 2);
}

/*
* 等待硬盘数据获取完成，最多30秒
*/
static bool BusyWait(Disk* hd) {
    IdeChannel* channel = hd->myChannel;
    uint16_t timeLimit = 30 * 1000;
    while (timeLimit -= 10 >= 0) {
        // PrintK("awdwda\n");
        if (!(inb(REG_STATUS(channel)) & BIT_STAT_BSY)) {
            return (inb(REG_STATUS(channel)) & BIT_STAT_DRQ);
        } else {
            MilliTimeSleep(10);
        }
    }
    return false;
}

/*
* 从硬盘读取secCnt个扇区到buf
*/
void IdeRead(Disk* hd, uint32_t lba, void* buf, uint32_t secCnt) {
    ASSERT(lba <= MAX_LBA);
    ASSERT(secCnt > 0);
    LockAcquire(&hd->myChannel->lock);

    SelectDisk(hd);

    uint32_t secsOp;        // 每次操作的扇区数
    uint32_t secsDone = 0;      // 已完成的扇区数
    while(secsDone < secCnt) {
        if ((secsDone + 256) <= secCnt) {
            secsOp = 256;
        } else {
            secsOp = secCnt - secsDone;
        }

        SelectSector(hd, lba + secsDone, secsOp);

        CmdOut(hd->myChannel, CMD_READ_SECTOR);

        // 阻塞自己，等待中断处理例程将自己唤醒
        SemaDown(&hd->myChannel->diskDone);

        if (!BusyWait(hd)) {
            char error[64];
            sprintf(error, "%s read sector %d failed!!!!!!\n", hd->name, lba);
            PANIC(error);
        }
        // 从硬盘缓冲区中读出数据
        ReadFromSector(hd, (void*)((uint32_t)buf + secsDone * 512), secsOp);
        secsDone += secsOp;
    }
    LockRelease(&hd->myChannel->lock);
}

/*
* 将buf中secCnt个扇区的数据写入硬盘
*/
void IdeWrite(Disk* hd, uint32_t lba, void* buf, uint32_t secCnt) {
    ASSERT(lba <= MAX_LBA);
    ASSERT(secCnt > 0);
    LockAcquire(&hd->myChannel->lock);

    SelectDisk(hd);

    uint32_t secsOp;
    uint32_t secsDone = 0;
    while(secsDone < secCnt) {
        if ((secsDone + 256) <= secCnt) {
            secsOp = 256;
        } else {
            secsOp = secCnt - secsDone;
        }

        SelectSector(hd, lba + secsDone, secsOp);

        CmdOut(hd->myChannel, CMD_WRITE_SECTOR);

        SemaDown(&hd->myChannel->diskDone);

        if (!BusyWait(hd)) {
            char error[64];
            sprintf(error, "%s write sector %d failed!!!!!!\n", hd->name, lba);
            PANIC(error);
        }
        WriteToSector(hd, (void*)((uint32_t)buf + secsDone * 512), secsOp);

        // 阻塞自己，等待中断处理例程将自己唤醒
        SemaDown(&hd->myChannel->diskDone);
        secsDone += secsOp;
    }
    LockRelease(&hd->myChannel->lock);

}

/*
* 硬盘中断处理例程
*/
void IntrHdHandler(uint8_t irqNo) {
    ASSERT(irqNo == 0x2e || irqNo == 0x2f);
    uint8_t chNo = irqNo - 0x2e;
    IdeChannel* channel = &gChannels[chNo];
    ASSERT(channel->irqNo == irqNo);

    if (channel->expectingIntr) {
        channel->expectingIntr = false;
        SemaUp(&channel->diskDone);     // 唤醒等待数据的线程
        inb(REG_STATUS(channel));
    }
}

// 记录总扩展分区的起始lba，初始为0，PartitionScan时以此为标记
int32_t gExtLbaBase = 0;

// 记录硬盘主分区和逻辑分区的下标
uint8_t gPNo = 0, gLNo = 0;

// 分区队列
List gPartitionList;

/*
* 分区表项，16字节
*/
struct _PartitionTableEntry {
    uint8_t bootable;       // 是否可引导
    uint8_t startHead;      // 起始磁头号
    uint8_t startSec;       // 起始扇区号
    uint8_t startChs;       // 起始柱面号
    uint8_t fsType;     // 分区类型
    uint8_t endHead;        // 结束磁头号
    uint8_t endSec;     // 结束扇区号
    uint8_t endChs;     // 结束柱面号

    uint32_t startLBA;      // 本分区起始扇区的lba
    uint32_t secCnt;        // 本分区扇区数目
} __attribute__ ((packed));     // 保证结构体是16字节
typedef struct _PartitionTableEntry PartitionTableEntry;

/*
* 引导扇区结构
*/
struct _BootSector {
    uint8_t other[446];     // 引导代码
    PartitionTableEntry partitionTable[4];      // 分区表
    uint16_t signature;     // 55 AA
} __attribute__ ((packed));     // 保证结构体是512字节
typedef struct _BootSector BootSector;

/*
* 将dts中len个相邻字节交换位置后存入buf
*/
static void SwapPairsBytes(const char* dst, char* buf, uint32_t len) {
    uint8_t idx;
    for (idx = 0; idx < len; idx += 2) {
        buf[idx + 1] = *dst++;
        buf[idx] = *dst++;
    }
    buf[idx] = '\0';
}

/*
* 获得硬盘参数信息
*/
static void IdentifyDisk(Disk* hd) {
    char idInfo[512];
    SelectDisk(hd);
    CmdOut(hd->myChannel, CMD_IDENTIFY);
    
    // 向硬盘发送指令后便通过信号量阻塞自己，待硬盘处理完成后，通过中断处理程序将自己唤醒
    SemaDown(&hd->myChannel->diskDone);

    if (!BusyWait(hd)) {
        char error[64];
        sprintf(error, "%s Identify failed!!!!!!\n", hd->name);
        PANIC(error);
    }
    ReadFromSector(hd, idInfo, 1);

    char buf[64];
    uint8_t snStart = 10 * 2, snLen = 20, mdStart = 27 * 2, mdLen = 40;
    SwapPairsBytes(&idInfo[snStart], buf, mdLen);
    PrintK("  disk %s info:\n    SN: %s\n", hd->name, buf);
    memset(buf, 0, sizeof(buf));
    SwapPairsBytes(&idInfo[mdStart], buf, mdLen);
    PrintK("    MODULE: %s\n", buf);
    uint32_t sectors = *(uint32_t*)&idInfo[60 * 2];
    PrintK("    SECTORS: %d\n", sectors);
    PrintK("    CAPACITY: %dMB\n", sectors * 512 / 1024 / 1024);
}

/*
* 扫描硬盘hd中地址为ext_lba的扇区中的所有分区
*/
static void PartitionScan(Disk* hd, uint32_t extLBA) {
    BootSector* bs = SysMalloc(sizeof(BootSector));
    IdeRead(hd, extLBA, bs, 1);     // 读取该扇区
    uint8_t partIdx = 0;
    PartitionTableEntry* p = bs->partitionTable;
    while (partIdx++ < 4) {
        if (p->fsType == 0x5) {     // 是扩展分区
            if (gExtLbaBase != 0) {
                PartitionScan(hd, gExtLbaBase + p->startLBA);       // 递归扫描该扇区
            } else {
                // 第一次读取引导块，也就是主引导记录所在的扇区
                gExtLbaBase = p->startLBA;      // 记录lba基址，后面的扩展分区都要基于此基址
                PartitionScan(hd, p->startLBA);
            }
        } else if (p->fsType != 0 ) {       // 分区类型有效
            if (extLBA == 0) {      // 当前是主分区
                hd->primParts[gPNo].startLBA = extLBA + p->startLBA;
                hd->primParts[gPNo].secCnt = p->secCnt;
                hd->primParts[gPNo].myDisk = hd;
                ListAppend(&gPartitionList, &hd->primParts[gPNo].partTag);
                sprintf(hd->primParts[gPNo].name, "%s%d", hd->name, gPNo + 1);
                gPNo++;
                ASSERT(gPNo < 4);
            } else {
                hd->logicParts[gLNo].startLBA = extLBA + p->startLBA;
                hd->logicParts[gLNo].secCnt = p->secCnt;
                hd->logicParts[gLNo].myDisk = hd;
                ListAppend(&gPartitionList, &hd->logicParts[gLNo].partTag);
                sprintf(hd->logicParts[gLNo].name, "%s%d", hd->name, gLNo + 5);     // 逻辑分区数字是从5开始,主分区是1～4.
                gLNo++;
                if (gLNo >= 8) break;
            }
        }
        p++;
    }
    SysFree(bs);
}

/*
* 打印分区信息
*/
static bool PartitionInfo(ListElem* elem, int arg) {
    Partition* part = ELEM_TO_ENTRY(Partition, partTag, elem);
    PrintK("    %s k startLba:0x%x, secCnt:0x%x\n", part->name, part->startLBA, part->secCnt);
    return false;
}

/*
* 磁盘驱动初始化
*/
void IdeInit(void) {
    PrintK("IdeInit start\n");
    uint8_t hdCnt = *((uint8_t*)(0x475));       // 由bios写入的，硬盘数量
    ASSERT(hdCnt > 0);
    ListInit(&gPartitionList);
    gChannelCnt = DIV_ROUND_UP(hdCnt, 2);       // 一个ide通道有两个硬盘，反推有几个ide通道

    IdeChannel* channel;
    uint8_t channelNo = 0, devNo = 0;

    // 处理每个硬盘上的通道
    while(channelNo < gChannelCnt) {
        channel = &gChannels[channelNo];
        sprintf(channel->name, "ide%d", channelNo);
        switch (channelNo) {
        case 0:
            channel->portBase = 0x1f0;      // ide0通道起始端口号是0x1f0
            channel->irqNo = 0x20 + 14;     // 从片8259a上倒数第二个中断引脚，也就是ide0通道的中断向量号
            break;
        case 1:
            channel->portBase = 0x170;      // ide1通道起始端口号是0x170
            channel->irqNo = 0x20 + 15;     // 从片8259a上最后一个中断引脚，用来响应ide1通道上的硬盘中断
            break;
        }
        channel->expectingIntr = false;     // 未向硬盘写入指令时不期待硬盘的中断
        LockInit(&channel->lock);
        SemaInit(&channel->diskDone, 0);        // 初始化为0，向硬盘请求数据后，硬盘驱动down此信号量会阻塞线程，直到硬盘完成后发中断，中断处理程序up此信号量，唤醒线程

        IntrRegisterHandler(channel->irqNo, IntrHdHandler);

        while (devNo < 2) {
            
            Disk* hd = &channel->devices[devNo];
            hd->myChannel = channel;
            hd->devNo = devNo;
            sprintf(hd->name, "sd%c", 'a' + channelNo * 2 + devNo);
            IdentifyDisk(hd);       // 获取硬盘参数
            if (devNo != 0) {       // 跳过内核裸硬盘
                PartitionScan(hd, 0);       // 扫描硬盘分区
            }
            gPNo = 0, gLNo = 0;
            devNo++;
            
        }
        devNo = 0;      // 硬盘驱动号置0，为下一个channel的两个硬盘初始化
        channelNo++;
    }
    PrintK("    all partition info\n");
    ListTraversal(&gPartitionList, PartitionInfo, (int)NULL);
    PrintK("IdeInit done\n");
}