A simple timer that will last 20 seconds. Uses the STC15W201S MCU. 
A button press will start 20 seconds of music and flashing LED. A second press before the end of music wil stop the timer.

Building
---
See comments in first lines of `geneeprom.c` and `ponzioPilato2.c`

EEPROM data
---
Use `geneeprom.c` to generate `eeprom.bin` and load the bin in STC-ISP

Connections
---
- button on P3.2 (INT0) and GND
- buzzer on P1.0 and P1.1
- LED with resistor on P5.5

STC-ISP settings
---
- Enable low voltage reset: OFF
- Low voltage level: 2.96V (triggers at ~3.4V, for lithium battery)
- EEPROM data from `eeprom.bin`

License
---
This software is licensed under the [Unlicense](https://unlicense.org/) terms and conditions