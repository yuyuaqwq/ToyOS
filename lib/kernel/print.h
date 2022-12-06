#ifndef LIB_KERNEL_PRINT_H_
#define LIB_KERNEL_PRINT_H_
#include "lib/stdint.h"
void PutChar(uint8 charAscii);
void PutStr(uint8* message);
void PutInt(uint32 num);
void SetCursor(uint32 cursorPos);
#endif // LIB_KERNEL_PRINT_H_