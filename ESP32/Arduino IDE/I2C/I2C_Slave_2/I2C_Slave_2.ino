#include<Wire.h>

#define pro_cpu 0
#define app_cpu 1

String rec;
uint8_t flag;

void setup()
{
  Serial.begin(115200);
  Wire.begin(1);
}

void loop()
{
  rec = "";
  while(Wire.available())
  {
    char c = Wire.read();
    rec += c;
    flag = 1;
  }
  if(flag == 1)
  {
    rec += '\0';
    Serial.println(rec);
  }
  Serial.println("IDLE");
  vTaskDelay(1000/portTICK_PERIOD_MS);
  flag = 0;
}
