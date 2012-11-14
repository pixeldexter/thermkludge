/******************************************************************************
  libds18b20 - Library for working with ds18b20 compatible term sensors
  Copyright (c) 2010, Maxim Pshevlotski <mpshevlotsky@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <avr/io.h>
#include "conf.h"

#include "delay.S"

#define p2h r21
#define p2l r20
#define p1h r23
#define p1l r22
#define p0h r25
#define p0l r24

.macro txbyte x
  ldi p0l,(\x)
  rcall _txbyte
.endm

#ifdef SUPPORT_COMPLETE
#include "ds18b20_ext.S"
#endif

.global ds18b20_selectSensor ; uint8_t ds18b20_selectSensor(const uint8_t id[8]);
/* выбрать текущий датчик */
ds18b20_selectSensor: ; X,Z,p0l,p0h
  rcall reset
  brtc 3f
  ldi XH,hi8(cur_sensor_id)
  ldi XL,lo8(cur_sensor_id)
  cpi p0l,0
  brne 1f
  cpi p0h,0
  brne 1f
0:clr p0l     ; если *id == 0:
  st X+,p0l ; первый байт *cur_sensor_id = 0x00
  rjmp setMode
1:movw ZL,p0l ; если *id != 0:
  adiw ZL,8 ; чтение с конца
  ser p0l
  st X+,p0l ; первый байт *cur_sensor_id = 0xFF
  ldi p0h,8
1:ld p0l,-Z ; прочитать следующий байт *id
  st X+,p0l ; копировать *id в *cur_sensor_id
  dec p0h
  brne 1b
  rjmp setMode
3:clr p0l ; результат = 0
  ret

setMode: ; Z,X,p0l,p0h,p1l
#ifdef SUPPORT_PARASITE_MODE
  rcall ds18b20_readPowerSupply ; определить источник питания
  ldi ZH,hi8(cur_mode)
  ldi ZL,lo8(cur_mode)
  tst p0l ; 0 - паразитное, 1 - внешнее
  brne 2f ; если паразитное:
  rcall match ; выбрать датчик
  txbyte 0xbe ; read scratchpad
  ldi p1l,5 ; scratchpad+5
0:rcall rxbyte
  dec p1l
  brne 0b
  cpi p0l,0x1f ; 9bit? => p1l=0
  breq 1f
  subi p1l,-2
  cpi p0l,0x3f ; 10bit? => p1l=2
  breq 1f
  subi p1l,-2
  cpi p0l,0x5f ; 11bit? => p1l=4
  breq 1f
  subi p1l,-2
  cpi p0l,0x7f ; 12bit? => p1l=6
  breq 1f
1:sbr p1l,0b10000000 ; set bit 7 => parasite power
  st Z,p1l ; cur_mode
  ret
2:ldi p1l,0b00000000 ; unset bit 7 => ext. power
  st Z,p1l ; cur_mode
  ret
#else
  ldi p0l,1
  ret
#endif // SUPPORT_PARASITE_MODE

.global ds18b20_convert ; void ds18b20_convert();
/* начать измерение температуры */
ds18b20_convert: ; Z,X,p0l,p0h,p1l
  rcall match ; выбрать датчик
  brtc 1f ; выйти если ошибка на линии
#ifdef SUPPORT_PARASITE_MODE
  ldi XH,hi8(cur_mode)
  ldi XL,lo8(cur_mode)
  ld p0l,X
  sbrs p0l,7 ; power mode bit == 1? => parasite power
  rjmp 2f ; parasite power:
  cbr p0l,0b10000000
  clr p0h
  ldi ZH,hi8(3f)
  ldi ZL,lo8(3f)
  add ZL,p0l
  adc ZH,p0h
  txbyte 0x44 ; parasite power, send convertT
  lpm tl,Z+
  lpm th,Z
  sbi DDR_SENSOR,NPIN_SENSOR
  sbi PORT_SENSOR,NPIN_SENSOR ; enable strong pullup
  rjmp _delay
3:.word (94*F_CPU)/(770*1000), (188*F_CPU)/(770*1000), \
    (375*F_CPU)/(770*1000), (750*F_CPU)/(770*1000)
#endif // SUPPORT_PARASITE_MODE
2:txbyte 0x44 ; ext. power, send convertT
0:rcall rxbyte ; wait for..
  cpi p0l,0
  breq 0b
1:ret

.global ds18b20_readTemp ; uint16_t ds18b20_readTemp();
/* чтение измеренного значения температуры */
ds18b20_readTemp: ; X,p0l,p0h,p1l
  rcall match ; выбрать датчик
  brtc 1f ; выйти если ошибка на линии
2:txbyte 0xbe ; read scratchpad
  rcall rxbyte ; lsb
  mov p1l,p0l
  rcall rxbyte ; msb
  mov p0h,p0l
  mov p0l,p1l
1:ret

#ifdef SUPPORT_ROMSEARCH
.global ow_searchROM ; int8_t ow_searchROM(uint8_t id[8])
/* поиск rom следующего устройства на шине */
ow_searchROM: ; r1 - регистр флагов
  ldi p1l,0xf0 ; search rom
  ; результат: -1=>сбой при проверке crc,
  ;   0=>поиск завершён,crc=>успешно
_search:
   movw ZL,p0l
   adiw ZL,8 ; чтение с конца
   ldi XH,hi8(last_discrepancy)
   ldi XL,lo8(last_discrepancy)
   ld p2h,X+ ; восстановить last discrepancy
   ld r1,X ; загрузить регистр флагов
   tst r1 ; is done flag set?
   brne 6f ; exit if is it
   rcall reset ; send reset signal on bus
   brtc 11f ; jump to 11f if not presence
   mov p0l,p1l ; код команды
   rcall _txbyte ; send search rom command
   ldi XH,1 ; set rom bit index = 1
#ifdef SUPPORT_CHECKSUM
   ldi XL,0 ; crc seed
#endif
   ldi p2l,0 ; clear discrepancy marker
4: ldi p0l,0 ; принятый байт
   ldi p1l,1 ; маска битов
3: rcall rxbit ; read bit a
   brtc 0f ; jump to 0f if a=0
1: rcall rxbit ; read bit b
   brtc 10f ; jump to 10f if a=1,b=0
11:clr p2h ; a=1,b=1, set last discrepancy=0
   rjmp 6f ; return false
10:set ; a=1,b=0, set rom bit=1
   rjmp 2f
0: rcall rxbit ; read bit b
   brts 01f ; jump to 01f if a=0,b=1
00:cp XH,p2h ; rom bit index = last discrepancy?
   brne 0f
   set ; set rom bit=1
   rjmp 2f
0: brlt 0f ; jump if rom bit index < last discrepancy
   clt ; set rom bit=0
   mov p2l,XH ; set discrepancy marker = rom bit index
   rjmp 2f
0: bst r1,0 ; T=r1(0), restore rom bit
   sbrs r1,0
   mov p2l,XH ; set discrepancy marker = rom bit index if rom bit=0
   rjmp 2f
01:clt
2: bld r1,0 ; r1(0)=T, save rom bit
   sbrc r1,0
   or p0l,p1l ; fill data byte
   rcall txbit
   inc XH ; increment rom bit index...
   lsl p1l ; сдвиг маски битов
   brcc 3b ; повтор если не переполнилась
   st -Z,p0l ; иначе байт данных заполнен, записываем
#ifdef SUPPORT_CHECKSUM
   cpi XH,64+1
   brsh 0f ; jump to 0f if last byte
   rcall crc8
#endif
0: cpi XH,64+1
   brlo 4b
   mov p2h,p2l ; set last discrepancy to discrepancy marker
   clr r1 ; set done flag=0
   tst p2h ; is last discrepancy = 0?
   brne 5f
   inc r1 ; set done flag=1
5: ; if ok search:
#ifdef SUPPORT_CHECKSUM
   cp XL,p0l ; crc seed = crc byte?
   breq 0f
   ldi p0l,-1
   rjmp 5f ; bad crc, result=-1
#endif
0: ldi XH,hi8(last_discrepancy)
   ldi XL,lo8(last_discrepancy)
   st X+,p2h ; запомнить last discrepancy
   st X,r1 ; запомнить регистр флагов
5: clr r1 ; to be always zero in C code
   ret
6: ldi p0l,0 ; if done search, result=0
   clr r1 ; set done flag = 0
   rjmp 0b
#endif // SUPPORT_ROMSEARCH

#ifdef SUPPORT_CHECKSUM
/* проверка контрольной суммы */
crc8: ; XL - seed, p0l - input; p0h,p1h
  ldi p1h,8
0:mov p0h,p0l
  eor p0h,XL
  lsr p0l
  sbrs p0h,0
  rjmp 1f
  ldi p0h,0b00011000
  eor XL,p0h
  lsr XL
  sbr XL,0b10000000
  rjmp 2f
1:lsr XL
2:dec p1h
  brne 0b
  ret
#endif // SUPPORT_CHECKSUM

/* передача байта p0l по шине */
_txbyte: ; p0l,p0h
  ldi p0h,8 ; счётчик бит для передачи
3:sbi DDR_SENSOR,NPIN_SENSOR
  cbi PORT_SENSOR,NPIN_SENSOR
  lsr p0l ; next bit
  brcc 4f
1:delay_us 10 ; bit 1
  cbi DDR_SENSOR,NPIN_SENSOR
  delay_us 60
  rjmp 2f
4:delay_us 60 ; bit 0
  cbi DDR_SENSOR,NPIN_SENSOR
  delay_us 10
2:dec p0h ; next
  brne 3b
  ret

/* приём байта в p0l по шине */
rxbyte: ; p0l,p0h
  ldi p0l,0 ; принятый байт
  ldi p0h,1 ; маска битов
0:sbi DDR_SENSOR,NPIN_SENSOR
  cbi PORT_SENSOR,NPIN_SENSOR
  delay_us 10
  cbi DDR_SENSOR,NPIN_SENSOR
  delay_us 10
  sbic PIN_SENSOR,NPIN_SENSOR
  or p0l,p0h
  delay_us 60
  lsl p0h ; сдвиг маски битов
  brcc 0b ; повтор если не переполнилась
  ret

/* сброс шины */
reset:
  clt
  sbi DDR_SENSOR,NPIN_SENSOR
  cbi PORT_SENSOR,NPIN_SENSOR
  delay_us 480
  cbi DDR_SENSOR,NPIN_SENSOR
  delay_us 220
  sbis PIN_SENSOR,NPIN_SENSOR
  ret ; err
  delay_us 260
  set
  ret ; ok

/* проверка адресуемости текущего сенсора */
match: ; X,p0l,p0h,p1l
  rcall reset
  brtc 2f
  ldi XH,hi8(cur_sensor_id) ; указатель на rom-данные
  ldi XL,lo8(cur_sensor_id)
  ld p0l,X+
  cpi p0l,0
  brne 0f
  ldi p0l,0xcc ; skip rom
  rjmp _txbyte
0:txbyte 0x55 ; match rom
  ldi p1l,8 ; счётчик байт для передачи
1:ld p0l,X+
  rcall _txbyte
  dec p1l
  brne 1b
2:ret

#ifdef SUPPORT_ROMSEARCH
/* приём бита по шине */
rxbit:
  clt ; bit=0
  sbi DDR_SENSOR,NPIN_SENSOR
  cbi PORT_SENSOR,NPIN_SENSOR
  delay_us 10
  cbi DDR_SENSOR,NPIN_SENSOR
  delay_us 10
  sbic PIN_SENSOR,NPIN_SENSOR
  set ; bit=1
  delay_us 60
  ret ; exit

/* передача бита по шине */
txbit:
  sbi DDR_SENSOR,NPIN_SENSOR
  cbi PORT_SENSOR,NPIN_SENSOR
  brtc 0f
1:delay_us 10 ; if bit=1
  cbi DDR_SENSOR,NPIN_SENSOR
  delay_us 60
  ret
0:delay_us 60 ; if bit=0
  cbi DDR_SENSOR,NPIN_SENSOR
  delay_us 10
  ret
#endif // SUPPORT_ROMSEARCH

#ifdef USE_EXTRACTINT
.global ds18b20_extractInt ; uint16_t ds18b20_extractInt(int16_t t);
/* извлечение значения целой части из температуры */
ds18b20_extractInt: ; Z,X,p0l,p0h,p1l,p1h,p2l,p2h
  rcall abs16
  ldi p1l,4
1:lsr p0h
  ror p0l
  dec p1l
  brne 1b
  ldi p1l,4
  rjmp bin2bcdn ; bin2bcdn(p0h:p0l,p1l)
#endif // USE_EXTRACTINT

#ifdef USE_EXTRACTFRACT
.global ds18b20_extractFract ; uint16_t ds18b20_extractFract(int16_t t, uint8_t r);
/* извлечение значения дробной части из температуры */
ds18b20_extractFract: ; Z,X,p0l,p0h,p1l,p1h,p2l,p2h
  rcall abs16
  ldi ZH,hi8(625)
  ldi ZL,lo8(625)
  cbr p0l,0b11110000
  clr XH
  clr XL ; счётчик дробной части
  ldi p1h,4
1:lsr p0l
  brcc 0f
  add XL,ZL
  adc XH,ZH
0:lsl ZL
  rol ZH ; 625->1250->2500->5000
  dec p1h
  brne 1b
  ldi ZH,hi8(2f)
  ldi ZL,lo8(2f)
  mov p2l,p1l
  dec p2l
  lsl p2l ; (r-1)*2
  add ZL,p2l
  adc ZH,p1h
  lpm p0l,Z+
  lpm p0h,Z
  add p0l,XL
  adc p0h,XH
  rjmp bin2bcdn ; bin2bcdn(p0h:p0l,p1l)
2:
.word 500,50,5,0
#endif // USE_EXTRACTFRACT

#if defined(USE_EXTRACTINT) || defined(USE_EXTRACTFRACT)
bin2bcdn:
  ldi ZH,hi8(3f)
  ldi ZL,lo8(3f)
  clr p2h ; старшая часть результата
  clr p1h ; младшая часть результата
0:rcall 0f ; подсчитать число тысяч/десятков/сотен/едениц
  mov XL,p2l
  ldi p2l,4
1:lsl p1h
  rol p2h
  dec p2l
  brne 1b
  add p1h,XL
  dec p1l
  brne 0b
  mov p0h,p2h
  mov p0l,p1h
  ret
0:clr p2l
  lpm XL,Z+
  lpm XH,Z+
1:sub p0l,XL
  sbc p0h,XH
  brcs 2f
  inc p2l
  rjmp 1b
2:add p0l,XL
  adc p0h,XH
  ret
3:; тысячи, сотни, десятки, и еденицы
.word 0x03e8, 0x0064, 0x000a, 0x0001

abs16:
  sbrs p0h,7 ; минус?
  ret
  mov XL,p0l
  mov XH,p0h
  clr p0l
  clr p0h
  sub p0l,XL
  sbc p0h,XH
  ret

#endif // USE_EXTRACTINT || USE_EXTRACTFRACT

.data

#ifdef SUPPORT_PARASITE_MODE
cur_mode:
.byte 0 ; 12bit=0, 11bit=1, 10bit=2, 9bit=3, parasite power?=(bit n.7)
#endif // SUPPORT_PARASITE_MODE

cur_sensor_id:
.byte 0, 0,0,0,0,0,0,0,0

#ifdef SUPPORT_ROMSEARCH
last_discrepancy:
.byte 0,0
#endif // SUPPORT_ROMSEARCH
