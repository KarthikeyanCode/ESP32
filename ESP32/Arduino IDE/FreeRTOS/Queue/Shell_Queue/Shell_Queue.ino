#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define LED 2

int LED_Delay=1000;
int counter=0;

String command;

//Queue handlers
static QueueHandle_t queue_1;
static QueueHandle_t queue_2;

//Task Handlers:
static TaskHandle_t task_A;
static TaskHandle_t task_B;

void taskA(void *parameters)
{
  while(1)
  {
    int delay_value = 0;
    String echo_string = "";
    String response = "";
    String delay_time = "";
    command = "";
    while(Serial.available())
    {
      //Serial.println(Serial.available());
      char c = Serial.read();
      command += c;
    }//Delay 78
    if(command.indexOf("Delay") != -1)
    {
      Serial.println("test");
      delay_time = command.substring(6, command.length()-1);
      delay_value = (delay_time.toInt())*1000; //to get the delay time in ms 
      xQueueSend(queue_1, (void*)&delay_value, 10);
    }
    else//echoing it back into the serial monitor 
    {
      if(command.indexOf("echo") != -1)
      {
        echo_string = command.substring(5, command.length()-1);
        Serial.println(echo_string);
      }
    }
    if(xQueueReceive(queue_2, (void*)&response, 0) == pdTRUE)
    {
      Serial.print("Number of times the LED has blinked: ");
      Serial.println(response);
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void taskB(void *parameters)
{
  while(1)
  {
    int delay_value;
    String response = "";
    if(xQueueReceive(queue_1, (void*)&delay_value, 0) == pdTRUE)//read the delay value from queue1
    {
      //update the delay value
      LED_Delay = delay_value; 
    }
    if((counter%10 == 0) && (counter != 0))
    {
      response = String(counter);
      response += '\0';
      xQueueSend(queue_2, (void*)&response, 10);
      vTaskDelay(3000/portTICK_PERIOD_MS);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  xTaskCreatePinnedToCore(taskA, "TASKA", 6000, NULL, 1, &task_A, app_cpu); 
  xTaskCreatePinnedToCore(taskB, "TASKB", 6000, NULL, 1, &task_B, app_cpu);

  queue_1 = xQueueCreate(10, sizeof(int));//integer delay queue - to change LED delay
  queue_2 = xQueueCreate(10, sizeof(String));//repsonse string queue - to print to serial monitor
}

void loop() {
  //Serial.println(LED_Delay);
  digitalWrite(LED, HIGH);
  vTaskDelay(LED_Delay/portTICK_PERIOD_MS);
  digitalWrite(LED, LOW);
  vTaskDelay(LED_Delay/portTICK_PERIOD_MS);
  counter++;
}
