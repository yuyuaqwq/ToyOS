#include "memory.h"

#include "lib/kernel/print.h"
#include "kernel/debug.h"



#define MEM_BITMAP_BASE 0xc009a000
#define K_HEAP_START 0xc0100000


PhyAddrPool gKernelPhyAddrPool, gUserPhyAddrPool;
VirAddrPool gKernelVirAddrPool;

static void MemPoolInit(uint32 memByteTotal) {
    PutStr("    MemPoolInit Start\n");
    uint32 pageTableSize = PG_SIZE * 256;       // 内核总页表大小，0x768~0x1023共256个页面，1M
    uint32 usedMem = 0x100000 + pageTableSize;      // 已使用内存字节数，低端1M空间及内核页表总大小
    uint32 freeMem = memByteTotal - usedMem;        // 目前可用内存字节数
    uint16 allFreePages = freeMem / PG_SIZE;        // 可用内存的页数

    uint16 kernelFreePages = allFreePages / 2;      // 内核空闲物理页数
    uint16 userFreePages = allFreePages - kernelFreePages;      // 用户空闲物理页数

    uint32 kbmLength = kernelFreePages / 8;     // 内核位图字节数，简单起见忽略余数
    uint32 ubmLength = userFreePages / 8;     // 用户位图字节数

    uint32 kpStart = usedMem;       // 内核物理池起始地址，即跳过低端1M空间及内核页表
    uint32 upStart = kpStart + kernelFreePages * PG_SIZE;       // 用户物理池起始地址，紧挨着内核物理池

    // 保存到全局对象中
    gKernelPhyAddrPool.phyAddrStart = kpStart;
    gUserPhyAddrPool.phyAddrStart = upStart;

    gKernelPhyAddrPool.poolSize = kernelFreePages * PG_SIZE;
    gUserPhyAddrPool.poolSize = userFreePages * PG_SIZE;

    gKernelPhyAddrPool.poolBitmap.btmpByteLen = kbmLength;
    gUserPhyAddrPool.poolBitmap.btmpByteLen = ubmLength;

    // 指向对应的位图数据
    gKernelPhyAddrPool.poolBitmap.bits = (void*)MEM_BITMAP_BASE;
    gUserPhyAddrPool.poolBitmap.bits = (void*)(MEM_BITMAP_BASE + kbmLength);

    PutStr("    KernelPoolBitmapStart:0x");
    PutInt((uint32)gKernelPhyAddrPool.poolBitmap.bits);
    PutStr("  KernelPoolPhyAddrStart:0x");
    PutInt(gKernelPhyAddrPool.phyAddrStart);
    PutChar('\n');

    PutStr("UserPoolBitmapStart:0x");
    PutInt((uint32)gUserPhyAddrPool.poolBitmap.bits);
    PutStr("  UserPoolPhyAddrStart:0x");
    PutInt(gUserPhyAddrPool.phyAddrStart);
    PutChar('\n');

    BitmapInit(&gKernelPhyAddrPool.poolBitmap);
    BitmapInit(&gUserPhyAddrPool.poolBitmap);

    gKernelVirAddrPool.vAddrBitmap.btmpByteLen = kbmLength;

    gKernelVirAddrPool.vAddrBitmap.bits = (void*)(MEM_BITMAP_BASE + kbmLength + ubmLength);

    gKernelVirAddrPool.vAddrStart = K_HEAP_START;
    BitmapInit(&gKernelVirAddrPool.vAddrBitmap);
    PutStr("    MemPoolInit done\n");
}

void MemInit(void) {
    PutStr("MemInit start\n");
    uint32 memByteTotal = (*(uint32*)(0xb00));
    PutStr("memByteTotal:0x");PutInt(memByteTotal); PutChar('\n');
    MemPoolInit(memByteTotal);
    PutStr("MemInit done\n");
}


/*
* 获取PDI
*/
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
/*
* 获取PTI
*/
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

/*
* 从虚拟内存池中分配连续的虚拟页
* 成功返回该虚拟页面的虚拟地址，失败返回NULL
*/
static void* VirAddrGet(PoolFlags pf, uint32 pgCnt) {
    int virAddrStart = 0, bitIdxStart = -1;
    uint32 cnt = 0;
    if (pf == kPfKernel) {
        bitIdxStart = BitmapScan(&gKernelVirAddrPool.vAddrBitmap, pgCnt);
        if (bitIdxStart == -1) {
            return NULL;
        }
        while (cnt < pgCnt) {
            BitmapSet(&gKernelVirAddrPool.vAddrBitmap, bitIdxStart + cnt++, 1);
        }
        virAddrStart = gKernelVirAddrPool.vAddrStart + bitIdxStart * PG_SIZE;
    }
    else {
        // 用户内存池，未来再补充
    }
    return virAddrStart;
}

/*
* 得到虚拟地址virAddr对应的pte指针
*/
uint32* PtePtr(uint32 virAddr) {
    uint32* pte = (uint32*)((0xffc00000) + ((virAddr & 0xffc00000) >> 10) + PTE_IDX(virAddr) * 4);
    return pte;
}

/*
* 得到虚拟地址virAddr对应的pde指针
*/
uint32* PdePtr(uint32 virAddr) {
    uint32* pde = (uint32*)((0xfffff000) + PDE_IDX(virAddr) * 4);
    return pde;
}

/*
* 从物理内存池中分配1个物理页
* 成功返回该物理页面的物理地址，失败返回NULL
*/
static void* PhyAlloc(PhyAddrPool* pool) {
    int32 bitIdx = BitmapScan(&pool->poolBitmap, 1);
    if (bitIdx == -1) {
        return NULL;
    }
    BitmapSet(&pool->poolBitmap, bitIdx, 1);
    uint32 pagePhyAddr = ((bitIdx * PG_SIZE) + pool->phyAddrStart);
    return (void*)pagePhyAddr;
}

/*
* 为虚拟页面与物理页面建立映射
*/
static void PageTableAdd(void* virBase_, void* phyBase_) {
    uint32 virBase = virBase_, phyBase = phyBase_;
    uint32* pde = PdePtr(virBase);
    uint32* pte = PtePtr(virBase);

    if (*pde & PG_P_1) {        // 判断pde是否已分配ptt

        // 此目录项已存在对应的页表
        ASSERT(!(*pte & PG_P_1));       // 因为是建立映射，页表项不应该有效

        if (!(*pte & PG_P_1)) {
            *pte = (phyBase | PG_US_U | PG_RW_W | PG_P_1);
        } else {
            PANIC("pte repate");
            *pte = (phyBase | PG_US_U | PG_RW_W | PG_P_1);
        }
    } else {
        // 此目录项不存在对应的页表，需要创建页表
        uint32 pdePhyBase = (uint32)PhyAlloc(&gKernelPhyAddrPool);
        *pde = (pdePhyBase | PG_US_U | PG_RW_W | PG_P_1);       // 页目录项指向页表
        memset((void*)((uint32)pte & 0xfffff000), 0, PG_SIZE);      // 上面已经将页目录项指向新分配的页表，这里直接将pte转成ptt，再清0页面

        ASSERT(!(*pte & PG_P_1));
        *pte = (phyBase | PG_US_U | PG_RW_W | PG_P_1);
    }
}

/*
* 分配虚拟内存，以页大小为单位
* 成功返回虚拟地址，失败返回NULL
*/
void* MallocPage(PoolFlags pf, uint32 pgCnt) {
    ASSERT(pgCnt > 0 && pgCnt < 3840);
    void* virAddrStart = VirAddrGet(pf, pgCnt);
    if (virAddrStart == NULL) {
        return NULL;
    }

    uint32 virAddr = (uint32)virAddrStart, cnt = pgCnt;
    PhyAddrPool* pool = pf == kPfKernel ? &gKernelPhyAddrPool : &gUserPhyAddrPool;
    while (cnt-- > 0) {
        void* pagePhyBase = PhyAlloc(pool);
        if (pagePhyBase == NULL) {
            // 分配失败
            // 回滚已映射的物理页
            return NULL;
        }
        PageTableAdd((void*)virAddr, pagePhyBase);
        virAddr += PG_SIZE;
    }
    return virAddrStart;
}


/*
* 从内核物理内存池申请连续虚拟页面
* 成功返回虚拟地址，失败返回NULL
*/
void* GetKernelPages(uint32 pgCnt) {
    void* virAddr = MallocPage(kPfKernel, pgCnt);
    if (virAddr != NULL) {
        memset(virAddr, 0, pgCnt * PG_SIZE);
    }
    return virAddr;
}