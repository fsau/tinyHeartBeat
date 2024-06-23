# HeartBeat controller

## Description

Tiny servo controller for HeartBeat project using a ATtiny13A 8-pin microcontroller.

It reads the analog value of pin B3 for heart rate adjustment. Outputs pulsed 
signals for controlling 3 hobby servos on pins B0, B1 and B2.

## Building

With the avr toolchain installed, just run `make` and it should generate the `.hex` file. Run `make prog` for programming the chip after building automatically.
