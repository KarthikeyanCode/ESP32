//code to scan for address of i2c device (lcd in our case)

#include<Wire.h>

int flag=0;

void setup() {
 Serial.println("I2C Scanner");
}

void loop() {
  if(flag==0)
  {
    for(int i=1;i<127;i++)
      {
        Wire.beginTransmission(i);
        if(Wire.endTransmission() == 0)//checking the return value of Wire.endTransmission, if it is 0, then transmission was successfull and device address is correct
        {
          Serial.print("I2C Device found at: ");
          Serial.println(i, HEX);
          Serial.println("Done");
          flag = 1;
          break;
        }
      }
  }
}
