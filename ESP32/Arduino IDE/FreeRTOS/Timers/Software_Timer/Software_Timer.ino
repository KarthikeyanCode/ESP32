//sample software timer 

//cnfiguring to run in 1 core
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static TimerHandle_t timer_1;
static TimerHandle_t timer_2;

//callback function, will be called when timer expires
void timer_CallBack(TimerHandle_t xTimer)
{
  if((uint32_t)pvTimerGetTimerID(xTimer) == 0)
  {
    Serial.println("Timer_1 Expired");
  }
  if((uint32_t)pvTimerGetTimerID(xTimer) == 1)
  {
    Serial.println("Timer_2 Expired");
  }
  
}


void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  //arguments: string indicating name of timer, period of timer (in ticks), atuo reload (y/n), timerID, callback function
  timer_1 = xTimerCreate("one_shot_timer", 2000/portTICK_PERIOD_MS, pdFALSE, (void*)0, timer_CallBack);
  timer_2 = xTimerCreate("auto-reload-timer", 2000/portTICK_PERIOD_MS, pdTRUE, (void*)1, timer_CallBack);

  if(timer_1 == NULL || timer_2 == NULL)
  {
    Serial.println("Timers could not be created");
  }
  else
  {
    vTaskDelay(1000/portTICK_PERIOD_MS);
    Serial.println("Starting the timer (one-shot-timer)");
    xTimerStart(timer_1, portMAX_DELAY);
    Serial.println("Starting the timer (auto-reload_timer)");
    xTimerStart(timer_2, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}

void loop() {

}
