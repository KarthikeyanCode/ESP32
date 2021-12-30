//I2C master ESP32 for Register C I2C slave Arduino 

#include<Wire.h>

#define pro_cpu 0
#define app_cpu 1

uint8_t Byte = 'k';

void setup() 
{
  Serial.begin(115200);
  Wire.begin();
}

void loop() 
{
  Serial.println("I2C Master has begun");
  Wire.beginTransmission(1);
  Wire.write(Byte);
  Wire.endTransmission();
  vTaskDelay(2000/portTICK_PERIOD_MS);
}
