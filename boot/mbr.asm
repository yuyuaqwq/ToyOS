%include "boot.inc"

SECTION MBR vstart=0x7c00
; 0000:7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00

    mov ax, 0xb800
    mov gs, ax      ; 显存

; 上卷清屏
    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0
    mov dx, 0x184f
    int 0x10

; 从磁盘中读取Loader
    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cx, 4
    call RdDiskM16

    jmp LOADER_BASE_ADDR     ; 跳转到Loader起始地址



;------------------------------------
; 从磁盘读取n个扇区
; eax = LBA扇区号
; bx = 缓冲区地址
; cx = 读取扇区数
;------------------------------------
RdDiskM16:
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
.goOnRead
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .goOnRead
    ret


times 510-($-$$) db 0
db 0x55,0xaa