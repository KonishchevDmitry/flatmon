# Boards pinout reference

## Serial

**Uno:** 0, 1

**Mega:** 0, 1, 14 - 19


## PWM and timers

Uno has 3 timers and 6 PWM outputs:
- Pins 5 and 6: controlled by Timer0
- Pins 9 and 10: controlled by Timer1
- Pins 11 and 3: controlled by Timer2

Mega has 6 timers and 15 PWM outputs:
- Pins 4 and 13: controlled by Timer0
- Pins 11 and 12: controlled by Timer1
- Pins 9 and 10: controlled by Timer2
- Pin 2, 3 and 5: controlled by Timer3
- Pin 6, 7 and 8: controlled by Timer4
- Pin 46, 45 and 44: controlled by Timer5


## External Interrupts

**Uno:** 2, 3

**Mega:** 2, 3, 18, 19, 20, 21


## I2C/TWI

**Uno:** A4 (SDA), A5 (SCL)

**Mega:** 20 (SDA), 21 (SCL)


## SPI

**Uno:** 10 (SS), 11 (MOSI), 12 (MISO), 13 (SCK)

**Mega:** 50 (MISO), 51 (MOSI), 52 (SCK), 53 (SS)


## Builtin LED

**Uno/Mega:** 13
