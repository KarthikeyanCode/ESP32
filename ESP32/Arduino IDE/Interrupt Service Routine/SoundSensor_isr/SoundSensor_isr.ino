//ISR with sound sensor 

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static TaskHandle_t task_1;
const uint8_t LED = 4;


struct ky038
{
  const uint8_t pin = 15;
  bool sound = false;
}s1;


void IRAM_ATTR ky038_isr()
{
  s1.sound = true;
}


void task1(void *parameters)
{
  while(1)
  {
    if(s1.sound == true)
    {
      Serial.println("Sound Detected");
      digitalWrite(LED, HIGH);
      vTaskDelay(1000/portTICK_PERIOD_MS);
      digitalWrite(LED, LOW);
      s1.sound = false;
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void setup() 
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(s1.pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(s1.pin), ky038_isr, HIGH);//After the sound is recognized
  xTaskCreatePinnedToCore(task1, "KY038_TASK", 1024, NULL, 1, &task_1, app_cpu);
  vTaskDelete(NULL);
}

void loop() 
{
 
}
