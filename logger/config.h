#ifndef _LOGGER_CONFIG_H_INC_
#define _LOGGER_CONFIG_H_INC_



/*
 * CPU clock speed in HZ as specified by fuses
 */
#define F_CPU 2000000



/*
 * logger functionality
 *
 * CONFIG_SAMPLE_INTERVAL - interval between temperature samples
 * CONFIG_STARTUP_DELAY   - power-up waiting time (used for full erase cmd)
 * CONFIG_ACTIVATE_DELAY  - delay to hold button to activate/sleep
 * CONFIG_MAX_SENSORS     - number of sensors to support
 *
 */
#define CONFIG_SAMPLE_INTERVAL 15
#define CONFIG_STARTUP_DELAY   5
#define CONFIG_ACTIVATE_DELAY  1
#define CONFIG_MAX_SENSORS     8



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
 * RTC configuration
 *
 * CONFIG_RTC_HZ        - clock tick frequency
 * CONFIG_RTC_F         - RTC oscillator value (usually 32khz)
 * CONFIG_RTC_PRECISION - number of bits to store fractional parts of second
 * CONFIG_RTC_LONG_JIFFES - use uint32_t for storing jiffes
 *
 */
#define CONFIG_RTC_PRECISION 4
#define CONFIG_RTC_HZ        4
#define CONFIG_RTC_F         32768U
#undef  CONFIG_RTC_LONG_JIFFIES



/*
 * 1-wire settings:
 *
 * CONFIG_ONEWIRE_GPIO defines the gpio port identifier where
 *   sensor(s) are connected to.
 * CONFIG_ONEWIRE_GPBIT is the bit designator.
 */
#define CONFIG_ONEWIRE_GPIO  C
#define CONFIG_ONEWIRE_GPBIT 5
#undef  CONFIG_ONEWIRE_CHECKSUM
#undef  CONFIG_ONEWIRE_SEARCH_ROM
#undef  CONFIG_ONEWIRE_PARASITE_POWER
#undef  CONFIG_ONEWIRE_COMPLETE



/*
 * other pins
 *
 * CONFIG_LED_...    - GPIO for led
 * CONFIG_BUTTON_... - GPIO for hardware button
 *
 */
#define CONFIG_LED_GPIO  D
#define CONFIG_LED_GPBIT 5
#define CONFIG_BUTTON_GPIO  D
#define CONFIG_BUTTON_GPBIT 2
#define CONFIG_BUTTON_INTR  INT0



#endif /* _LOGGER_CONFIG_H_INC_ */
