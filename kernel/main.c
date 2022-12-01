#include "kernel/init.h"
#include "kernel/debug.h"
#include "lib/kernel/print.h"

int _main(void) {
    PutStr("I am kernel\n");
    InitAll();
    ASSERT(1==2);
    while(1);
    return 0;
}