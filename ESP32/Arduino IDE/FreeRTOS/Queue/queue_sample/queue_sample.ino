#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//queue handler:
static QueueHandle_t queue_1;
static TaskHandle_t task_1;

void simple(void *parameter)
{
  
  int item;
  
  while(1)
  {
    if(xQueueReceive(queue_1, (void*)&item, 0) == pdTRUE)
    {
      Serial.print("Message received: ");
      Serial.println(item);
    }
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}

//task main will run on core 1 with priority 1
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //creating a simple integer queue, returns handle to the queue if queue is created
  queue_1 = xQueueCreate(10, sizeof(int));//returns NULL if queue could not be created due to memory issues
  
  xTaskCreatePinnedToCore(simple, "queue_task", 1024, NULL, 1, &task_1, app_cpu);
  
}

void loop() {
  //sending numbers to queue
  int num=0;
  while(1)
  {
    xQueueSend(queue_1, (void*)&num, 10);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    num++;
  }
}
