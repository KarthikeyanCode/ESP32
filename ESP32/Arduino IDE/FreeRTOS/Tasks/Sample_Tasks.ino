//Iplementation of RTOS concepts using FreeRTOS and ESP32

//Using 1 core of ESP32
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//using task handles so that we can control their states from outside the task (another task)

static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;

void task1(void* parameter)
{
  while(1)
  {
    for(int i=0;i<30;i++)
    {
      Serial.print("_");
    }
    Serial.println();//new line 
  } 
}

void task2(void* parameter)
{
  while(1)
  {
    Serial.print("*");
    vTaskDelay(500/portTICK_PERIOD_MS); 
  } 
}

//main runs as its own task with priority 1 on core 1

void setup()
{
  Serial.begin(300);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println("Setup and loop are running on core:");
  Serial.println(xPortGetCoreID());
  Serial.println("With priority:");
  Serial.println(uxTaskPriorityGet(NULL));

  //Create a task and pin it to the core
  xTaskCreatePinnedToCore(task1, "TASK_1", 1024, NULL, 1, &task_1, app_cpu); 
  xTaskCreatePinnedToCore(task2, "TASK_2", 1024, NULL, 2, &task_2, app_cpu);
  
}

void loop()
{
  vTaskDelay(10000/portTICK_PERIOD_MS);
  /*if(task_1 != NULL && task_2 != NULL)
  {
    vTaskDelete(task_1);//delete task1
    vTaskDelete(task_2);//delte task2
  }*/
}
