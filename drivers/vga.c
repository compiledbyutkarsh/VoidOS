#include "vga.h"
#include "../lib/types.h"

static uint8_t  cursor_x = 0;
static uint8_t  cursor_y = 0;
static uint8_t  current_color = 0;
static uint16_t *vga_buf = VGA_MEMORY;

static inline uint8_t vga_make_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_make_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

static inline void vga_update_cursor() {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;

    __asm__ volatile(
        "outb %0, %1"
        :
        : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4)
    );
    __asm__ volatile(
        "outb %0, %1"
        :
        : "a"((uint8_t)(pos & 0xFF)), "Nd"((uint16_t)0x3D5)
    );
    __asm__ volatile(
        "outb %0, %1"
        :
        : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4)
    );
    __asm__ volatile(
        "outb %0, %1"
        :
        : "a"((uint8_t)((pos >> 8) & 0xFF)), "Nd"((uint16_t)0x3D5)
    );
}

static void vga_scroll() {
    uint16_t blank = vga_make_entry(' ', current_color);

    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buf[y * VGA_WIDTH + x] = vga_buf[(y + 1) * VGA_WIDTH + x];
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buf[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }

    cursor_y = VGA_HEIGHT - 1;
}

void vga_init() {
    current_color = vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_clear() {
    uint16_t blank = vga_make_entry(' ', current_color);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buf[i] = blank;
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    current_color = vga_make_color(fg, bg);
}

void vga_set_cursor(uint8_t x, uint8_t y) {
    cursor_x = x;
    cursor_y = y;
    vga_update_cursor();
}

void vga_putc(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
    } else {
        vga_buf[cursor_y * VGA_WIDTH + cursor_x] = vga_make_entry(c, current_color);
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
    }

    vga_update_cursor();
}

void vga_puts(const char *str) {
    while (*str) {
        vga_putc(*str++);
    }
}

static void vga_print_int(int32_t n) {
    if (n < 0) {
        vga_putc('-');
        n = -n;
    }
    if (n >= 10) vga_print_int(n / 10);
    vga_putc('0' + (n % 10));
}

static void vga_print_hex(uint32_t n) {
    vga_puts("0x");
    bool leading = true;
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (n >> i) & 0xF;
        if (nibble || !leading || i == 0) {
            leading = false;
            vga_putc(nibble < 10 ? '0' + nibble : 'a' + nibble - 10);
        }
    }
}

void vga_printf(const char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd': vga_print_int(__builtin_va_arg(args, int32_t)); break;
                case 'x': vga_print_hex(__builtin_va_arg(args, uint32_t)); break;
                case 's': vga_puts(__builtin_va_arg(args, const char*)); break;
                case 'c': vga_putc((char)__builtin_va_arg(args, int)); break;
                case '%': vga_putc('%'); break;
                default:  vga_putc('%'); vga_putc(*fmt); break;
            }
        } else {
            vga_putc(*fmt);
        }
        fmt++;
    }

    __builtin_va_end(args);
}
