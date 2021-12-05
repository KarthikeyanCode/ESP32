//timer samples adc pin values 10 times each second
//TaskA computes the average each second and updates the global variable
//TaskB interpretes user commands, associates with shell

//restricting to single core
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//rotary encoder pins
#define CLK 5
#define DT 18

//task handles 
static TaskHandle_t task_A;
static TaskHandle_t task_B;
static TaskHandle_t task_enc;

//timer variables
static const uint16_t timer_div = 8; //clock is divided by 80, counter increments by 1MHz or 10^6 each second 
static const uint64_t timer_max_count = 1000000; //max count 

//hardware timer handle 
static hw_timer_t *timer = NULL;

//delays 
static const TickType_t gen_delay = 2000/portTICK_PERIOD_MS;

//semaphore handle
static SemaphoreHandle_t bin_sem = NULL;

//spinlock
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

//queue handle
static QueueHandle_t avg_queue;

//global variables
volatile uint8_t idx = 0;
volatile uint8_t initial = 0;
volatile uint8_t present = 0;
int flag = 0;
volatile int rot_pos;
volatile int buf[10];


//isr function
void IRAM_ATTR hw_timer_isr()
{
  BaseType_t TaskWake_token = pdFALSE; 
  if(xSemaphoreTakeFromISR(bin_sem, &TaskWake_token) == pdTRUE)
  {
    //portENTER_CRITICAL_ISR(&spinlock);
    buf[idx++] = rot_pos;
    //portEXIT_CRITICAL_ISR(&spinlock);
    if(idx == 10)
    {
      idx = 0;
      vTaskNotifyGiveFromISR(task_A, &TaskWake_token);
      if(TaskWake_token == pdTRUE)
      {
        portYIELD_FROM_ISR();
      }
    }
    xSemaphoreGiveFromISR(bin_sem, &TaskWake_token);
  } 
}

//task functions
void taskA(void *parameters)//computes the average after 10 samples are updated
{
  float average;
  int sum;
  uint8_t cycles = 0;
  while(1)
  {
    average = 0.00;
    sum = 0;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if(xSemaphoreTake(bin_sem, portMAX_DELAY) == pdTRUE)
    {
      //critical section .. no isr shld be activated during this section
      portENTER_CRITICAL_ISR(&spinlock);
      for(int i=0;i<10;i++)
      {
        sum += buf[i];
      }
      portEXIT_CRITICAL_ISR(&spinlock);
      xSemaphoreGive(bin_sem);
    }
    average = (float)sum/10;
    xQueueSend(avg_queue, (void*)&average, portMAX_DELAY);
    cycles++;
    if(cycles == 10)
    {
      //10 averages are calculated then reset the queue and set the cycles value back to 0
      xQueueReset(avg_queue);
      cycles = 0;
    }
  }
}
  
   

void taskB(void *parameters) //interprets shell for user commands 
{
  char c;
  String command;
  float avg;
  while(1)
  {
     command = "";
     while(Serial.available())
     {
        char c = Serial.read();
        command += c;
     }
     //Serial.println(flag);
     command += '\0';
     if(command.indexOf("avg") != -1)//average command
     {
        xQueueReceive(avg_queue, (void*)&avg, 0);
        Serial.println(avg);
     }
     else if(command != "\0")//any other command
     {
        Serial.println(command);
     }
     vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

//computes the position of the rotary encoder 
void read_enc(void* parameters)
{
  while(1)
  {
    //computing the position of rotary encoder 
    present = digitalRead(CLK);
    if(present != initial)
    {
      if(present != digitalRead(DT))//clockwise 
      {
        rot_pos++;
      }
      else //anti-clockwise
      {
        rot_pos--;
      }
    }
    initial = present;
  }
}

//task main .. runs on core 1 with priority 1
void setup() 
{
  Serial.begin(115200);

  //rotary encoder pins
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  initial = digitalRead(CLK);

  //binary semaphore
  bin_sem = xSemaphoreCreateBinary();
  
  if(bin_sem == NULL)
  {
    Serial.println("Semaphore could not be created");
  }

  //give the semaphore first
  xSemaphoreGive(bin_sem);
  
  //create and start timer 
  timer = timerBegin(0, timer_div, true);//arguments: (num, divider, coutup(t/f))

  //attach ISR to timer
  timerAttachInterrupt(timer, &hw_timer_isr, true);//arguments: (timer, &isr_function, edge(t/f))

  //count at which the ISR is to be triggered
  timerAlarmWrite(timer, timer_max_count, true);//argumens: (timer, max_count, autoreload(t/f))

  //queue creations
  avg_queue = xQueueCreate(10, sizeof(float));

  //task creations 
  xTaskCreatePinnedToCore(taskA, "TASKA", 1024, NULL, 2, &task_A, app_cpu);
  xTaskCreatePinnedToCore(taskB, "TASKB", 1024, NULL, 1, &task_B, app_cpu);
  xTaskCreatePinnedToCore(read_enc, "SERVICE_1", 1024, NULL, 1, &task_enc, app_cpu);
    
  //enable the timer 
  timerAlarmEnable(timer);

  //delete main task
  vTaskDelete(NULL);
}

void loop() 
{
  
}
