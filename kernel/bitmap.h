#ifndef KERNEL_BITMAP_H_
#define KERNEL_BITMAP_H_

#include "kernel/global.h"
#include "lib/stdint.h"

#define BITMAP_MASK 1

typedef struct _Bitmap {
    uint32 btmpByteLen;     // 位图数据字节数
    uint8* bits;        // 指向位图数据
} Bitmap;

void BitmapInit(Bitmap* btmp);
bool BitmapScanTest(Bitmap* btmp, uint32 bitIdx);
int32 BitmapScan(Bitmap* btmp, uint32 cnt);
void BitmapSet(Bitmap* btmp, uint32 bitIdx, int8 value);

#endif // KERNEL_BITMAP_H_