%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

LOADER_STACK_TOP equ LOADER_BASE_ADDR

SELECTOR_CODE equ (0x0001 << 3) | TI_GDT | RPL0
SELECTOR_DATA equ (0x0002 << 3) | TI_GDT | RPL0
SELECTOR_VIDEO equ (0x0003 << 3) | TI_GDT | RPL0


    jmp loaderStart

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
    



; Loader
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
    jmp dword SELECTOR_CODE:protectionModeStart     ; 刷新流水线，并加载cs，指令能得以执行是因为实模式和保护模式都会优先使用段缓冲寄存器里面的基址，此时cs中的隐藏基址依旧是正确的。

[bits 32]
protectionModeStart:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    
    




    jmp $

