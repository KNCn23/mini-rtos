#include "uart.h"
#define UART_DR   0x000
#define UART_FR   0x018
#define UART_IBRD 0x024
#define UART_LCRH 0x02C
#define UART_CR   0x030
#define FR_TXFF (1 << 5)

static volatile u32 *reg(unsigned long off) {
    return (volatile u32 *)(UART0_BASE + off);
}

void uart_init(void) {
    *reg(UART_CR)   = 0;
    *reg(UART_IBRD) = 13;
    *reg(UART_LCRH) = (3 << 5) | (1 << 4);
    *reg(UART_CR)   = (1 << 0) | (1 << 8) | (1 << 9);
}

void uart_putc(char c) {
    while (*reg(UART_FR) & FR_TXFF) {}
    *reg(UART_DR) = c;
    if (c == '\n') uart_putc('\r');
}

void uart_puts(const char *s) { while (*s) uart_putc(*s++); }

void uart_print_dec(u64 v) {
    char b[24]; int i = 0;
    if (v == 0) { uart_putc('0'); return; }
    while (v) { b[i++] = '0' + (v % 10); v /= 10; }
    while (i--) uart_putc(b[i]);
}
void uart_print_hex(u64 v) {
    static const char h[]="0123456789abcdef";
    uart_puts("0x");
    int started = 0;
    for (int i = 60; i >= 0; i -= 4) {
        char c = h[(v >> i) & 0xF];
        if (c != '0') started = 1;
        if (started || i == 0) uart_putc(c);
    }
}
