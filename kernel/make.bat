..\tools\nasm -f coff -o ..\build\kernel.o -l ..\build\kernel.lst kernel.asm
gcc -m32 -c -o ..\build\main.o -I .. main.c
gcc -m32 -c -o ..\build\interrupt.o -I .. interrupt.c
gcc -m32 -c -o ..\build\init.o -I .. init.c
gcc -m32 -c -o ..\build\debug.o -I .. debug.c
gcc -m32 -c -o ..\build\bitmap.o -I .. bitmap.c
gcc -m32 -c -o ..\build\memory.o -I .. memory.c
ld -Ttext 0xc0001500 -e main -o ..\build\kernel.bin -mi386pe ..\build\main.o ..\build\init.o ..\build\interrupt.o ..\build\kernel.o ..\build\debug.o ..\build\bitmap.o ..\build\memory.o ..\build\thread.o ..\build\sync.o ..\build\switch.o ..\build\timer.o ..\build\console.o ..\build\keyboard.o ..\build\ioqueue.o ..\build\string.o ..\build\print.o ..\build\list.o
