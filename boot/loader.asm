%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

LOADER_STACK_TOP equ LOADER_BASE_ADDR

    jmp loaderStart

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

SELECTOR_CODE equ (0x0001 << 3) | TI_GDT | RPL0
SELECTOR_DATA equ (0x0002 << 3) | TI_GDT | RPL0
SELECTOR_VIDEO equ (0x0003 << 3) | TI_GDT | RPL0

; gdtr
    gdt_ptr dw GDT_LIMIT
            dd GDT_BASE
    
    loadErrMsg db '2 loader in real.'

loaderStart:
    mov sp, LOADER_BASE_ADDR
    mov bp, loadErrMsg
    mov cx, 17
    mov ax, 0x1301
    mov bx, 0x001f
    mov dx, 0x1800
    int 0x10

; 准备进入保护模式

    ; 1.打开A20地址线
    in al, 0x92
    or al, 0x02
    out 0x92, al

    ; 2.加载GDT
    lgdt [gdt_ptr]

    ; 3.cr0.PE置1
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax

; 正式进入保护模式
    jmp dword SELECTOR_CODE:protectionModeStart     ; 刷新流水线，并加载cs，指令能得以执行是因为实模式和保护模式都会优先使用段缓冲寄存器里面的基址，此时cs的基址依旧是正确的。

[bits 32]
protectionModeStart:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov byte gs:[160], 'P'

    jmp $