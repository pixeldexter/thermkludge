#
# MCU programming support
#
# Common variables
#
#  TARGET - target name
#  CPU    - processor name as in gcc
#  FLASHER - one of: avreal-2232, todo
#  FUSES   - fuses list
#
# external targets:
#  $(TARGET).flash.hex
#  $(TARGET).eeprom.hex
#


ifeq ($(FLASHER),avreal-ft2232)
PROG_UTIL:=/opt/bin/avreal
PROG_OPTS += -aft2232:enable=~acbus1 +$(CPU) -2
PROG_OPT_ERASE:=-e
PROG_OPT_READ:=-r+
PROG_OPT_WRITE:=-w
PROG_OPT_CODE:=-c $(TARGET).flash.hex
PROG_OPT_CODE_OUT:=-c $(TARGET).flash-read.hex
PROG_OPT_DATA:=-d $(TARGET).eeprom.hex
PROG_OPT_DATA_OUT:=-d $(TARGET).eeprom-read.hex
PROG_OPT_VERIFY:=-v
PROG_OPT_FUSES := -f$(FUSES) # no space
else
$(error flasher '$(FLASHER)' not supported)
endif


flash: PROG_COMMAND=$(PROG_OPT_ERASE) $(PROG_OPT_WRITE) $(PROG_OPT_CODE) $(PROG_OPT_VERIFY)
flash: $(TARGET).flash.hex __prog

flash-eeprom: PROG_COMMAND=$(PROG_OPT_WRITE) $(PROG_OPT_DATA) $(PROG_OPT_VERIFY)
flash-eeprom: $(TARGET).eeprom.hex __prog

flash-read: PROG_COMMAND=$(PROG_OPT_READ) $(PROG_OPT_CODE_OUT)
flash-read: __prog

flash-eeprom-read: PROG_COMMAND=$(PROG_OPT_READ) $(PROG_OPT_DATA_OUT)
flash-eeprom-read: __prog

flash-fuses: PROG_COMMAND=$(PROG_OPT_WRITE) $(PROG_OPT_FUSES) $(PROG_OPT_VERIFY)
flash-fuses: __prog

__prog:
	$(PROG_UTIL) $(PROG_OPTS) $(PROG_COMMAND)
