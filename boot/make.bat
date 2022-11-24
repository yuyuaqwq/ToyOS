..\tools\nasm mbr.asm -o ..\build\mbr.bin
..\tools\nasm loader.asm -o ..\build\loader.bin
..\tools\filecp.exe ..\build\disk.vhd 0 ..\build\mbr.bin
..\tools\filecp.exe ..\build\disk.vhd 1024 ..\build\loader.bin
del ..\build\disk.vhd.lock
pause