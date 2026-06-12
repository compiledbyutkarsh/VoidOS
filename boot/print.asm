print_string:
    pusha
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

print_hex:
    pusha
    mov cx, 4
.loop:
    mov ax, dx
    and ax, 0xF000
    shr ax, 12
    cmp ax, 9
    jle .digit
    add ax, 'A' - 10
    jmp .print
.digit:
    add ax, '0'
.print:
    mov ah, 0x0E
    int 0x10
    shl dx, 4
    loop .loop
    popa
    ret

print_nl:
    pusha
    mov ah, 0x0E
    mov al, 0x0D
    int 0x10
    mov al, 0x0A
    int 0x10
    popa
    ret
