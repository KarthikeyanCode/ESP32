#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! 
#endif

//LED pin
#define LED 2

//object creation (class instantiation)
BluetoothSerial SerialBT;

void setup() 
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  SerialBT.begin("ESP32"); //Bluetooth device name
  Serial.println("Bluetooth initiated, visible to other devices as ESP32");
}

void loop() 
{

  char input[100];
  char output[100];
  bool flag=false;
  int i=0;

  memset(input, 0, sizeof(input));
  memset(output, 0, sizeof(output));
  
  //if serial input data available transfer it to connected device
  while (Serial.available()) 
  {
    SerialBT.write(Serial.read());
  }

  //if information available to be read from connected device, print it to serial monitor
  while (SerialBT.available()) 
  {
    //Serial.write(SerialBT.read());
    input[i] = SerialBT.read();
    i++;
    if(flag==false)
    {
      flag=true;
    }
  }
  input[i] = '\0';
  if(flag==true)
  {
    Serial.println(input);
    if(!(memcmp(input, "on", 2)))
    {
      digitalWrite(LED, HIGH);
      memcpy(output, "LED ON", 6);
      output[6]='\n';
      output[7]='\0';
      for(int j=0;j<=6;j++)
      {
        SerialBT.write(output[j]);
      }
    }
    if(!(memcmp(input, "off", 3)))
    {
      digitalWrite(LED, LOW);
      memcpy(output, "LED OFF", 7);
      output[7]='\n';
      output[8]='\0';
      for(int j=0;j<=7;j++)
      {
        SerialBT.write(output[j]);
      }
    }
    flag=false;
  }
  delay(100);
}
