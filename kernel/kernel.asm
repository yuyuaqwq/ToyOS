[bits 32]
%define ERROR_CODE nop
%define ZERO push 0

extern _PutInt

extern _gIdtTable

section .data
global _gIntrEntryTable
_gIntrEntryTable:

%macro VECTOR 2
section .text

intr%1Entry:

    %2
    push ds
    push es
    push fs
    push gs
    pushad

    ; 告知8259A中断处理完成
    mov al, 0x20
    out 0xa0, al
    out 0x20, al

    push %1
    call [_gIdtTable + %1*4]        ; 调用C版本的中断处理函数
    jmp IntrExit

section .data
    dd intr%1Entry

%endmacro

section .text
global IntrExit
IntrExit:
    add esp, 4      ; 跳过中断向量号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4      ; 跳过errorCode
    iretd


VECTOR 0x00, ZERO
VECTOR 0x01, ZERO
VECTOR 0x02, ZERO
VECTOR 0x03, ZERO
VECTOR 0x04, ZERO
VECTOR 0x05, ZERO
VECTOR 0x06, ZERO
VECTOR 0x07, ZERO
VECTOR 0x08, ERROR_CODE
VECTOR 0x09, ZERO
VECTOR 0x0a, ERROR_CODE
VECTOR 0x0b, ERROR_CODE
VECTOR 0x0c, ZERO
VECTOR 0x0d, ERROR_CODE
VECTOR 0x0e, ERROR_CODE
VECTOR 0x0f, ZERO
VECTOR 0x10, ZERO
VECTOR 0x11, ERROR_CODE
VECTOR 0x12, ZERO
VECTOR 0x13, ZERO
VECTOR 0x14, ZERO
VECTOR 0x15, ZERO
VECTOR 0x16, ZERO
VECTOR 0x17, ZERO
VECTOR 0x18, ERROR_CODE
VECTOR 0x19, ZERO
VECTOR 0x1a, ERROR_CODE
VECTOR 0x1b, ERROR_CODE
VECTOR 0x1c, ZERO
VECTOR 0x1d, ERROR_CODE
VECTOR 0x1e, ERROR_CODE
VECTOR 0x1f, ZERO
VECTOR 0x20, ZERO       ; 时钟中断对应入口
VECTOR 0x21, ZERO       ; 键盘中断对应入口
VECTOR 0x22, ZERO       ; 级联
VECTOR 0x23, ZERO       ; 串口2对应入口
VECTOR 0x24, ZERO       ; 串口1对应入口
VECTOR 0x25, ZERO       ; 并口2对应入口
VECTOR 0x26, ZERO       ; 软盘对应入口
VECTOR 0x27, ZERO       ; 并口1对应入口
VECTOR 0x28, ZERO       ; 实时时钟对应入口
VECTOR 0x29, ZERO       ; 重定向
VECTOR 0x2a, ZERO       ; 保留
VECTOR 0x2b, ZERO       ; 保留
VECTOR 0x2c, ZERO       ; ps/2 鼠标
VECTOR 0x2d, ZERO       ; FPU浮点单元异常
VECTOR 0x2e, ZERO       ; 硬盘
VECTOR 0x2f, ZERO       ; 保留


extern _gSyscallTable
section .text
; 80号中断
_SyscallHandle:
    ; 构造中断环境
    push 0

    push ds
    push es
    push fs
    push gs
    pushad

    push 0x80

    ; 压入r3参数
    push edx
    push ecx
    push ebx
    call [_gSyscallTable + eax*4]       ; 系统调用服务表
    add esp, 12

    mov [esp + 8*4], eax        ; 返回值存放到内核栈中eax的位置，交给popad恢复eax
    jmp IntrExit