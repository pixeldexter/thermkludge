#ifndef UART_H_INC
#define UART_H_INC

#include <stdint.h>

void uart_init(uint8_t divisor);
void uart_putchar(char ch);
void uart_putchar2(uint16_t char_pair);
void uart_puthex (uint8_t byte);
void uart_putword(uint16_t word);
int  uart_getchar(void);

#endif // UART_H_INC
