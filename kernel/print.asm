TI_GDT equ (0 << 2)
RPL0 equ 0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text

;----------将以0结尾的字符串写入到光标所在处----------
; int8* str
global _PutStr
_PutStr:
    push ebx
    push ecx

    xor ecx, ecx
    mov ebx, [esp+12]       ; str

.goon:
    mov cl, [ebx]
    cmp cl, 0
    jz .strOver
    push ecx
    call _PutChar
    add esp, 4
    inc ebx
    jmp .goon

.strOver:
    pop ecx
    pop ebx
    ret


;----------将整数转为字符串写入到光标所在处----------
section .data
putIntBuff dq 0

section .text
global _PutInt
_PutInt:
    pushad
    mov ebp, esp
    mov eax, [ebp+4*9]
    mov edx, eax
    mov edi, 7
    mov ecx, 8
    mov ebx, putIntBuff

.16based_4bits:
    and edx, 0x0000000f
    cmp edx, 9
    jg .isA2F
    add edx, '0'
    jmp .store

.isA2F:
    sub edx, 10
    add edx, 'A'

.store:
    mov [ebx+edi], dl
    dec edi
    shr eax, 4
    mov edx, eax
    loop .16based_4bits

.readyToPrint:
    inc edi

.skipPrefix0:
    cmp edi, 8

    je .full0

.goOnSkip:
    mov cl, [putIntBuff+edi]
    inc edi
    cmp cl, '0'
    je .skipPrefix0
    dec edi
    jmp .putEachNum

.full0:
    mov cl, '0'

.putEachNum:
    push ecx
    call _PutChar
    add esp, 4
    inc edi
    mov cl, [putIntBuff+edi]
    cmp edi, 8
    jl .putEachNum
    popad
    ret




;----------将字符写入到光标所在处----------
; int8 charAscii
global _PutChar
_PutChar:
    pushad
    mov ax, SELECTOR_VIDEO
    mov gs, ax

; 获取光标位置
    ; 高8位
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    in al, dx
    mov ah, al

    ; 低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in al, dx

    mov bx, ax      ; bx保存光标位置
    mov ecx, [esp+36]       ; charAscii，待打印字符

; 控制字符的特殊处理
    cmp cl, 0xd
    je .isCarriageReturn
    cmp cl, 0xa
    je .isLineFeed
    cmp cl, 0x8
    je .isBackSpace
    jmp .putOther

.isBackSpace:
; 退格键
    dec bx
    shl bx, 1

    mov byte gs:[bx], 0x20
    inc bx

    mov byte gs:[bx], 0x07
    shr bx, 1
    jmp .setCursor

.putOther:
; 可见字符
    shl bx, 1
    mov gs:[bx], cl
    inc bx
    mov byte gs:[bx], 0x07
    shr bx, 1
    inc bx
    cmp bx, 2000
    jl .setCursor

.isLineFeed:
.isCarriageReturn:
; 回车/换行
    xor dx, dx
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx

.isCarriageReturnEnd:
    add bx, 80
    cmp bx, 2000

.isLineFeedEnd:
    jl .setCursor

.rollScreen:
; 滚动屏幕
    cld 
    mov ecx, 960
    mov esi, 0xc00b80a0
    mov edi, 0xc00b8000
    rep movsd

    mov ebx, 3840
    mov ecx, 80

.cls:
    mov word gs:[ebx], 0x720
    add ebx, 2
    loop .cls
    mov bx, 1920

.setCursor:
; 设置光标
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al

.putCharDone:
    popad
    ret