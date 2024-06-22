# HeartBeat controller

## Description

Tiny servo controller for HeartBeat project using a ATtiny13A microcontroller.

It reads the analog value of pin B3 for beat frequency adjustment. Outputs pulsed 
signals for controlling 3 independent hobby servomotors.

## Building

With the avr toolchain insatalled, just run `make` and it should generate the `.hex` file. Run `make prog` for programming the chip after building automatically.
