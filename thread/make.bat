gcc -m32 -c -o ..\build\thread.o -I .. thread.c
..\tools\nasm -f coff -o ..\build\switch.o -l ..\build\switch.lst ..\thread\switch.asm