..\tools\nasm -f coff -o ..\build\print.o -l ..\build\print.lst print.asm
gcc -m32 -c -o ..\build\main.o -l ..\build\main.lst -I .. main.c
ld -Ttext 0xc0001500 -e main -o ..\build\kernel.bin -mi386pe ..\build\main.o ..\build\print.o
pause