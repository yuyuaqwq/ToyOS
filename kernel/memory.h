#ifndef KERNEL_MEMORY_H_
#define KERNEL_MEMORY_H_

#include "lib/stdint.h"
#include "thread/sync.h"
#include "kernel/bitmap.h"


/*
* 虚拟地址池
*/
typedef struct _VirAddrPool {
    Bitmap vAddrBitmap;     // 位图
    uint32 vAddrStart;      // 起始地址
} VirAddrPool;

/*
* 物理地址池
*/
typedef struct _PhyAddrPool {
    Bitmap poolBitmap;
    uint32 phyAddrStart;
    uint32 poolSize;

    Lock lock;
} PhyAddrPool;

typedef enum _PoolFlags {
    kPfKernel = 1,      // 内核内存池
    kPfUser = 2,        // 用户内存池
} PoolFlags;

#define PG_P_1 1
#define PG_P_0 0
#define PG_RW_R (0 << 1)
#define PG_RW_W (1 << 1)
#define PG_US_S (0 << 2)
#define PG_US_U (1 << 2)

#define PG_SIZE 4096

extern PhyAddrPool gKernelPhyAddrPool, gUserPhyAddrPool;
void MemInit(void);

uint32* PtePtr(uint32 virAddr);
uint32* PdePtr(uint32 virAddr);
uint32 AddrV2P(uint32 vAddr);

void* MallocPage(PoolFlags pf, uint32 pgCnt);

void* GetKernelPages(uint32 pgCnt);


#endif // KERNEL_MEMORY_H_