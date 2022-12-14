#ifndef KERNEL_MEMORY_H_
#define KERNEL_MEMORY_H_

#include "lib/stdint.h"
#include "thread/sync.h"
#include "lib/kernel/bitmap.h"


/*
* 虚拟地址池
*/
typedef struct _VirAddrPool {
    Bitmap vAddrBitmap;     // 位图
    uint32_t vAddrStart;      // 起始地址
} VirAddrPool;

/*
* 物理地址池
*/
typedef struct _PhyAddrPool {
    Bitmap poolBitmap;
    uint32_t phyAddrStart;
    uint32_t poolSize;
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

uint32_t* PtePtr(uint32_t virAddr);
uint32_t* PdePtr(uint32_t virAddr);
uint32_t AddrV2P(uint32_t vAddr);

void* GetKernelPages(uint32_t pgCnt);

void* GetAPage(PoolFlags pf, uint32_t vAddr);

/*
* 内存块，即被Arena管理的块，用于连接到MemBlockDesc的空闲链表，也是分配给用户的整块内存
*/
typedef struct _MemBlock {
    ListElem freeElem;      // 链接空闲节点，分配给用户时该节点就被摘除了，因此并不额外占用空间
} MemBlock;

/*
* 内存块描述符，描述内存块的规格，连接此所有属于当前描述符的Arena的空闲MemBlock
*/
typedef struct _MemBlockDesc {
    uint32_t blockSize;     // 内存块大小
    uint32_t blockPerArena;     // 当前Arena中可容纳的MemBlock的数量
    List freeList;      // 空闲MemBlock链表
} MemBlockDesc;

#define MEM_DESC_CNT 7      // 7种规格的内存块，16, 32, 64, 128, 256, 512, 1024

void BlockDescInit(MemBlockDesc* descArray);

void* SysMalloc(uint32_t size);
void SysFree(void* ptr);

#endif // KERNEL_MEMORY_H_