#ifndef KERNEL_BITMAP_H_
#define KERNEL_BITMAP_H_

#include "kernel/global.h"
#include "lib/stdint.h"

#define BITMAP_MASK 1

typedef struct _Bitmap {
    uint32_t btmpByteLen;     // 位图数据字节数
    uint8_t * bits;        // 指向位图数据
} Bitmap;

void BitmapInit(Bitmap* btmp);
bool BitmapScanTest(Bitmap* btmp, uint32_t bitIdx);
int32_t BitmapScan(Bitmap* btmp, uint32_t cnt);
void BitmapSet(Bitmap* btmp, uint32_t bitIdx, int8_t value);

#endif // KERNEL_BITMAP_H_