//sample I2C master code ESP32

#include<Wire.h>

byte input = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();//begin wire library for I2C protocol communication
}

void loop() {
  Wire.requestFrom(1, 1);//request from address 1, 1 byte
  if(Wire.available())//when the wire is avialable
  {
    input = Wire.read();//read the incoming byte one by one 
  }
  Serial.println(input);//print the byte received to the serial monitor
  delay(5000);//5 seconds
}
