/******************************************************************************
  libds18b20 - Library for working with ds18b20 compatible term sensors
  Copyright (c) 2009, Maxim Pshevlotski <mpshevlotsky@gmail.com>

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

#include <stdint.h>
#define RES_9BIT  0x1F
#define RES_10BIT 0x3F
#define RES_11BIT 0x5F
#define RES_12BIT 0x7F

typedef struct {
  uint8_t t_lsb;
  uint8_t t_msb;
  int8_t t_h;
  int8_t t_l;
  uint8_t config;
  uint8_t reserved[3];
} sspad;

// выбирает текущий датчик по его уникальному коду
uint8_t ds18b20_selectSensor(const uint8_t id[8]);
// выполняет измерение температуры на текущем датчике
void ds18b20_convert();
// читает уникальный код датчика, единственного в сети
uint8_t ds18b20_readROM(uint8_t id[8]);
// возвращает измеренное значение температуры текущего датчика
int16_t ds18b20_readTemp();
// возвращает 1 если текущий датчик использует внешнее питание
uint8_t ds18b20_readPowerSupply();
// загружает внутреннюю память текущего датчика в *p
uint8_t ds18b20_readScratchpad(sspad* p);
// записывает данные в *p в память текущего датчика
void ds18b20_writeScratchpad(const sspad* p);
// копирует состояние памяти текущего датчика в его eeprom
void ds18b20_copyScratchpad();
// восстанавливает состояние памяти текущего датчика из его eeprom
void ds18b20_recall();
// выполняет поиск следующего из устройств на шине 1-wire
int8_t ow_searchROM(uint8_t id[8]);
// выполняет поиск следующего датчика у которого пороговое
//  значение температуры вышло из заданных пределов
int8_t ds18b20_searchAlarm(uint8_t id[8]);
// возвращает целую часть значения температуры из двухбайтовой
//  величины получаемой при помощи readTemp
uint16_t ds18b20_extractInt(int16_t t);
// возвращает дробную часть значения температуры из двухбайтовой
//  величины получаемой при помощи readTemp, округляя до 1/10^r
uint16_t ds18b20_extractFract(int16_t t, uint8_t r);
