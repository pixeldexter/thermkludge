/******************************************************************************
  libds18b20 - Library for working with ds18b20 compatible term sensors
  Copyright (c) 2010, Maxim Pshevlotski <mpshevlotsky@gmail.com>

  This file is part of libds18b20.

  libds18b20 is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  libds18b20 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with libds18b20.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

.global ds18b20_readROM ; uint8_t ds18b20_readROM(uint8_t* id);
/* чтение rom сенсора, единственного в системе */
ds18b20_readROM: ; Z,X,p0l,p0h,p1l,p1h
  movw ZL,p0l
  adiw ZL,8 ; смещение с конца
  rcall reset
  brtc 1f
  txbyte 0x33 ; read rom
#ifdef SUPPORT_CHECKSUM
  ldi p1l,7
  ldi XL,0 ; seed
0:rcall rxbyte
  st -Z,p0l
  rcall crc8
  dec p1l
  brne 0b
  rcall rxbyte
  st -Z,p0l
  cp p0l,XL ; seed == crc?
  brne 1f
#else
  ldi p1l,8
0:rcall rxbyte
  st -Z,p0l
  dec p1l
  brne 0b
#endif // SUPPORT_CHECKSUM
  ret ; ok, ret crc
1:clr p0l
  ret ; err, ret 0

.global ds18b20_readScratchpad ; uint8_t ds18b20_readScratchpad(sspad* p);
/* чтение блокнота в *p */
ds18b20_readScratchpad: ; Z,X,p0l,p0h,p1l,p1h
  movw ZL,p0l ; *sspad
  rcall match ; выбрать датчик
  brtc 1f ; выйти если ошибка на линии
  txbyte 0xbe ; read scratchpad
  ldi p1l,8 ; read 8 bytes from scratchpad
#ifdef SUPPORT_CHECKSUM
  ldi XL,0 ; seed
0:rcall rxbyte
  st Z+,p0l
  rcall crc8
  dec p1l
  brne 0b
  rcall rxbyte ; crc
  cp p0l,XL ; seed == crc?
  brne 1f
  ret ; ok, ret crc
#else
0:rcall rxbyte
  st Z+,p0l
  dec p1l
  brne 0b
  rjmp rxbyte ; ok, ret crc
#endif // SUPPORT_CHECKSUM
1:clr p0l
  ret ; err, ret 0

.global ds18b20_writeScratchpad ; void ds18b20_writeScratchpad(const sspad* p);
/* запись блокнота из *p */
ds18b20_writeScratchpad: ; Z,X,p0l,p0h,p1l
  movw ZL,p0l ; *sspad
  rcall match ; выбрать датчик
  brtc 1f ; выйти если ошибка на линии
  txbyte 0x4e ; write scratchpad
  adiw ZL,2 ; sspad+2
  ldi p1l,3
0:ld p0l,Z+
  rcall _txbyte
  dec p1l
  brne 0b
1:ret

.global ds18b20_copyScratchpad ; void ds18b20_copyScratchpad();
/* копирование блокнота в eeprom */
ds18b20_copyScratchpad: ; X,p0l,p0h,p1l
  rcall match ; выбрать датчик
  brtc 1f ; выйти если ошибка на линии
  txbyte 0x48 ; copy scratchpad
#ifdef SUPPORT_PARASITE_MODE
  ldi XH,hi8(cur_mode)
  ldi XL,lo8(cur_mode)
  ld p0l,X
  sbrs p0l,7 ; power mode bit == 1? => parasite power
  rjmp 1f ; parasite power:
  sbi DDR_SENSOR,NPIN_SENSOR
  sbi PORT_SENSOR,NPIN_SENSOR ; enable strong pullup
  delay_ms 10
#endif // SUPPORT_PARASITE_MODE
1:ret

.global ds18b20_recall ; void ds18b20_recall();
/* восстановление значений th, tl, и config из eeprom */
ds18b20_recall: ; X,p0l,p0h,p1l
  rcall match ; выбрать датчик
  brtc 1f ; выйти если ошибка на линии
  txbyte 0xb8 ; recall e2
0:rcall rxbyte ; wait for..
  cpi p0l,0
  breq 0b
1:ret

#ifdef SUPPORT_PARASITE_MODE
.global ds18b20_readPowerSupply ; uint8_t ds18b20_readPowerSupply();
/* определение источника питания */
ds18b20_readPowerSupply: ; X,p0l,p0h,p1l
  rcall match ; выбрать датчик
  brtc 1f ; выйти если ошибка на линии
  txbyte 0xb4 ; read power supply
  rjmp rxbyte ; rx == 0? => parasite
1:ret
#endif // SUPPORT_PARASITE_MODE

#ifdef SUPPORT_ROMSEARCH
.global ds18b20_searchAlarm ; int8_t ds18b20_searchAlarm(uint8_t id[8])
/* поиск rom следующего устройства на шине */
ds18b20_searchAlarm: ; r1 - регистр флагов
  ldi p1l,0xec ; alarm search
  rjmp _search ; -1=>сбой при проверке crc,0=>поиск завершён,crc=>успешно
#endif
