//ESP32 - RGB controller using FreeRTOS


#define pro_cpu 0
#define app_cpu 1

#define red_1 4
#define green_1 2
#define blue_1 15

static TaskHandle_t task_1;
static TaskHandle_t task_2;

volatile uint8_t flag=0;
int len;


void Task_1(void* parameters)
{
  while(1)
  {
    digitalWrite(blue_1, LOW);
    digitalWrite(green_1, LOW);
    digitalWrite(red_1, HIGH);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    digitalWrite(red_1, LOW);
    digitalWrite(green_1, LOW);
    digitalWrite(blue_1, HIGH);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    digitalWrite(red_1, LOW);
    digitalWrite(blue_1, LOW);
    digitalWrite(green_1, HIGH);
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}


void Task_2(void* parameters)
{
  while(1)
  {
    char input[50];
    input[0] = '\0';
    len=0;
    while(Serial.available())
    {
      char c = Serial.read();
      input[len] = c;
      len++;
      flag=1;
    }
    input[len-1] = '\0';
    if(flag==1)
    {
      Serial.print("Message Received: ");
      Serial.print("[");
      Serial.print(input);
      Serial.println("]");
      if(memcmp(input, "on", 2) == 0)
      {
        vTaskResume(task_1);  
      }
      else if(memcmp(input, "off", 3) == 0)
      {
        digitalWrite(red_1, LOW);
        digitalWrite(green_1, LOW);
        digitalWrite(blue_1, LOW);
        vTaskSuspend(task_1);
      }
      flag=0; 
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(red_1, OUTPUT);
  pinMode(green_1, OUTPUT);
  pinMode(blue_1, OUTPUT);
  xTaskCreatePinnedToCore(Task_1, "TASK_1", 2048, NULL, 1, &task_1, app_cpu);
  xTaskCreatePinnedToCore(Task_2, "TASK_1", 2048, NULL, 1, &task_2, app_cpu);
  vTaskDelete(NULL);
}

void loop() 
{
  
}