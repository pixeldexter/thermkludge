
#define F_CPU 20000000 /* частота мк, Гц */

#define USE_EXTRACTINT    /* поддержка ds18b20_extractInt() */
#define USE_EXTRACTFRACT  /* поддержка ds18b20_extractFract() */

// поддержка автоматической проверки данных контрольной суммой
#define SUPPORT_CHECKSUM
// поддержка возможности поиска устройств на шине
#define SUPPORT_ROMSEARCH
// поддержка датчиков питающихся паразитным током
#define SUPPORT_PARASITE_MODE
// поддержка некоторых дополнительных функций
#define SUPPORT_COMPLETE

/* порт к которому подключена линия данных */
#define DDR_SENSOR   _SFR_IO_ADDR(DDRD)
#define PIN_SENSOR   _SFR_IO_ADDR(PIND)
#define PORT_SENSOR  _SFR_IO_ADDR(PORTD)
/* ножка к которой подключена линия данных */
#define NPIN_SENSOR  PD2
