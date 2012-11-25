#
# parameters
#
TARGET:=logger
FORMAT:=ihex
CPU := atmega8
DEFINES = F_CPU=2000000
FLASHER := avreal-ft2232
OPTIMIZE := size

#
# sources
#
vpath %.c ../common
vpath %.S ../common ../ds18b20/

CSOURCES := logger.c rtc.c flashdb.c

ASOURCES := 	uart.S \

FUSES=CKOPT=1,CKSEL=2,BOOTRST=0,BOOTSZ=0,EESAVE=0

#
# specific options
#
INCLUDES := ../
COMMON_FLAGS += -include config.h
CFLAGS += -std=c99
LDFLAGS += -Wl,--section-start=.text=0x1800
LDFLAGS += -Wl,-Tlogger.lds

include ../mk/rules.mk
include ../mk/prog.mk

readchip: flash-read
	$(OBJCOPY) -I ihex -O binary $(TARGET).flash-read.hex $(TARGET).flash-read.bin && hexdump -C $(TARGET).flash-read.bin
