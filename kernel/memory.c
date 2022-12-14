#include "memory.h"

#include "lib/string.h"
#include "lib/kernel/print.h"
#include "kernel/debug.h"
#include "thread/thread.h"
#include "kernel/interrupt.h"



#define MEM_BITMAP_BASE 0xc009a000
#define K_HEAP_START 0xc0100000


PhyAddrPool gKernelPhyAddrPool, gUserPhyAddrPool;
VirAddrPool gKernelVirAddrPool;




/*
* 获取PDI
*/
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
/*
* 获取PTI
*/
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)


/*
* 得到虚拟地址virAddr对应的pte指针
*/
uint32_t* PtePtr(uint32_t virAddr) {
    uint32_t* pte = (uint32_t*)((0xffc00000) + ((virAddr & 0xffc00000) >> 10) + PTE_IDX(virAddr) * 4);
    return pte;
}

/*
* 得到虚拟地址virAddr对应的pde指针
*/
uint32_t* PdePtr(uint32_t virAddr) {
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(virAddr) * 4);
    return pde;
}

/*
* 将虚拟地址转换为物理地址
*/
uint32_t AddrV2P(uint32_t vAddr) {   
    uint32_t* pte = PtePtr(vAddr);
    return ((*pte & 0xfffff000) + (vAddr & 0x00000fff));
}


/*
* 从物理内存池中分配1个物理页
* 成功返回该物理页面的物理地址，失败返回NULL
*/
static void* PhyAlloc(PhyAddrPool* pool) {
    int32_t bitIdx = BitmapScan(&pool->poolBitmap, 1);
    if (bitIdx == -1) {
        return NULL;
    }
    BitmapSet(&pool->poolBitmap, bitIdx, 1);
    uint32_t pagePhyAddr = ((bitIdx * PG_SIZE) + pool->phyAddrStart);
    return (void*)pagePhyAddr;
}

/*
* 释放物理页
*/
static void PhyFree(uint32_t pagePhyAddr) {
    PhyAddrPool* memPool;
    uint32_t bitIdx = 0;
    if (pagePhyAddr >= gUserPhyAddrPool.phyAddrStart) {
        ASSERT(pagePhyAddr >= gUserPhyAddrPool.phyAddrStart && pagePhyAddr < gUserPhyAddrPool.phyAddrStart + gUserPhyAddrPool.poolSize)
        // 是用户物理内存池
        memPool = &gUserPhyAddrPool;
        bitIdx = (pagePhyAddr - gUserPhyAddrPool.phyAddrStart) / PG_SIZE;
    } else {
        ASSERT(pagePhyAddr >= gKernelPhyAddrPool.phyAddrStart && pagePhyAddr < gKernelPhyAddrPool.phyAddrStart + gKernelPhyAddrPool.poolSize)
        // PutInt(pagePhyAddr); PutStr("q ");
        memPool = &gKernelPhyAddrPool;
        bitIdx = (pagePhyAddr - gKernelPhyAddrPool.phyAddrStart) / PG_SIZE;
    }
    
    BitmapSet(&memPool->poolBitmap, bitIdx, 0);
}


/*
* 为虚拟页面与物理页面建立映射
*/
static void PageTableAdd(void* virBase_, void* phyBase_) {
    uint32_t virBase = (uint32_t)virBase_, phyBase = (uint32_t)phyBase_;
    uint32_t* pde = PdePtr(virBase);
    uint32_t* pte = PtePtr(virBase);

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
        uint32_t pdePhyBase = (uint32_t)PhyAlloc(&gKernelPhyAddrPool);
        *pde = (pdePhyBase | PG_US_U | PG_RW_W | PG_P_1);       // 页目录项指向页表
        memset((void*)((uint32_t)pte & 0xfffff000), 0, PG_SIZE);      // 上面已经将页目录项指向新分配的页表，这里直接将pte转成ptt，再清0页面
        ASSERT(!(*pte & PG_P_1));
        *pte = (phyBase | PG_US_U | PG_RW_W | PG_P_1);

    }
}

/*
* 从页表中删除虚拟地址的映射(只移除pte)
*/
static void PageTablePteRemove(uint32_t vAddr) {
    uint32_t* pte = PtePtr(vAddr);
    *pte &= ~PG_P_1;
    asm volatile("invlpg %0"::"m"(vAddr):"memory");     // 更新tlb
}

/*
* 从虚拟内存池中分配连续的虚拟页
* 成功返回该虚拟页面的虚拟地址，失败返回NULL
*/
static void* VirAddrAlloc(PoolFlags pf, uint32_t pgCnt) {
    int virAddrStart = 0, bitIdxStart = -1;
    uint32_t cnt = 0;
    if (pf == kPfKernel) {
        // 内核内存池
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
        // 用户内存池
        TaskStruct* cur = RunningThread();
        bitIdxStart = BitmapScan(&cur->userprogVAddr.vAddrBitmap, pgCnt);
        if (bitIdxStart == -1) {
            return NULL;
        }
        while (cnt < pgCnt) {
            BitmapSet(&cur->userprogVAddr.vAddrBitmap, bitIdxStart + cnt++, 1);
        }
        virAddrStart = cur->userprogVAddr.vAddrStart + bitIdxStart * PG_SIZE;
        ASSERT((uint32_t)virAddrStart < (0xc0000000 - PG_SIZE));
    }
    return virAddrStart;
}

/*
* 从虚拟内存池中删除连续的虚拟页
*/
static void VirAddrRemove(PoolFlags pf, void* vAddr_, uint32_t pgCnt) {
    uint32_t bitIdxStart = 0, vAddr = (uint32_t)vAddr_, i = 0;
    if (pf == kPfKernel) {
        bitIdxStart = (vAddr - gKernelVirAddrPool.vAddrStart) / PG_SIZE;
        while (i < pgCnt) {
            BitmapSet(&gKernelVirAddrPool.vAddrBitmap, bitIdxStart + i++, 0);
        }
    } else {
        TaskStruct* curThread = RunningThread();
        bitIdxStart = (vAddr - curThread->userprogVAddr.vAddrStart) / PG_SIZE;
        while (i < pgCnt) {
            BitmapSet(&curThread->userprogVAddr.vAddrBitmap, bitIdxStart + i++, 0);
        }
    }
}


/*
* 分配虚拟内存，以页大小为单位
* 成功返回虚拟地址，失败返回NULL
*/
static void* MemAllocPage(PoolFlags pf, uint32_t pgCnt) {
    ASSERT(pgCnt > 0 && pgCnt < 3840);
    void* virAddrStart = VirAddrAlloc(pf, pgCnt);
    if (virAddrStart == NULL) {
        return NULL;
    }
    uint32_t virAddr = (uint32_t)virAddrStart, cnt = pgCnt;
    PhyAddrPool* pool = pf == kPfKernel ? &gKernelPhyAddrPool : &gUserPhyAddrPool;
    while (cnt-- > 0) {
        void* pagePhyBase = PhyAlloc(pool);
        if (pagePhyBase == NULL) {
            // 分配失败
            // 回滚已映射的物理页，待补充
            return NULL;
        }
        PageTableAdd((void*)virAddr, pagePhyBase);
        virAddr += PG_SIZE;
    }
    return virAddrStart;
}

/*
* 释放虚拟内存，以页大小为单位
*/
static void MemFreePage(PoolFlags pf, void* vAddr_, uint32_t pgCnt) {
    uint32_t pagePhyAddr;
    uint32_t vAddr = (uint32_t)vAddr_;
    ASSERT(pgCnt >= 1 && vAddr % PG_SIZE == 0);
    pagePhyAddr = AddrV2P(vAddr);
    ASSERT((pagePhyAddr % PG_SIZE) == 0 && pagePhyAddr >= 0x102000);
    if (pagePhyAddr >= gUserPhyAddrPool.phyAddrStart) {
        ASSERT(pf == kPfUser);
        for (int i = 0; i < pgCnt; i++) {
            pagePhyAddr = AddrV2P(vAddr);
            ASSERT(pagePhyAddr != 0);

            ASSERT((pagePhyAddr % PG_SIZE) == 0 && pagePhyAddr >= gUserPhyAddrPool.phyAddrStart);
        
            PhyFree(pagePhyAddr);
            PageTablePteRemove(vAddr);

            vAddr += PG_SIZE;
        }
        VirAddrRemove(pf, vAddr_, pgCnt);
        
    } else {
        ASSERT(pf == kPfKernel);
        for (int i = 0; i < pgCnt; i++) {
            pagePhyAddr = AddrV2P(vAddr);
            ASSERT(pagePhyAddr != 0);
            ASSERT((pagePhyAddr % PG_SIZE) == 0 && pagePhyAddr >= gKernelPhyAddrPool.phyAddrStart && pagePhyAddr < gUserPhyAddrPool.phyAddrStart);
            PhyFree(pagePhyAddr);
            PageTablePteRemove(vAddr);
            vAddr += PG_SIZE;
        }
        VirAddrRemove(pf, vAddr_, pgCnt);
    }
}

/*
* 从内核物理内存池申请连续虚拟页面
* 成功返回虚拟地址，失败返回NULL
*/
void* GetKernelPages(uint32_t pgCnt) {
    LockAcquire(&gKernelPhyAddrPool.lock);
    void* virAddr = MemAllocPage(kPfKernel, pgCnt);
    if (virAddr != NULL) {
        memset(virAddr, 0, pgCnt * PG_SIZE);
    }
    LockRelease(&gKernelPhyAddrPool.lock);
    return virAddr;
}

/*
* 从用户物理内存池申请连续虚拟页面
* 成功返回虚拟地址，失败返回NULL
*/
void* GetUserPages(uint32_t pgCnt) {
    LockAcquire(&gUserPhyAddrPool.lock);
    void* vAddr = MemAllocPage(kPfUser, pgCnt);
    memset(vAddr, 0, pgCnt * PG_SIZE);
    LockRelease(&gUserPhyAddrPool.lock);
    return vAddr;
}


/*
* 指定vAddr映射到新分配的物理页面，仅支持一页空间分配
*/
void* GetAPage(PoolFlags pf, uint32_t vAddr) {
    PhyAddrPool* memPool = pf == kPfKernel ? &gKernelPhyAddrPool : &gUserPhyAddrPool;
    LockAcquire(&memPool->lock);
    TaskStruct* cur = RunningThread();
    int32_t bitIdx = -1;
    if (cur->pgDir != NULL && pf == kPfUser) {
        bitIdx = (vAddr - cur->userprogVAddr.vAddrStart) / PG_SIZE;
        ASSERT(bitIdx > 0);
        BitmapSet(&cur->userprogVAddr.vAddrBitmap, bitIdx, 1);
    } else if (cur->pgDir == NULL && pf == kPfKernel) {
        bitIdx = (vAddr - gKernelVirAddrPool.vAddrStart) / PG_SIZE;
        ASSERT(bitIdx > 0);
        BitmapSet(&gKernelVirAddrPool.vAddrBitmap, bitIdx, 1);
    } else {
        PANIC("GetAPage:not allow kernel alloc userspace or user alloc kernelspace by GetAPage");
    }

    void* pagePhyAddr = PhyAlloc(memPool);
    if (pagePhyAddr == NULL) {
        return NULL;
    }
    PageTableAdd((void*)vAddr, pagePhyAddr);
    LockRelease(&memPool->lock);
    return (void*)vAddr;
}




static void MemPoolInit(uint32_t memByteTotal) {
    PutStr("    MemPoolInit Start\n");

    uint32_t pageTableSize = PG_SIZE * 256;       // 内核总页表大小，0x768~0x1023共256个页面，1M
    uint32_t usedMem = 0x100000 + pageTableSize;      // 已使用内存字节数，低端1M空间及内核页表总大小
    uint32_t freeMem = memByteTotal - usedMem;        // 目前可用内存字节数
    uint16_t allFreePages = freeMem / PG_SIZE;        // 可用内存的页数

    uint16_t kernelFreePages = allFreePages / 2;      // 内核空闲物理页数
    uint16_t userFreePages = allFreePages - kernelFreePages;      // 用户空闲物理页数

    uint32_t kbmLength = kernelFreePages / 8;     // 内核位图字节数，简单起见忽略余数
    uint32_t ubmLength = userFreePages / 8;     // 用户位图字节数

    uint32_t kpStart = usedMem;       // 内核物理池起始地址，即跳过低端1M空间及内核页表
    uint32_t upStart = kpStart + kernelFreePages * PG_SIZE;       // 用户物理池起始地址，紧挨着内核物理池

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
    BitmapInit(&gKernelPhyAddrPool.poolBitmap);
    BitmapInit(&gUserPhyAddrPool.poolBitmap);
    LockInit(&gKernelPhyAddrPool.lock);
    LockInit(&gUserPhyAddrPool.lock);


    PutStr("    KernelPoolBitmapStart:0x");
    PutInt((uint32_t)gKernelPhyAddrPool.poolBitmap.bits);
    PutStr("  KernelPoolPhyAddrStart:0x");
    PutInt(gKernelPhyAddrPool.phyAddrStart);
    PutChar('\n');

    PutStr("UserPoolBitmapStart:0x");
    PutInt((uint32_t)gUserPhyAddrPool.poolBitmap.bits);
    PutStr("  UserPoolPhyAddrStart:0x");
    PutInt(gUserPhyAddrPool.phyAddrStart);
    PutChar('\n');

    
    // 放到物理池位图之后???应该会出问题，按照设计，只有4页内存用来存放位图数据，共512M
    gKernelVirAddrPool.vAddrBitmap.btmpByteLen = kbmLength;
    gKernelVirAddrPool.vAddrBitmap.bits = (void*)(MEM_BITMAP_BASE + kbmLength + ubmLength);
    gKernelVirAddrPool.vAddrStart = K_HEAP_START;       // 起始可分配地址
    BitmapInit(&gKernelVirAddrPool.vAddrBitmap);
    PutStr("    MemPoolInit done\n");
}




/*
* 内存仓库，管理相同规格的MemBlock，通常管理一页(超过1024时一个Arena可能管理多页，但依旧只有一个MemBlock)
*/
typedef struct _Arena {
    MemBlockDesc* desc;     // 当前Arena所属的MemBlockDesc
    uint32_t cnt;       // large为true时，cnt表示页框数，否则表示空闲MemBlock数量
    bool large;
    // char data[];     // 余下就是数据区域，按照内存规格划分为一块一块的内存块，每块内存块头部就是MemBlock，但分配使用时MemBlock所占用的空间也会给用户使用，故不额外占用空间
} Arena;

// 内核内存块描述符数组
MemBlockDesc gKernelBlockDescs[MEM_DESC_CNT];

void BlockDescInit(MemBlockDesc* descArray) {
    uint16_t descIdx, blockSize = 16;

    // 初始化7种规格的内存块描述符
    for (descIdx = 0; descIdx < MEM_DESC_CNT; descIdx++) {
        descArray[descIdx].blockSize = blockSize;
        descArray[descIdx].blockPerArena = (PG_SIZE - sizeof(Arena)) / blockSize;       // 除去每个页面首部的Arena
        ListInit(&descArray[descIdx].freeList);
        blockSize *= 2;     // 下一规格的内存块大小
    }
}

static MemBlock* Arena2Block(Arena* a, uint32_t idx) {
    return (MemBlock*)((uint32_t)a + sizeof(Arena) + idx * a->desc->blockSize);     // 略过Arena首部，定位到该索引对应的位置
}

static Arena* Block2Arena(MemBlock* b) {
    return (Arena*)((uint32_t)b & 0xfffff000);      // 页面首部即Arena的位置
}

void* SysMalloc(uint32_t size) {
    PoolFlags pf;
    PhyAddrPool* memPool;
    uint32_t poolSize;
    MemBlockDesc* descs;
    TaskStruct* curThread = RunningThread();

    if (curThread->pgDir == NULL) {
        pf = kPfKernel;
        poolSize = gKernelPhyAddrPool.poolSize;
        memPool = &gKernelPhyAddrPool;
        descs = gKernelBlockDescs;
    } else {
        pf = kPfUser;
        poolSize = gUserPhyAddrPool.poolSize;
        memPool = &gUserPhyAddrPool;
        descs = curThread->userBlockDesc;
    }

    if (!(size > 0 && size < poolSize)) {
        return NULL;
    }

    Arena* a;
    MemBlock* b;
    LockAcquire(&memPool->lock);

    if (size > 1024) {
        // 超过1024字节，整页分配
        uint32_t pageCnt = DIV_ROUND_UP(size + sizeof(Arena), PG_SIZE);

        // 前12字节是Arean头，存放元信息
        a = MemAllocPage(pf, pageCnt);

        if (a != NULL) {
            memset(a, 0, pageCnt * PG_SIZE);
            a->desc = NULL;     // 不属于7种规格的内存块描述符
            a->cnt = pageCnt;
            a->large = true;
            LockRelease(&memPool->lock);
            return (void*)(a + 1);      // 跨过Arean头，剩下是可用空间
        } else {
            LockRelease(&memPool->lock);
            return NULL;
        }
    }
    else {
        uint32_t descIdx;
        // 从7种规格中查找合适的内存大小
        for (descIdx = 0; descIdx < MEM_DESC_CNT; descIdx++) {
            if (size <= descs[descIdx].blockSize) {
                break;
            }
        }
        if (ListEmpty(&descs[descIdx].freeList)) {
            // 此规格的内存块已经消耗完毕
            a = MemAllocPage(pf, 1);      // 分配一页内存创建新的Arena
            if (a == NULL) {
                LockRelease(&memPool->lock);
                return NULL;
            }
            memset(a, 0, PG_SIZE);

            // 当前页面的Arena信息初始化
            a->desc = &descs[descIdx];
            a->large = false;
            a->cnt = descs[descIdx].blockPerArena;
            uint32_t blockIdx;

            IntrStatus oldStatus = IntrDisable();
            // 数据区拆分为内存块
            for (blockIdx = 0; blockIdx < descs[descIdx].blockPerArena; blockIdx++) {
                b = Arena2Block(a, blockIdx);       // 每个内存块中，前面一部分是MemBlock头
                ASSERT(!ElemFind(&a->desc->freeList, &b->freeElem));
                ListAppend(&a->desc->freeList, &b->freeElem);       // 分好的内存块插到空闲链表中
            }
            IntrSetStatus(oldStatus);
        }

        // 分配一块MemBlock，与MemBlock头重叠并无关系，可以直接给用户使用，因为已经脱链了节点也就无效了，也不影响后续回收(直接通过释放时的指针得到Arena，即可得到当前内存块规格)
        b = ELEM_TO_ENTRY(MemBlock, freeElem, ListPop(&(descs[descIdx].freeList)));
        memset(b, 0, descs[descIdx].blockSize);

        a = Block2Arena(b);
        a->cnt--;
        LockRelease(&memPool->lock);
        return (void*)b;
    }
}

void SysFree(void* ptr) {
    ASSERT(ptr != NULL);
    if (ptr != NULL) {
        PoolFlags pf;
        PhyAddrPool* memPool;

        if (RunningThread()->pgDir == NULL) {
            ASSERT((uint32_t)ptr >= K_HEAP_START);
            pf = kPfKernel;
            memPool = &gKernelPhyAddrPool;
        } else {
            pf = kPfUser;
            memPool = &gUserPhyAddrPool;
        }
        LockAcquire(&memPool->lock);
        MemBlock* b = ptr;
        Arena* a = Block2Arena(b);
        ASSERT(a->large == true || a->large == false);
        if (a->desc == NULL && a->large == true) {
            // 大于1024字节的整块内存
            MemFreePage(pf, a, a->cnt);
        } else {
            ASSERT(a->desc);
            IntrStatus oldStatus = IntrDisable();
            // 挂接到所属内存块描述符的空闲链表中
            ListAppend(&a->desc->freeList, &b->freeElem);
            // 如果当前Arena中所有内存块都是空闲，则释放该Arena
            if (++a->cnt == a->desc->blockPerArena) {
                for (uint32_t blockIdx = 0; blockIdx < a->desc->blockPerArena; blockIdx++) {
                    MemBlock* b = Arena2Block(a, blockIdx);
                    ASSERT(ElemFind(&a->desc->freeList, &b->freeElem));
                    ListRemove(&b->freeElem);
                }
                MemFreePage(pf, a, 1);
            }
            IntrSetStatus(oldStatus);
        }
        LockRelease(&memPool->lock);
    }
}


void MemInit(void) {
    PutStr("MemInit start\n");
    uint32_t memByteTotal = (*(uint32_t*)(0xb00));
    PutStr("memByteTotal:0x");PutInt(memByteTotal); PutChar('\n');
    MemPoolInit(memByteTotal);
    BlockDescInit(gKernelBlockDescs);
    PutStr("MemInit done\n");
}