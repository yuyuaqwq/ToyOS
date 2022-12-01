..\tools\nasm -f coff -o ..\build\kernel.o -l ..\build\kernel.lst kernel.asm
gcc -m32 -c -o ..\build\main.o -I .. main.c
gcc -m32 -c -o ..\build\interrupt.o -I .. interrupt.c
gcc -m32 -c -o ..\build\init.o -I .. init.c
gcc -m32 -c -o ..\build\debug.o -I .. debug.c
ld -Ttext 0xc0001500 -e main -o ..\build\kernel.bin -mi386pe ..\build\main.o ..\build\init.o ..\build\interrupt.o ..\build\kernel.o ..\build\debug.o ..\build\timer.o ..\build\print.o
