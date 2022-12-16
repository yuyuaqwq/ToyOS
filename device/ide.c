#include "ide.h"
#include "device/timer.h"
#include "kernel/interrupt.h"
#include "kernel/memory.h"
#include "kernel/debug.h"
#include "lib/kernel/stdio-kernel.h"
#include "lib/stdio.h"
#include "lib/kernel/io.h"


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

#define BIT_ALT_STAT_BSY 0x80
#define BIT_ALT_STAT_DRDY 0x40
#define BIT_ALT_STAT_DRQ 0x8

#define BIT_DEV_MBS 0xa0
#define BIT_DEV_LBA 0x40
#define BIT_DEV_DEV 0x10

#define CMD_IDENTIFY 0xec
#define CMD_READ_SECTOR 0x20
#define CMD_WRITE_SECTOR 0x30

#define MAX_LBA ((80*1024*1024/512) - 1)

uint8_t gChannelCnt;
IdeChannel channels[2];

static void SelectDisk(Disk* hd) {
    uint32_t regDevice = BIT_DEV_MBS | BIT_DEV_LBA;
    if (hd->devNo == 1) {
        regDevice |= BIT_DEV_DEV;
    }
    outb(REG_DEV(hd->myChannel), regDevice);
}

static void SelectSector(Disk* hd, uint32_t lba, uint32_t secCnt) {
    ASSERT(lba <= MAX_LBA);
    IdeChannel* channel = hd->myChannel;

    outb(REG_SECT_CNT(channel), secCnt);

    outb(REG_LBA_L(channel), lba);

    outb(REG_LBA_M(channel), lba >> 8);
    outb(REG_LBA_H(channel), lba >> 16);

    outb(REG_DEV(channel), BIT_DEV_MBS | BIT_DEV_LBA | (hd->devNo == 1 ? BIT_DEV_DEV : 0) | lba >> 24);
}

static void CmdOut(IdeChannel* channel, uint8_t cmd) {
    channel->expectingIntr = true;
    outb(REG_CMD(channel), cmd);
}

static void ReadFromSector(Disk* hd, void* buf, uint8_t secCnt) {
    uint32_t sizeInByte;
    if (secCnt == 0) {
        sizeInByte = 256 * 512;
    } else {
        sizeInByte = secCnt * 512;
    }
    insw(REG_DATA(hd->myChannel), buf, sizeInByte / 2);
}

static void WriteToSector(Disk* hd, void* buf, uint8_t secCnt) {
    uint32_t sizeInByte;
    if (secCnt == 0) {
        sizeInByte = 256 * 512;
    } else {
        sizeInByte = secCnt * 512;
    }
    outsw(REG_DATA(hd->myChannel), buf, sizeInByte / 2);
}

static bool BusyWait(Disk* hd) {
    IdeChannel* channel = hd->myChannel;
    uint16_t timeLimit = 30 * 1000;
    while (timeLimit -= 10 >= 0) {
        if (!(inb(REG_STATUS(channel)) & BIT_ALT_STAT_DRQ)) {
            return (inb(REG_STATUS(channel)) & BIT_ALT_STAT_DRQ);
        } else {
            MilliTimeSleep(10);
        }
    }
    return false;
}

void IdeRead(Disk* hd, uint32_t lba, void* buf, uint32_t secCnt) {
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

        CmdOut(hd->myChannel, CMD_READ_SECTOR);

        SemaDown(&hd->myChannel->diskDone);

        if (!BusyWait(hd)) {
            char error[64];
            sprintf(error, "%s read sector %d failed!!!!!!\n", hd->name, lba);
            PANIC(error);
        }
        ReadFromSector(hd, (void*)((uint32_t)buf + secsDone * 512), secsOp);
        secsDone += secsOp;
    }
    LockRelease(&hd->myChannel->lock);
}

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

        SemaDown(&hd->myChannel->diskDone);
        secsDone += secsOp;
    }
    LockRelease(&hd->myChannel->lock);

}

void IntrHdHandler(uint8_t irqNo) {
    ASSERT(irqNo == 0x2e || irqNo == 0x2f);
    uint8_t chNo = irqNo - 0x2e;
    IdeChannel* channel = &channels[chNo];
    ASSERT(channel->irqNo == irqNo);

    if (channel->expectingIntr) {
        channel->expectingIntr = false;
        SemaUp(&channel->diskDone);
        inb(REG_STATUS(channel));
    }
}

int32_t gExtLbaBase = 0;
uint8_t gPNo = 0, gLNo = 0;

List gPartitionList;

struct _PartitionTableEntry {
    uint8_t bootable;
    uint8_t startHead;
    uint8_t startSec;
    uint8_t startChs;
    uint8_t fsType;
    uint8_t endHead;
    uint8_t endSec;
    uint8_t endChs;

    uint32_t startLBA;
    uint32_t secCnt;
} __attribute__ ((packed));
typedef struct _PartitionTableEntry PartitionTableEntry;

struct _BootSector {
    uint8_t other[446];
    PartitionTableEntry partitionTable[4];
} __attribute__ ((packed));
typedef struct _BootSector BootSector;


static void SwpaPairsBytes(const char* dst, char* buf, uint32_t len) {
    uint8_t idx;
    for (idx = 0; idx < len; idx += 2) {
        buf[idx + 1] = *dst++;
        buf[idx] = *dst++;
    }
    buf[idx] = '\0';
}

static void IdentifyDisk(Disk* hd) {
    char idInfo[512];
    SelectDisk(hd);
    CmdOut(hd->myChannel, CMD_IDENTIFY);

    SemaDown(&hd->myChannel->diskDone);

    if (!BusyWait(hd)) {
        char error[64];
        sprintf(error, "%s Identify failed!!!!!!\n", hd->name);
        PANIC(error);
    }
    ReadFromSector(hd, idInfo, 1);

    char buf[64];
    uint8_t snStart = 10 * 2, snLen = 20, mdStart = 27 * 2, mdLen = 40;
    SwpaPairsBytes(&idInfo[mdStart], buf, mdLen);
    PrintK("    MODULE: %s\n", buf);
    uint32_t sectors = *(uint32_t*)&idInfo[60 * 2];
    PrintK("    SECTORS: %d\n", sectors);
    PrintK("    CAPACITY: %s\n", sectors * 512 / 1024 / 1024);
}

static void PartitionScan(Disk* hd, uint32_t extLBA) {
    BootSector* bs = SysMalloc(sizeof(BootSector));
    IdeRead(hd, extLBA, bs, 1);
    uint8_t partIdx = 0;
    PartitionTableEntry* p = bs->partitionTable;
    while (partIdx++ < 4) {
        if (p->fsType == 0x5) {
            if (gExtLbaBase != 0) {
                PartitionScan(hd, p->startLBA + gExtLbaBase);
            } else {
                gExtLbaBase = p->startLBA;
                PartitionScan(hd, p->startLBA);
            }
        } else if (p->fsType != 0 ) {
            if (extLBA == 0) {
                hd->primParts[gPNo].startLBA = extLBA + p->startLBA;
                hd->primParts[gPNo].secCnt = p->secCnt;
                hd->primParts[gPNo].myDisk = hd;
                ListAppend(&gPartitionList, &hd->primParts[gPNo].partTag);
                sprintf(hd->primParts[gPNo].name, "%s%d", hd->name, gPNo + 1);
                gPNo++;
                ASSERT(gPNo < 4);
            } else {
                hd->logicParts[gLNo].startLBA = extLBA + p->startLBA;
                hd->primParts[gLNo].secCnt = p->secCnt;
                hd->primParts[gLNo].myDisk = hd;
                ListAppend(&gPartitionList, &hd->primParts[gLNo].partTag);
                sprintf(hd->primParts[gLNo].name, "%s%d", hd->name, gLNo + 1);
                gLNo++;
                if (gLNo >= 8) break;
            }
        }
        p++;
    }
    SysFree(bs);
}

static bool PartitionInfo(ListElem* elem, int arg) {
    Partition* part = ELEM_TO_ENTRY(Partition, partTag, elem);
    PrintK("    %k start lba:0x%x, secCnt:0x%x\n", part->name, part->startLBA, part->secCnt);
    return false;
}

void IdeInit(void) {
    PrintK("IdeInit start\n");
    uint8_t hdCnt = *((uint8_t*)(0x475));
    ASSERT(hdCnt > 0);
    ListInit(&gPartitionList);
    gChannelCnt = DIV_ROUND_UP(hdCnt, 2);

    IdeChannel* channel;
    uint8_t channelNo = 0, devNo = 0;

    while(channelNo < gChannelCnt) {
        channel = &channels[channelNo];
        sprintf(channel->name, "ide%d", channelNo);

        switch (channelNo) {
        case 0:
            channel->portBase = 0x1f0;
            channel->irqNo = 0x20 + 14;
            break;
        case 1:
            channel->portBase = 0x170;
            channel->irqNo = 0x20 + 15;
            break;
        }
        channel->expectingIntr = false;
        LockInit(&channel->lock);
        SemaInit(&channel->diskDone, 0);

        IntrRegisterHandler(channel->irqNo, IntrHdHandler);

        while (devNo < 2) {
            Disk* hd = &channel->devices[devNo];
            hd->myChannel = channel;
            hd->devNo = devNo;
            sprintf(hd->name, "sd%c", 'a' + channelNo * 2 + devNo);
            IdentifyDisk(hd);
            if (devNo != 0) {
                PartitionScan(hd, 0);
            }
            gPNo = 0, gLNo = 0;
            devNo++;
            
        }
        devNo = 0;
        channelNo++;
    }
    PrintK("    all partition info\n");
    ListTraversal(&gPartitionList, PartitionInfo, (int)NULL);
    PrintK("IdeInit done\n");
}