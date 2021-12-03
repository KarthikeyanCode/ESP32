//FreeRTOS Memory management

//check if single core is enabled already
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static TaskHandle_t task_A = NULL;
static TaskHandle_t task_B = NULL;

//char *ptr = (char*)pvPortMalloc(1024*sizeof(char));

void taskA(void *parameter)
{
  char c;
  Serial.println("Heap size in bytes:");
  Serial.println(xPortGetFreeHeapSize());
  while(1)
  { 
    //i and flag shld be reassinged again and again
    int i = 0;
    int flag = 0;
    char *ptr = (char*)pvPortMalloc(1024*sizeof(char));
    vTaskDelay(1000/portTICK_PERIOD_MS);
    //Serial.println(flag);
    while(Serial.available())
    {
      char c = Serial.read();
      if(c == '\n')
      {
        *(ptr+i) = '\0';
        flag = 1;
        break;
      }
      *(ptr+i) = c;
      i += 1;
    }
    if(flag == 1)
    {
      xTaskCreatePinnedToCore(taskB, "TASKB", 2048, (void*)ptr, 1, &task_B, app_cpu);
      vTaskSuspend(NULL);
    }
    vPortFree(ptr); 
  }
  /*
  Serial.println("Bytes of task stack left (bytes):");
  Serial.println(4*uxTaskGetStackHighWaterMark(NULL));
  Serial.println("Heap size in bytes:");
  Serial.println(xPortGetFreeHeapSize());
  vTaskDelete(NULL);
  */
}

void taskB(void *parameter)
{
  char *ptr1 = (char*)parameter;
  Serial.println(ptr1);
  Serial.println();
  vTaskResume(task_A);
  vTaskDelete(NULL);
}

void setup() 
{
  Serial.begin(115200);
  vTaskDelay(500/portTICK_PERIOD_MS);

  xTaskCreatePinnedToCore(taskA, "TASKA", 2048, NULL, 1, &task_A, app_cpu);
  //xTaskCreatePinnedToCore(taskB, "TASKB", 2048, NULL, 1, &task_B, app_cpu);
  //vTaskDelete(NULL);//delete task associated with setup and loop
  vTaskDelete(NULL);
  sleep();
}

void loop() 
{
  
}
