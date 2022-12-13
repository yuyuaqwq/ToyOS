..\tools\nasm -f coff -o ..\build\print.o .\kernel\print.asm
gcc -m32 -c -o ..\build\string.o -I .. string.c
gcc -m32 -c -o ..\build\list.o -I .. .\kernel\list.c
