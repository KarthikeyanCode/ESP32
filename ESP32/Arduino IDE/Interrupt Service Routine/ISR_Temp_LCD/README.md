# ISR based temperature display with FreeRTOS

- Hardware components used: ESP32 Devkit v1, SPI LCD, KY-001 Temperature sensor,
SMD RGB module Ky-009, KY-036 Touch sensor and jumpers. 

- When touch is detected on the touch sensor, red light is flashed by LED and LCD displays the temperature of the room.

- Led will be in green state when there is no interrupt.