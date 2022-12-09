#cd .\boot
#.\make.bat
#cd ..

cd .\build
del *.o
del *.bin
del *.lst
cd ..

cd .\boot
call .\make.bat
cd ..

cd .\lib
call .\make.bat
cd ..

cd .\thread
call .\make.bat
cd ..

cd .\device
call .\make.bat
cd ..

cd .\lib
call .\make.bat
cd ..

cd .\userprog
call .\make.bat
cd ..


cd .\kernel
call .\make.bat
cd ..

.\tools\filecp.exe .\build\disk.vhd 0 .\build\mbr.bin
.\tools\filecp.exe .\build\disk.vhd 1024 .\build\loader.bin
.\tools\filecp.exe .\build\disk.vhd 4608 .\build\kernel.bin
del ..\build\disk.vhd.lock
pause