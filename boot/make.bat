..\tools\nasm mbr.asm -o ..\build\mbr.bin
..\tools\filecp.exe ..\build\disk.vhd 0 ..\build\mbr.bin
pause