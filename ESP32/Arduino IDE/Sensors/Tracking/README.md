# Interfacing KY-033 tracking sensor module with ESP32 with LCD and buzzer 

## Hardware Required

- ESP32, LCD (with I2C module), buzzer, KY-033 tracking sensor module, bread-baord and jumper wires.

- I2C pins SDA and SCL of LCD module must be connected to GPIO 21 and 22 respectively, which are the default I2C pins in ESP32.

- The buzzer and tracking sensor module is connected to GPIO 4 and 15 respectively.
The vcc and GND of all modules are connnected to 3.3V and GND of ESP32 through bread-board.

- Upload the code to ESP32 through arduino IDE and check the status through serial monitor and lcd.