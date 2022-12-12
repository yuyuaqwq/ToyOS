[bits 32]
section .text

; 切换线程(即切换栈)
global _SwitchTo
_SwitchTo:
    ; 将必要寄存器备份到当前线程的栈中
    push esi
    push edi
    push ebx
    push ebp

    
    mov eax, [esp + 20]     ; 第一个参数，cur
    mov [eax], esp      ; cur.selfKStack = esp

    mov eax, [esp + 24]     ; 第二个参数，next
    mov esp, [eax]      ; esp = next.selfKStack，至此完成栈切换，即线程切换

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret     ; 由于栈切换了，ret是返回到新线程栈存储的地址
