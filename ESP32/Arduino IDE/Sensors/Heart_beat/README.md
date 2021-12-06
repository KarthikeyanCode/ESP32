# Interfacing KY-039 Sensor with ESP32

## Hardware required

- Two 2.2K ohms to 2.7K ohms resistors, jumpers, ESP32 and KY-039 sensor module.

- The output data signal of KY-039 is of 5v, since ESP32 is a 3.3v board we have to step down the voltages

- Therefore we connect two 2.7K ohm (any value within above range can be used), between the sensor and ESP32. 

- GND pin of sensor is connected to GND of ESP32 through a resistor and Vcc pin of sensor is connected to 3.3v pin of ESP32 through another resistor. The data pin is directly connected to the ADC GPIO, which is 36 in my case.

- Upload the code to ESP32 and open the serial plotter to observe the output.