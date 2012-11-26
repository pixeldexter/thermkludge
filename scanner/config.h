#ifndef _SCANNER_CONFIG_H_INC_
#define _SCANNER_CONFIG_H_INC_



/*
 * CPU clock speed in HZ as specified by fuses
 */
#define F_CPU 2000000



/*
 * software (bit-bang) UART settings
 *
 * CONFIG_UART_GPIO  defines the gpio port where to place UART
 * CONFIG_UART_RXBIT defines RX pin number (undef to disable function)
 * CONFIG_UART_TXBIT defines TX pin number (undef to disable function)
 * CONFIG_UART_FORMATTER enable built-in formatter
 *
 */
#define CONFIG_UART_GPIO  B
#undef  CONFIG_UART_RXBIT
#define CONFIG_UART_TXBIT 3
#define CONFIG_UART_FORMATTER true
#define CONFIG_UART_BAUDRATE  9600




/*
 * 1-wire settings:
 *
 * CONFIG_ONEWIRE_GPIO defines the gpio port identifier where
 *   sensor(s) are connected to.
 * CONFIG_ONEWIRE_GPBIT is the bit designator.
 */
#define CONFIG_ONEWIRE_GPIO  C
#define CONFIG_ONEWIRE_GPBIT 5
#define CONFIG_ONEWIRE_CHECKSUM   true
#define CONFIG_ONEWIRE_SEARCH_ROM true
#undef  CONFIG_ONEWIRE_PARASITE_POWER
#undef  CONFIG_ONEWIRE_COMPLETE



#endif /* _SCANNER_CONFIG_H_INC_ */
