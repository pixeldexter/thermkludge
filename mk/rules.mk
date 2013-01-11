#
# default rules for avr-gcc programs
#
# Control variables
#
#  TARGET   - name of the executable and .hex file
#  FORMAT   - one of: ihex, binary
#  CPU      - cpu as specified in gcc
#  OPTIMIZE - one of: size, or level
#
#  CSOURCES   - list of .c source files
#  ASOURCES   - list of .S source files (asm)
#  CXXSOURCES - list of .cpp source files

#  INCLUDES - list of include directories
#  DEFINES  - list of definitions (comma or space separated)
#  NDEBUG   - disable debugging code


# tools
CC:=avr-gcc
CXX:=avr-g++
AS:=avr-gcc
LD:=avr-ld
OBJCOPY:=avr-objcopy
AVRLIB:=/usr/lib/avr

# defaults: optimization
ifeq ($(OPTIMIZE),)
COMMON_FLAGS += -Os
else ifeq ($(OPTIMIZE),size)
COMMON_FLAGS += -Os
else
COMMON_FLAGS += -O$(OPTIMIZE)
endif
#defaults: debugging
ifdef NDEBUG
COMMON_FLAGS += -DNDEBUG=1
else
COMMON_FLAGS += -g
endif
# defaults: other options
COMMON_FLAGS += -mmcu=$(CPU)
COMMON_FLAGS += -Wall -Wextra
COMMON_FLAGS += $(addprefix -D,$(DEFINES))
COMMON_FLAGS += $(addprefix -I,$(INCLUDES)) -I$(AVRLIB)/include

# specific flags
ASFLAGS += $(COMMON_FLAGS)
ASFLAGS += -D__SFR_OFFSET=0

CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS)

LDFLAGS += $(COMMON_FLAGS)
LDFLAGS += -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += -B$(AVRLIB)/lib

# directories
OBJDIR ?= .obj
DEPDIR ?= .dep

#
# build mechanics
#
AOBJECTS:=$(addprefix .obj/,$(notdir $(ASOURCES:.S=.o)))
COBJECTS:=$(addprefix .obj/,$(notdir $(CSOURCES:.c=.o)))
CXXOBJECTS:=$(addprefix .obj/,$(notdir $(CXXSOURCES:.cpp=.o)))
OBJECTS:=$(AOBJECTS) $(COBJECTS) $(CXXOBJECTS)

all: build

hex: $(TARGET).flash.hex $(TARGET).eeprom.hex

build: $(TARGET).elf
	avr-size -A -d $^

release:
	$(MAKE) NDEBUG=1 build

$(TARGET).flash.hex: $(TARGET).elf
	$(OBJCOPY) -O $(FORMAT) -R .eeprom -R .fuse -R .noinit $< $@

$(TARGET).eeprom.hex: $(TARGET).elf
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 --no-change-warnings -O $(FORMAT) $< $@ || exit 0

$(TARGET).elf: $(OBJECTS)
	$(CC) $^ --output $@ $(LDFLAGS)

$(OBJECTS): | $(OBJDIR)

$(DEPENDS): | $(DEPDIR)

$(OBJDIR) $(DEPDIR):
	-mkdir $@

$(AOBJECTS):
.obj/%.o: %.S
	$(AS) -c $(ASFLAGS) $< -o $@

$(COBJECTS):
.obj/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(CXXOBJECTS):
.obj/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

#
# utility
#
.PHONY: clean build release all flash

clean:
	-rm -f $(TARGET).flash.hex $(TARGET).eeprom.hex $(TARGET).elf
	-rm -f $(OBJECTS)
