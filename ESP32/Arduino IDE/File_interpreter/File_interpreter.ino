#include "SPIFFS.h"

File file;

void setup() 
{
  Serial.begin(115200);
  SPIFFS.begin(true);
  file = SPIFFS.open("/test.txt");
}

void loop() 
{
  int input=0;
  while(Serial.available()>1) //not reading the last byte(last character)
  {
    input = (Serial.read())-48; 
  }
  Serial.println(input);
  if(input==0)
  {
    //do nothing 
    //Serial.println("Execution got here");
  }
  else if(input==1)
  {
    Serial.println("Contents of file:");
    while(file.available())
    {
      Serial.write(file.read());
    }
  }
  else if(input==2)
  {
    file.close();
    Serial.println("Closed the program");
    while(1) {}
  }
  else
  {
    Serial.println("Enter only valid numbers");
  }
  vTaskDelay(1000/portTICK_PERIOD_MS);
}
