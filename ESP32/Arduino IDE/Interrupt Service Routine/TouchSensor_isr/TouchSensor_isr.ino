//KY 036 module with ISR 

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static TaskHandle_t task_1;

const uint8_t buzz_pin = 4;

struct KY_036
{
  const uint8_t pin = 15; // sensor is connected to gpio 15
  bool touch = false;
}s1;

void IRAM_ATTR ky036_isr()
{
  s1.touch = true;
  digitalWrite(buzz_pin, HIGH);
}

void task1(void *parameters)
{
  while(1)
  {
    if(s1.touch == true)
    {
      Serial.println("Touch Detected");
      s1.touch = false;
      vTaskDelay(1000/portTICK_PERIOD_MS);
      digitalWrite(buzz_pin, LOW);
    }
    vTaskDelay(1500/portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(s1.pin, INPUT);
  pinMode(buzz_pin, OUTPUT);
  attachInterrupt(s1.pin, ky036_isr, FALLING);//when the person takes away the hand we record the touch i.e, high to low
  xTaskCreatePinnedToCore(task1, "TASK_KY036", 1024, NULL, 1, &task_1, app_cpu);
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
