#include "bitmap.h"

#include "lib/string.h"
#include "lib/kernel/print.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"


/*
* 将位图数据填充为0
*/
void BitmapInit(Bitmap* btmp) {
    memset(btmp->bits, 0, btmp->btmpByteLen);
}

/*
* 判断bit_idx位是否位1
*/
bool BitmapScanTest(Bitmap* btmp, uint32 bitIdx) {
    uint32 byteIdx = bitIdx / 8;
    uint32 bitOdd = bitIdx % 8;
    return (btmp->bits[byteIdx] & (BITMAP_MASK << bitOdd));
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
        // 整个位图中没有找到为0的位
        return -1;
    }

    int32 idxBit = 0;
    while ((uint8)(BITMAP_MASK << idxBit) & btmp->bits[idxByte]) {
        // 在有空闲的字节中找出为0的位
        idxBit++;
    }

    int32 bitIdxStart = idxByte * 8 + idxBit;
    if (cnt == 1) {
        // 如果只需要1个0位，直接返回
        return bitIdxStart;
    }

    // 需要连续的多个0位
    uint32 bitLeft = (btmp->btmpByteLen * 8 - bitIdxStart);     // 剩余长度
    uint32 nextBit = bitIdxStart + 1;       // 0位的下一位的索引
    uint32 curCnt = 1;      // 当前有1个0位
    bitIdxStart = -1;

    while (bitLeft-- > 0) {
        if (!(BitmapScanTest(btmp, nextBit))) {
            curCnt++;
        } else {
            // 不连续，计数置0
            curCnt = 0;
        }

        if (curCnt == cnt) {
            // 连续0位满足要求
            bitIdxStart = nextBit - cnt + 1;
            break;
        }
        nextBit++;
    }
    return bitIdxStart;
}


/*
* 修改位图中指定位的值
*/
void BitmapSet(Bitmap* btmp, uint32 bitIdx, int8 value) {
    ASSERT((value == 0) || (value == 1));
    uint32 byteIdx = bitIdx / 8;
    uint32 bitOdd = bitIdx % 8;

    if (value) {
        // 置1
        btmp->bits[byteIdx] |= (BITMAP_MASK << bitOdd);
    } else {
        // 置0
        btmp->bits[byteIdx] &= ~(BITMAP_MASK << bitOdd);
    }
}