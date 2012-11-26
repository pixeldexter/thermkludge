#
# parameters
#
TARGET:=scanner
FORMAT:=ihex
CPU := atmega8
FLASHER := avreal-ft2232
OPTIMIZE := size

#
# sources
#
vpath %.c ../common
vpath %.S ../common ../ds18b20/

CSOURCES := scanner.c

ASOURCES := 	uart.S \
		ds18b20/ds18b20.S \

FUSES=CKOPT=1,CKSEL=2,BOOTRST=0,BOOTSZ=0,EESAVE=0

#
# specific options
#
INCLUDES := ../
COMMON_FLAGS += -include config.h
CFLAGS += -std=c99

include ../mk/rules.mk
include ../mk/prog.mk

readchip: flash-eeprom-read
	$(OBJCOPY) -I ihex -O binary $(TARGET).eeprom-read.hex $(TARGET).eeprom-read.bin && hexdump -C $(TARGET).eeprom-read.bin
