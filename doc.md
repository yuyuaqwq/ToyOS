# 物理内存布局
|起始地址|结束地址|大小|说明|
|:--:|:--:|:--:|:--|:--|
|0x9fc00|0x9ffff|1K|Extended BIOS Data Area<br>扩展BIOS数据区|
|0x7e00|0x9fbff|622080B 约608K|可用区域|
|0x7c00|0x7dff|512B|MBR被BIOS加载到此处|
|0x500|0x7bff|30464B 约30K|可用区域|
|0x400|0x4ff|256B|BIOS Data Area<br>BIOS数据区|
|0x000|0x3ff|1K|Interrupt Vector Table<br>中断向量表|


# 物理内存使用设计
## 0x900 ~ 0x10ff
将Loader加载至此处，共2048Byte

## 0x70000 ~ 0x88fff
kernel.bin文件buffer，共100K，加载内核后可覆盖

## 0x9a000 ~ 0x9dfff
内存位图，16K，4页，可管理512M物理内存。

## 0x9e000 ~ 0x9efff
kernel使用的栈空间，向低地址扩展，设置地址为0x9f000
也是main线程的PCB

## 0x1500 ~ 0x99fff
kernel映像
MBR此时已经无用，可以被覆盖，故可以连接

## 0x100000 ~ 0x100fff
页目录

## 0x101000 ~ 0x101fff
第一个内核页表

## 0x101fff ~ 0x1fffff
剩余所有内核页表

# PCB
PCB与栈处于同一页面，PCB从低向高扩展，栈由高向低扩展


# 虚拟内存布局
## 0x00000000 ~ 0xbfffffff
用户进程独立空间

## 0xc0000000 ~ 0xffffffff
内核共享空间

## 0xc0000000 ~ 0xc00fffff
映射到物理空间0x000000 ~ 0x0fffff，低端1M空间

## 0xfffff000 ~ 0xffffffff
映射到页目录