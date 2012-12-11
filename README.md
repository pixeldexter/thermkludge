thermkludge
===========

An AVR-mega8 based temperature logger

Why thermkludge
---

Because it is.. er.. kludge. Yet, it is still a THERMograph.

What it does
---

It records temperature over a period of time, that's all. No fancy UI, no triggers, no programmability, no SD cards, just bare flash and 1-wire temperature sensor (ds18b20).

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
- two optional leds (red and green preferably)
- a couple of 0.1uF capacitors for decoupling (optional)
- a push-button switch (optional)

Connect DS18b20's VCC (pin3) to PC4, DATA (pin2) to PC5 (these are configurable details). Flash the firmware and you are all set.

Operation
---

Bugs and TODO
---

Credits
---
