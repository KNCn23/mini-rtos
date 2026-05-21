#ifndef UART_H
#define UART_H
#include "types.h"
#define UART0_BASE 0x09000000UL
void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_print_dec(u64 v);
void uart_print_hex(u64 v);
#endif
