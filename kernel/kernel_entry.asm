section .multiboot
align 4
    dd 0x1BADB002
    dd 0x3
    dd -(0x1BADB002 + 0x3)

[BITS 32]

[EXTERN kernel_main]

global _start

_start:
    mov esp, kernel_stack_top

    push ebx
    push eax

    call kernel_main

    cli
    hlt

section .bss
align 16
kernel_stack_bottom:
    resb 16384
kernel_stack_top:
