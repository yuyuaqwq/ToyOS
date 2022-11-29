#include "kernel/print.h"

int _main(void) {
    PutStr("I am kernel\n");
    PutInt(0x666);
    while(1);
    return 0;
}