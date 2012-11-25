INCLUDE /usr/lib/ldscripts/avr4.x
PROVIDE(rtc_jiffies = DEFINED(rtc_jiffies_32) ? rtc_jiffies_32 : .);
