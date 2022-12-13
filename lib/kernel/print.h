#ifndef LIB_KERNEL_PRINT_H_
#define LIB_KERNEL_PRINT_H_
#include "lib/stdint.h"
void PutChar(uint8_t charAscii);
void PutStr(uint8_t * message);
void PutInt(uint32_t num);
void SetCursor(uint32_t cursorPos);
#endif // LIB_KERNEL_PRINT_H_