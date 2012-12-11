thermkludge
===========

An AVR-mega8 based temperature logger

Why thermkludge
---

Because it is.. er.. kludge. Yet, it is still a THERMograph.

What it does
---

It records temperature over a period of time, that's all. No fancy UI, no triggers, no programmability, no SD cards, just bare flash and 1-wire temperature sensor (ds18b20). The main idea is to keep the firmware as minimal as possible to fit it into bootloader section, and use rest of flash memory for storing data.

Why not buy...
---

Sometimes situation demands a kludge, nothing more, nothing less. Of course there are better implementations around (you name them), but you have to shop first, pay and then wait-wait-wait... This device however is very minimal, it requires only ATmega8 and DS18b20 which are readily available in almost all electronic stores.

Design
---

Again, very minimalistic. The BOM includes:

- 1x ATmega8 (any package you can solder)
- 1x DS18b20
- 1x 4.7kOhm resistor
- 1x 32768Hz crystal
- 2x 22-27pF ceramic capacitors
- two leds (red and green preferably, optional)
- a couple of 0.1uF capacitors for decoupling (optional)
- a push-button switch (optional)

Connect DS18b20's VCC (pin3) to PC4, DATA (pin2) to PC5 (these are configurable details). Flash the firmware and you are all set. The data is written into microcontroller's flash memory, you'll need a programmer to read back the data. Since you just assembled the device you'll probably have it around.

Operation
---

00. Clone the thermkludge repository. Install avr-gcc and your favorite AVR flasher (AVreal is assumed).
0. If you have multiple sensors connected, continue to the next step. To use just one sensor skip straight to step #4.
1. cd scanner/ and build the firmware by calling 'make'.
2. Flash the firmware and the fuses (EESAVE=0). The fuses are critical since scanner stores configuration in the EEPROM, and we don't want to lose it over the next step.
3. Turn ON the power to your kludge and let it scan 1-wire bus and store configuration to EEPROM (wait until both lights go off).
4. cd to ../logger and build the firmware by calling 'make'.
5. Erase the chip, and flash the actual logger firmware. It will use whatever configuration it finds in EEPROM to select DS18b20 chips. If EEPROM is empty, the logger will issue 'SKIP ROM' commands to select ALL chips on bus, this WILL lead to data corruption if there are multiple chips.
6. Disconnect the programmer and connect the battery. You're ready.
7. After you gather some data, cd to ../analyze and run analyze.sh. It will read the device's flash memory and produce a nice graph.

After you turn on the chip with the logger firmware inside it, it light the green led for 5 seconds or so, and then go to idle mode. The RTC will still be ticking, but no data will be recorded. If the button is held down for 1-2 seconds, the logger will go into active mode (which is confirmed by a short green flash) and will record data every 15 seconds (configurable). Holding down the button for 1-2 seconds again will bring the device into idle mode (confirmed by red flash).

An initial delay of 5 seconds is used for erasing the data. To do that, first disconnect the power, then press the button and connect the power back at the same time holding the button down. The led turns red, and if you keep pressing the button for 5 seconds, the memory will be erased. After that the device goes into idle mode.

Bugs and TODO
---

1. ACTUALLY implement a scanner firmware. The major problem here is that 1-wire scanning code in DS18b20 library does not work for some reason, and I'm too old and lazy to debug it. So, I have to find a better library but haven't done so yet. Since I'm busy with the other stuff this is postponed to an indefinite period of time.
2. GET RID of separate scanner firmware. The only reason for having two separate firmwares was that current ds18b20/ with ROM scan enabled does not fit into memory. It can be squeezed in with a few tweaks, but ROM scanning code seems to be broken, and all alternative implementations I could find still do not fit into memory :( So, it is effectively two tasks: find (or write) ROM scanning code that works; squeeze that code into 2kb (together with rest of logic).
3. Do a minor cleanup: some pins need a configuration param instead of being hard-coded; led and button code needs to be moved to platform section and so on. There are some tricks to shave off a couple of bytes here and there...

Credits
---

- Maxim Pshevlotski for DS18b20 library

License
---

GPLv3
