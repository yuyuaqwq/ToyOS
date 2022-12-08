%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

LOADER_STACK_TOP equ LOADER_BASE_ADDR

SELECTOR_CODE equ (0x0001 << 3) | TI_GDT | RPL0
SELECTOR_DATA equ (0x0002 << 3) | TI_GDT | RPL0
SELECTOR_VIDEO equ (0x0003 << 3) | TI_GDT | RPL0


; Loader data: 0x900

; GDT，全局描述符表
GDT_BASE:
    dd 0x00000000
    dd 0x00000000

CODE_DESC:
    dd 0x0000ffff
    dd DESC_CODE_HIGH4

DATA_STACK_DESC:
    dd 0x0000ffff
    dd DESC_DATA_HIGH4

VIDEO_DESC:
    dd 0x80000007
    dd DESC_VIDEO_HIGH4

GDT_SIZE equ $ - GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1

    times 60 dq 0       ; 预留60个描述符空位


; 物理内存总数，在内存中的地址是0xb00
    memBytesTotal dd 0


; lgdt时提供的48位数据
    gdtPtr dw GDT_LIMIT
            dd GDT_BASE
    
; 除开GDT外的数据区为256字节
    ardsBuf times 244 db 0
    ardsNr dw 0     ; 记录ARDS结构体数量
    



; Loader code: 0x900+0x300
loaderStart:

    xor ebx, ebx
    mov edx, 0x534d4150
    mov di, ardsBuf

; 0xE820 方法获取物理内存信息
.e820MemGetLoop:
    mov eax, 0x0000e820     ; 每次都要更新子功能号
    mov ecx, 20     ; ARDS地址范围描述符结构体大小是20字节
    int 0x15
    jc .e801MemGetLoop       ; cf为1表示发生错误，使用其他方法

    add di, cx      ; 更新缓冲区指针
    inc word [ardsNr]       ; ARDS数量
    cmp ebx, 0      ; ebx为0且cf不为1，说明ARDS全部返回
    jnz .e820MemGetLoop     ; 获取下一个

; 遍历ARDS结构获取总大小
    mov cx, [ardsNr]
    mov ebx, ardsBuf
    xor edx, edx

.findMaxMemArea:
    mov eax, [ebx]      ; base_add_low
    add eax, [ebx+8]    ; +length_low，比如总大小是0x1000，最后一块是0xc00 + 0x400，正好就是总大小
    add ebx, 20
    cmp edx, eax
    jge .nextArds
    mov edx, eax        ; 更新最大的内存容量

.nextArds:
    loop .findMaxMemArea
    jmp .memGetOk

; 0xE801 方法获取物理内存信息
.e801MemGetLoop:
    mov ax, 0xe801
    int 0x15
    jc .88MemGetLoop

    ; 1.算出低15MB的内存
    ; ax和cx中是KB为单位的内存大小，转换为byte单位
    mov cx, 0x400       ; cx和ax值一样，cx作乘数
    mul cx
    shl edx, 16
    and eax, 0x0000ffff
    or edx, eax
    add edx, 0x100000       ; ax只有15MB，故加1MB
    mov esi, edx        ; 低15MB的内存容量存入esi备份

    ; 2.将16MB以上的内存转换为byte单位
    ; bx和dx中是以64KB为单位的内存大小
    xor eax, eax
    mov ax, bx
    mov ecx, 0x10000        ; 64KB
    mul ecx

    add esi, eax
    mov edx, esi
    jmp .memGetOk


; 0x88 方法获取物理内存信息
.88MemGetLoop:
    mov ah, 0x88
    int 0x15
    jc .errorHlt
    
    ; ax单位为KB，将ax的内存大小转换为byte单位
    and eax, 0x0000ffff
    mov cx, 0x400
    mul cx
    shl edx, 16
    or edx, eax
    add edx, 0x100000       ; 只会返回1MB以上的内存，故实际内存需要加上1MB

.memGetOk:
    mov [memBytesTotal], edx        ; 保存可用内存总数
    jmp readyEntryProtectedMode
    
.errorHlt:
    hlt
    jmp .errorHlt

; 准备进入保护模式
readyEntryProtectedMode:
    ; 1.打开A20地址线
    in al, 0x92
    or al, 0x02
    out 0x92, al

    ; 2.加载GDT
    lgdt [gdtPtr]

    ; 3.cr0.PE置1
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax

; 正式进入保护模式
    jmp dword SELECTOR_CODE:protectionModeStart     ; 刷新流水线，并加载cs，指令能得以执行是因为实模式和保护模式都会优先使用段缓存寄存器里面的基址，此时cs中的隐藏基址依旧是正确的。

[bits 32]
protectionModeStart:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    
; 读取kernel.bin
    mov eax, KERNEL_START_SECTOR
    mov ebx, KERNEL_BIN_BASE_ADDR       ; 临时存放内核执行文件，用于拉伸
    mov ecx, 200        ; 读取200扇区
    call RdDiskM32




; 准备启用分页模式
    call SetupPage

    sgdt [gdtPtr]       ; 需要重新加载GDT，先保存

    mov ebx, [gdtPtr + 2]       ; 获得GDTBase
    or dword [ebx + 0x18 + 4], 0xc0000000       ; GDT[2]是视频段描述符，修改基址
    add dword [gdtPtr + 2], 0xc0000000      ; 启用分页后，访问GDT的地址会变为虚拟地址
    add esp, 0xc0000000     ; 栈指针修改为虚拟地址

    ; cr3是页目录物理地址
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax

    ; 开启分页模式
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; cs段描述符缓存的基址都是0，且ip指向的虚拟地址已经提前映射到相同的物理地址上了，所以指令是能够正常继续执行的
    lgdt [gdtPtr]

    jmp SELECTOR_CODE:entryKernel       ; 强制刷新流水线，测试没有问题，稳妥起见
entryKernel:
    call KernelInit
    cmp eax, 0
    je .error

    mov esp, 0xc009f000
    jmp KERNEL_ENTRY_POINT      ; 控制权交给Kernel

.error:
    hlt
    jmp .error

;----------加载内核----------
KernelInit:
    push ebx
    push edx
    push edi
    

    mov ebx, KERNEL_BIN_BASE_ADDR

    cmp word [ebx], 'MZ'
    jne .error

    mov edi, 0x3c

    add ebx, [ebx+edi]  ; 访问dosHeader.e_lfanew，使ebx指向ntHeader32

    cmp dword [ebx], 'PE'
    jne .error

    add ebx, 4      ; ebx指向fileHeader
    
    xor ecx, ecx
    mov cx, [ebx+0x2]      ; fileHeader.NumberOfSections
    xor eax, eax
    mov ax, [ebx+0x10]      ; fileHeader.SizeOfOptionalHeader

    
    add ebx, 0x14       ; ebx指向optionalHeader32
    mov edx, [ebx+0x1c]     ; optionalHeader32.ImageBase

    add ebx, eax     ; ebx指向sectionsTable


; 遍历节表拷贝节区
.copySectionLoop:
    mov eax, [ebx+0x10]      ; sectionHeader.SizeOfRawData
    push eax        ; size

    mov eax, [ebx+0x14]      ; sectionHeader.PointerToRawData
    add eax, KERNEL_BIN_BASE_ADDR
    push eax        ; src

    mov eax, [ebx+0xc]      ; sectionHeader.VirtualAddress
    add eax, edx
    push eax        ; dst

    call MemCpy
    add esp, 0xc

    add ebx, 0x28       ; 指向下一个sectionsTable
    loop .copySectionLoop


    mov eax, 1
    jmp .ret
.error:
    mov eax, 0
.ret:

    pop edi
    pop edx
    pop ebx
    ret




;----------内存拷贝----------
; int dst, int src, int size
MemCpy:
    cld
    push ebp
    mov ebp, esp
    push ecx        ; rep指令用到了ecx，但ecx对于外层段的循环还有用，故先入栈备份
    mov edi, [ebp + 8]      ; dst
    mov esi, [ebp + 12]     ; src
    mov ecx, [ebp + 16]     ; size
    rep movsb       ; 逐字节拷贝


    pop ecx		
    pop ebp
    ret





;----------读取扇区到内存----------
RdDiskM32:
    mov esi, eax
    mov di, cx

; 1.设置要读取的扇区数
    mov dx, 0x1f2
    mov al, cl
    out dx, al      ; 读取扇区数
    mov eax, esi

; 2.将LBA存入0x1f3 ~ 0x1f6
    ; bit7 ~ bit0
    mov dx, 0x1f3
    out dx, al

    ; bit15 ~ bit8
    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al

    ; bit23 ~ bit16
    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    ; bit31 ~ bit24
    shr eax, cl
    and al, 0x0f        ; LBA bit27 ~ bit24
    or al, 0xe0         ; bit31 ~ bit28为1110，表示LBA模式
    mov dx, 0x1f6
    out dx, al

; 3.向0x1f7端口写入读命令，0x20
    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

; 4.检测硬盘状态
.notReady:
    ; 同一端口，写时表示写入命令字，读时表示读入硬盘状态
    nop
    in al, dx
    and al, 0x88        ; bit4为1表示硬盘控制器已准备好数据，bit7为一表示硬盘忙
    cmp al, 0x08
    jnz .notReady       ; 未准备好，继续等待

; 5.从0x1f0端口读数据
    ; di是要读取的扇区数，一个扇区有512字节，每次读入一个字，需要di*512/2次，所以di*256
    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax

    mov dx, 0x1f0
.goOnRead:
    in ax, dx
    mov [ebx], ax
    add ebx, 2
    loop .goOnRead
    ret






; ----------创建页目录及页表----------
SetupPage:
    ; 存放页目录的页面清0
    mov ecx , 0x1000
    mov esi, 0
.clearPageDir:
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clearPageDir

    ; 创建页目录项(PDE)
.createPDE:
    mov eax, PAGE_DIR_TABLE_POS
    or eax, PG_US_U | PG_RW_W | PG_P        ; 加上属性，组成PDE

    mov [PAGE_DIR_TABLE_POS + 0xffc], eax       ; 写到PDT[0x3ff]中，即页目录自映射

    ; 创建页表(PTT)
    add eax, 0x1000     ; 页目录页面之后紧挨着的第一个页面的基址，0x101000，用于PDT[0x300]指向的页表
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax       ; 写到PDT[0x300]中
    mov [PAGE_DIR_TABLE_POS + 0x0], eax       ; 写到PDT[0x0]中


    ; 创建页表项(PTE)，将物理地址 0x00000 ~ 0xfffff 映射到虚拟地址 0x00000000 ~ 0x000fffff 和 0xc0000000 ~ 0xc00fffff
    mov ebx, PAGE_DIR_TABLE_POS
    add ebx, 0x1000     ; PDT[0x300]指向的页表
    mov ecx, 256        ; 256项
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P       ; base为0
.createPTE:
    mov [ebx+esi*4], edx        ; PDT[0x300]指向的页表
    add edx, 0x1000     ; base += 0x1000
    inc esi
    loop .createPTE

    ; 为内核剩余页目录项分配页表
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000     ; 页目录页面之后紧挨着的第二个页面的基址，0x102000，用于PDT[0x301]指向的页表
    or eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254
    mov esi, 0x301      ; 范围为 0x301 ~ 0x3fe 的所有PDE
.createKernelPDE:
    mov [ebx+esi*4], eax
    inc esi
    add eax, 0x1000     ; 继续使用下一个物理页面当作PTT
    loop .createKernelPDE
    ret