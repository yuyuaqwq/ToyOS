..\tools\nasm -f coff -o ..\build\print.o -l ..\build\print.lst .\kernel\print.asm
gcc -m32 -c -o ..\build\string.o -I .. string.c