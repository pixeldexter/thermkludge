#!/bin/sh

SRC=ds18b20.S

CFLAGS=-Wall -x assembler-with-cpp -mmcu=atmega644
PROG=$(basename $(SRC))

.PHONY : clean

all: .$(PROG).o # pack
	avr-ar rcs lib$(PROG).a .$(PROG).o 
	avr-size -d $^

.$(PROG).o: $(SRC) # compile
	avr-gcc $(CFLAGS) -o $@ -c $(SRC)

clean: #clean
	- rm -f lib$(PROG).a
	- rm -f .$(PROG).o
