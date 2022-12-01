#ifndef KERNEL_BITMAP_H_
#define KERNEL_BITMAP_H_

#include "kernel/global.h"
#include "lib/stdint.h"

#define BITMAP_MASK 1

typedef struct _Bitmap {
    uint32 btmpByteLen;
    uint8* bits;
} Bitmap;

void BitmapInit(Bitmap* btmp);
void BitmapScanTest(Bitmap* btmp, uint32 bitIdx);
int BitmapScan(Bitmap* btmp, uint32 cnt);
void BitmapSet(Bitmap* btmp, uint32 bitIdx, int8 value);

#endif // KERNEL_BITMAP_H_