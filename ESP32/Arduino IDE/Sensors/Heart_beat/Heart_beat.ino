//Working with the heart beat sensor 

#define sensor 36

void setup() 
{
  Serial.begin(115200);
  pinMode(sensor, INPUT);
}

void loop() 
{
  Serial.println(analogRead(sensor));
  vTaskDelay(100/portTICK_PERIOD_MS);
}
