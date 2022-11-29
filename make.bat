#cd .\boot
#.\make.bat
#cd ..
#cd .\kernel
#.\make.bat
#cd ..

.\tools\filecp.exe .\build\disk.vhd 0 .\build\mbr.bin
.\tools\filecp.exe .\build\disk.vhd 1024 .\build\loader.bin
.\tools\filecp.exe .\build\disk.vhd 4608 .\build\kernel.bin
del ..\build\disk.vhd.lock
pause