# Website based temperature forcast

- In this project, arduino UNO collects data from the sensor and trasnfers the data 
  to ESP32 through I2C protocol.

- ESP32 hosts a http server in the WiFi network and broadcasts the temperature 
  information in the website. 

- The users can refresh the website to obtain current information about temperature.

- Temperature sensor is connected to pin 2 of arduino, here i have used KY001 sensor
  and LCD is I2C connected to ESP32. 