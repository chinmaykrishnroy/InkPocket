# Hardware reference

InkPocket targets a Seeed XIAO ESP32-C3 and a 1.54-inch 200×200 GxEPD2 D67
monochrome e-paper panel.

| Signal | XIAO ESP32-C3 GPIO |
| --- | ---: |
| E-paper SCK | 8 |
| E-paper MOSI | 10 |
| E-paper CS | 7 |
| E-paper DC | 21 |
| E-paper RST | 20 |
| E-paper BUSY | 6 |
| BOOT / action button (active-low) | 9 |

All project-owned pin and display settings live in
[`config/HardwareConfig.h`](../config/HardwareConfig.h). Change that file when
adapting the firmware to a different wiring layout or panel profile.

GPIO 9 is the XIAO BOOT button. It supports this project’s light-sleep wake flow
but is not an ESP32-C3 RTC-domain wake pin; deep sleep needs different hardware
and a suitable external wake source.
