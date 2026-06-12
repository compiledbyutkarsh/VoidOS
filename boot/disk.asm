disk_load:
    pusha
    push dx

    mov ah, 0x02
    mov al, dh
    mov cl, 0x02
    mov ch, 0x00
    mov dh, 0x00

    int 0x13
    jc disk_error

    pop dx
    cmp al, dh
    jne sectors_error

    popa
    ret

disk_error:
    mov si, DISK_ERROR_MSG
    call print_string
    jmp $

sectors_error:
    mov si, SECTORS_ERROR_MSG
    call print_string
    jmp $

DISK_ERROR_MSG db 'ERR: Disk read failure', 0x0D, 0x0A, 0
SECTORS_ERROR_MSG db 'ERR: Sector count mismatch', 0x0D, 0x0A, 0
