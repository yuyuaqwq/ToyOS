#include "bitmap.h"

#include "lib/string.h"
#include "lib/kernel/print.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"

void BitmapInit(Bitmap* btmp) {
    memset(btmp->bits, 0, btmp->btmpByteLen);
}

/*
* 判断bit_idx位是否位1
*/
bool BitmapScanTest(Bitmap* btmp, uint32 bitIdx) {
    uint32 byteIdx = bitIdx / 8;
    uint32 bitOdd = bitIdx % 8;
    uint32 bitOdd = bitIdx % 8;
    return (btmp->bits[byteIdx] & (BITMAP_MASK << bitOdd))
}

/*
* 在位图btmp中找到连续的cnt个可用位
* 返回起始空闲位下标
*/
int32 BitmapScan(Bitmap* btmp, uint32 cnt) {
    uint32 idxByte = 0;
    while (0xff == btmp->bits[idxByte] && idxByte < btmp->btmpByteLen) {
        idxByte++;
    }

    ASSERT(idxByte < btmp->btmpByteLen);
    if (idxByte == btmp->btmpByteLen) {
        return -1;
    }

    int32 idxBit = 0;
    while ((uint8)(BITMAP_MASK << idxBit) & btmp->bits[idxByte]) {
        idxBit++;
    }

    int32 bitIdxStart = idxByte * 8 + idxBit;
    if (cnt == 1) {
        return bitIdxStart;
    }

    uint32 bitLeft = (btmp->btmpByteLen * 8 - bitIdxStart);
    uint32 nextBit = bitIdxStart + 1;
    uint32 count = 1;
    bitIdxStart = -1;

    while (bitLeft-- > 0) {
        if (!(BitmapScanTest(btmp, nextBit))) {
            count++;
        } else {
            count = 0;
        }

        if (count == cnt) {
            bitIdxStart = nextBit - cnt + 1;
            break;
        }
        nextBit++;
    }
    return bitIdxStart;
}

void BitmapSet(Bitmap* btmp, uint32 bitIdx, int8 value) {
    ASSERT((value == 0) || (value == 1));
    uint32 byteIdx = bitIdx / 8;
    uint32 bitOdd = bitIdx % 8;

    if (value) {
        btmp->bits[byteIdx] |= (BITMAP_MASK << bitOdd);
    } else {
        btmp->bits[byteIdx] &= ~(BITMAP_MASK << bitOdd);
    }
}