#include "debug.h"

#include "lib/kernel/print.h"
#include "kernel/interrupt.h"

/*
* 打印文件名、行号、函数名、条件并使程序悬停
*/
void PanicSpin(char* fileName, int line, const char* func, const char* condition) {
    IntrDisable();
    PutChar('\n'); PutStr("!!!!!error!!!!!"); PutChar('\n');
    PutStr("filename:"); PutStr(fileName); PutChar('\n');
    PutStr("line:0x"); PutInt(line); PutChar('\n');
    PutStr("function:"); PutStr((uint8*)func); PutChar('\n');
    PutStr("condition:"); PutStr((uint8*)condition); PutChar('\n');
    while(1);
}