Bootloader works at 19200Baud
It uses standard STK500/arduino commands, but since LIN is half duplex some modifications is needed. (Read back every sent byte)

To enter the bootloader, 2 ways exists.
Pullning HWR-pin (PC3) low will enter bootloader unconditionally.
Sending [BREAK]+[SYNC]+[0x3B] (plus parity=0x7B) will enter bootloader if skip_loader EEPROM location is unprogrammed.

Bootloader is running with a watchdog at 8sec, reset by READ and WRITE commands.
It will require that the application code has a start segment of .text=0x200 in the linker memory settings.

Please note that the boatloader enables pull-up on all IO-pins.

BOOTEND=4.




