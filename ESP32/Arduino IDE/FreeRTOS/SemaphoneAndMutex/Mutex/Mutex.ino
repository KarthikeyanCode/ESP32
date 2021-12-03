//working with mutex and semaphores
//implementing mutex 

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

char buf[20];

static TaskHandle_t task_A;
static TaskHandle_t task_B; 

static SemaphoreHandle_t mutex;

void taskA(void *paramter)
{
  char bufA[20] = {'T', 'A', 'S', 'K', 'A', '\0'};
  while(1)
  {
    if(xSemaphoreTake(mutex, 0) == pdTRUE)
    {
      memcpy(buf, bufA, 20);
      xSemaphoreGive(mutex);
      Serial.println(buf);
    }
  }
}

void taskB(void *parameter)
{
  char bufB[20] = {'T', 'A', 'S', 'K', 'B', '\0'};
  while(1)
  {
    if(xSemaphoreTake(mutex, 0) == pdTRUE)
    {
      memcpy(buf, bufB, 20);
      xSemaphoreGive(mutex);
      Serial.println(buf);
    }
  }
}

void setup() {
  Serial.begin(115200);

  vTaskDelay(1000/portTICK_PERIOD_MS);
  
  mutex = xSemaphoreCreateMutex();

  //creating tasks
  xTaskCreatePinnedToCore(taskA, "TASKA", 1024, NULL, 1, &task_A, app_cpu);
  xTaskCreatePinnedToCore(taskB, "TASKB", 1024, NULL, 1, &task_B, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
    
}
