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

#define temp  r0
#define th    r19
#define tl    r18

.macro delay_ms t ; задержать выполнение на t миллисекунд
  .set x, ((\t)*F_CPU)/(770*1000)
  .if (x == 0)
    delay_us (\t)*1000
  .else
    ldi tl,lo8(x)
    ldi th,hi8(x)
    rcall _delay
  .endif
.endm

.macro delay_us t ; задержать выполнение на t микросекунд
  .set x, ((\t)*F_CPU)/(770*1000000)
  .if (x == 0)
    delay_ns (\t)*1000
  .else
    ldi tl,lo8(x)
    ldi th,hi8(x)
    rcall _delay
  .endif
.endm

.macro delay_ns t ; задержать выполнение на t наносекунд
   ldi tl,((\t)*F_CPU)/(3*1000000*1000)
99:dec tl
   brne 99b ; 3 такта
.endm

; задержка на заданное значение
_delay: ; до 770*65535/F_CPU, сек
   clr temp
99:com temp
98:dec temp
   brne 98b
   subi tl,1
   sbci th,0
   brne 99b ; 770 тактов за цикл
   ret
