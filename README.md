This is a code for a LPC1769 based IC tester. 

It's based on usbd_lib_cdc example from LPCOpen library, as one of the needed features was USB VCOM port. 
Project is a self contained build system, it does not need any additional sources. Everything needed to build is included, except the compiler.
To build - use arm-none-eabi-gcc compiler from MCUXpresso suite.

This project uses a simple custom board with LPC1769, USB port and two breakout headers for GPIO pins. 
I'm using board.h for NXP LPCXpresso 1769 development board.

Tester has 32 GPIO pins, and uses 4 TI TXB0108 bidirectional level translators to have 3.3V/5V compatibility.


