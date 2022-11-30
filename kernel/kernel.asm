[bits 32]
%define ERROR_CODE nop
%define ZERO push 0

extern PutStr

section .data
intrStr db "interrupt occur!", 0xa, 0

global intrEntryTable
intrEntryTable:

%macro VECTOR 2
section .text

intr%1Entry:

    %2
    push intrStr
    call PutStr
    add esp, 4

    mov al, 0x20
    out 0xa0, al
    out 0x20, al

    add esp, 4
    iret

section .data
    dd intr%1Eentry

%endmacro


VECRIR 0x00, ZERO
VECRIR 0x01, ZERO
VECRIR 0x02, ZERO
VECRIR 0x03, ZERO
VECRIR 0x04, ZERO
VECRIR 0x05, ZERO
VECRIR 0x06, ZERO
VECRIR 0x07, ZERO
VECRIR 0x08, ZERO
VECRIR 0x09, ZERO
VECRIR 0x0a, ZERO
VECRIR 0x0b, ZERO
VECRIR 0x0c, ZERO
VECRIR 0x0d, ZERO
VECRIR 0x0e, ZERO
VECRIR 0x0f, ZERO
VECRIR 0x10, ZERO
VECRIR 0x11, ZERO
VECRIR 0x12, ZERO
VECRIR 0x13, ZERO
VECRIR 0x14, ZERO
VECRIR 0x15, ZERO
VECRIR 0x16, ZERO
VECRIR 0x17, ZERO
VECRIR 0x18, ZERO
VECRIR 0x19, ZERO
VECRIR 0x1a, ZERO
VECRIR 0x1b, ZERO
VECRIR 0x1c, ZERO
VECRIR 0x1d, ZERO
VECRIR 0x1e, ERROR_CODE
VECRIR 0x1f, ZERO
VECRIR 0x20, ZERO
