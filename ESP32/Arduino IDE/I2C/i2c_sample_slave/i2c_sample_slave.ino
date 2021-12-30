#include<Wire.h>

String temp_string;
float temp;

void setup() {
  Serial.begin(115200);
  Wire.begin(1);
  Wire.onReceive(onrec);
}

void loop() {
  delay(10000);//delay of 10 secs
  Serial.println(temp);//log the value of temp to the serial monitor 
}

void onrec(int bytes)
{
  temp_string = "";
  while(Wire.available())
  {
    char c = Wire.read();
    temp_string += c;
  }
  temp = temp_string.toFloat();
}
