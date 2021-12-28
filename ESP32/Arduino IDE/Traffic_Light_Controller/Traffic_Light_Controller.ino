//Traffic Light Controller 
//Using ESP32 and FreeRTOS (SMP changes for ESP32)

//Libraries
#include<WiFi.h>

//ESP32 Cores
#define pro_cpu 0
#define app_cpu 1

//GPIOs -----------------------------------------

//sensors
#define FarmSensor 13
#define HighwaySensor 12
#define trigger 14

//Highway lights
#define HighwayRed 5
#define HighwayBlue 18
#define HighwayGreen 19

//Farmroad Lights
#define FarmroadRed 4
#define FarmroadGreen 2
#define FarmroadBlue 15

//------------------------------------------------

//task handlers
static TaskHandle_t highway;
static TaskHandle_t farmroad;
static TaskHandle_t StateMachine;
static TaskHandle_t WebLogger;
static TaskHandle_t TrafficStatus;

//semaphore handler
static SemaphoreHandle_t mutex;

//timers
static TimerHandle_t YellowWait;
static TimerHandle_t FarmGreen;
static TimerHandle_t HighwayGreen_;

//global variables 
char state[100];
uint8_t yellow_flag=0;
uint8_t farmgreen_flag=0;
uint8_t highwaygreen_flag=0;
uint8_t flag;
char* ssid = "Enter your ssid here";
char* password = "Enter your password here";
String client_message;
bool highway_status;
char c;
char prev_c;

//HTTP header - HTTP server
const char* header = "HTTP/1.1 200 OK\n"
                     "Content-type:text/html\n"
                     "Connection: close\n";

//HTML Code (HTTP response) - HTTP server
const char* code1 = "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "<style>\n"
                    ".button\n"
                   "{\n"
                   "  border: none;\n"
                   "  color: #ffffff;\n"
                   "  padding: 15px 32px;\n"
                   "  text-align: center;\n"
                   "  display: inline-block;\n"
                   "  font-size: 16px;\n"
                   "  margin: 4px 2px;\n"
                   "  cursor: pointer;\n"
                   "  background-color: #000000;\n"
                   "}\n"
                   ".myDiv\n"
                   "{\n"
                   "text-align: center;\n"
                   "}\n"
                   "</style>\n"
                   "</head>\n"
                   "<body>\n";

const char* code2 = "<h1> Highway Traffic Status : </h1>\n"
                    "<ul>\n"
                    "<l1>"; //send traffic detection from ultrasonic sensor after this (traffic low or high)

const char* traffic_low = " low ";
const char* traffic_high = " high ";

const char* code3 = "</l1>\n"
                     "</ul>\n"
                     "<h1> State of the Traffic Signal </h1>\n"
                     "<ul>\n"
                     "<l1> "; //send state variable after this 

const char* code4 = " </l1>\n"
                     "</ul>\n"
                     "<div class=\"myDiv\">\n"
                     "<a href=\"/INFO\">\n"
                     "<button class=\"button\">\n"
                     "Refresh\n"
                     "</button>\n"
                     "</a>\n"
                     "</div>\n"
                     "</body>\n"
                     "</html>\n";                           
                 
WiFiServer server(80); //Pinning server to port 80

//StateMachine Task
void State_Machine(void* parameters)
{
  while(1)
  {
    if(xSemaphoreTake(mutex, 0) == pdTRUE)
    {
      if(highwaygreen_flag != 1 || !memcmp(state, "wait", 4))
      {
        Serial.print("Current State: ");
        Serial.println(state);
      }
      if(memcmp(state, "default", 7)==0)
      {
        if(highwaygreen_flag==0)
        {
          xSemaphoreGive(mutex);
          xTimerStart(HighwayGreen_, portMAX_DELAY);
          vTaskSuspend(NULL);
          xSemaphoreTake(mutex, portMAX_DELAY);
        }
        if(!digitalRead(FarmSensor))
        {
          memset(state, 0, sizeof(state));
          memcpy(state, "wait", 4);
        }
      }
      else if(memcmp(state, "wait", 4) == 0)
      {
        xSemaphoreGive(mutex);
        xTimerStart(YellowWait, portMAX_DELAY);
        vTaskSuspend(NULL);
        xSemaphoreTake(mutex, portMAX_DELAY);
        if(yellow_flag==1 && highwaygreen_flag==1)
        {
          memset(state, 0, sizeof(state));
          memcpy(state, "farm", 4);
          yellow_flag=0;
          highwaygreen_flag=0;
        }
        if(yellow_flag==1 && farmgreen_flag==1)
        {
          memset(state, 0, sizeof(state));
          memcpy(state, "default", 7);
          yellow_flag=0;
          farmgreen_flag=0;
        }
      }
      else if(memcmp(state, "farm", 4) == 0)
      {
        xSemaphoreGive(mutex);
        xTimerStart(FarmGreen, portMAX_DELAY);
        vTaskSuspend(NULL);
        xSemaphoreTake(mutex, portMAX_DELAY);
        memset(state, 0, sizeof(state));
        memcpy(state, "wait", 4);
      }
      xSemaphoreGive(mutex);
      vTaskDelay(50/portTICK_PERIOD_MS);
    }
  }
}

//highway task fucntion
void HighWay(void* parameters)
{
  while(1)
  {
    if(xSemaphoreTake(mutex, 0) == pdTRUE)
    {
      if(memcmp(state, "default", 7)==0)
      {
        //Serial.println("here");
        digitalWrite(HighwayRed, LOW);
        digitalWrite(HighwayBlue, LOW);
        digitalWrite(HighwayGreen, HIGH);
        
      }
      else if(memcmp(state, "wait", 4) == 0)
      {
        digitalWrite(HighwayBlue, LOW);
        digitalWrite(HighwayGreen, HIGH);
        digitalWrite(HighwayRed, HIGH);
      }
      else if(memcmp(state, "farm", 4) == 0)
      {
        digitalWrite(HighwayGreen, LOW);
        digitalWrite(HighwayBlue, LOW);
        digitalWrite(HighwayRed, HIGH);
      }
      else
      {
        Serial.println("Error in the state machine");
      }
      xSemaphoreGive(mutex);
      vTaskDelay(2/portTICK_PERIOD_MS);
    }
  }
}

//farmroad task function
void FarmRoad(void* parameters)
{
  while(1)
  {
    if(xSemaphoreTake(mutex, 0) == pdTRUE)
    {
      if(memcmp(state, "default", 7)==0)
      {
        digitalWrite(FarmroadGreen, LOW);
        digitalWrite(FarmroadBlue, LOW);
        digitalWrite(FarmroadRed, HIGH);
      }
      else if(memcmp(state, "wait", 4) == 0)
      {
        digitalWrite(FarmroadBlue, LOW);
        digitalWrite(FarmroadGreen, HIGH);
        digitalWrite(FarmroadRed, HIGH);
      }
      else if(memcmp(state, "farm", 4) == 0)
      {
        digitalWrite(FarmroadBlue, LOW);
        digitalWrite(FarmroadRed, LOW);
        digitalWrite(FarmroadGreen, HIGH);
      }
      else
      {
        Serial.println("Error in the state machine");
      }
      xSemaphoreGive(mutex);
      vTaskDelay(2/portTICK_PERIOD_MS);
    }
  }
}

void Web_Logger(void* parameters)
{
  while(1)
  {
    WiFiClient client = server.available();
    if(client)//new client
    {
      Serial.println("New Client");
      if(client.connected())//while the client is still connected 
      {
        //Serial.println("Client Connected");
        //client_message = "";
        while(client.available())//information available to be read from client 
        {
            c = client.read();
            client_message += c;
            if(c=='\n' && prev_c=='\n')
            {
              //flag=1; 
              break;
            }
            prev_c = c;
        }
        //if(flag==1)
        //{
        //Sending the HTTP header
        client.print(header);
        client.println();

        //sending the required HTTP Code - HTTP response
        if(client_message.indexOf("GET /INFO") >= 0)
        {
          client.print(code1);
          client.print(code2);
          if(highway_status == true)
          {
            client.print(traffic_high);
          }
          else
          {
            client.print(traffic_low);
          }
          client.print(code3);
          client.print(state);
          client.print(code4);
          client.println();
        }
        else
        {
          client.print(code1);
          client.print(code2);
          if(highway_status == true)
          {
            client.print(traffic_high);
          }
          else
          {
            client.print(traffic_low);
          }
          client.print(code3);
          client.print(state);
          client.print(code4);
          client.println();
        }
          //flag=0;
        //}
        client.stop();
      }
      else
      {
        Serial.println("Waiting for client to connect");
      }
    }
    //yeilding the first core
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

void Traffic_Status(void* parameters)
{
  float duration, distance;
  while(1)
  {
    //giving the trig pulse 
    digitalWrite(trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);

    //duration to catch the incoming pulse
    duration = pulseIn(HighwaySensor, HIGH);

    //computing distance 
    distance = (duration * 0.034)/2; //in cm
   
    //setting the status
    if(distance <= 10)
    {
      highway_status = true;
    }
    else
    {
      highway_status = false;
    }

    //yeilding the processor 
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

//timer call back functions
//yellow light callback function
void Yellow_Wait(TimerHandle_t xTimer)
{
  yellow_flag = 1;
  vTaskResume(StateMachine);
}

//Farmroad Green Timer
void Farm_Green(TimerHandle_t xTimer)
{
  farmgreen_flag = 1;
  vTaskResume(StateMachine);
}

//Highway Green Timer
void Highway_Green(void* parameters)
{
  highwaygreen_flag = 1;
  vTaskResume(StateMachine);
}

void setup() 
{
  //Starting the serial monitor
  Serial.begin(115200);

  //Variable Initialization 
  highway_status = false;
  prev_c = 's';
  flag=0;
  
  //********************************************************

  //Setting up the HTTP server -----------------------------
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
  
  server.begin();

  //-------------------------------------------------------

  //*******************************************************
  
  //GPIO configuration ------------------------------------
  
  //Outputs
  pinMode(HighwayRed, OUTPUT);
  pinMode(HighwayBlue, OUTPUT);
  pinMode(HighwayGreen, OUTPUT);
  pinMode(FarmroadRed, OUTPUT);
  pinMode(FarmroadBlue, OUTPUT);
  pinMode(FarmroadGreen, OUTPUT);
  pinMode(trigger, OUTPUT);

  //Inputs
  pinMode(FarmSensor, INPUT);
  pinMode(HighwaySensor, INPUT);

  //-------------------------------------------------------

  //*******************************************************

  //initializing the state
  state[0] = '\0';
  memcpy(state, "default", 7);

  vTaskDelay(100/portTICK_PERIOD_MS);

  //creating mutex
  mutex = xSemaphoreCreateMutex();//create mutex to handle global variable 

  //creating timers
  YellowWait = xTimerCreate("YELLOW_TIMER", 2000/portTICK_PERIOD_MS, pdFALSE, (void*)0, Yellow_Wait);
  FarmGreen = xTimerCreate("FARMGREEN_TIMER", 10000/portTICK_PERIOD_MS, pdFALSE, (void*)1, Farm_Green);
  HighwayGreen_ = xTimerCreate("HIGHWAYGREEN", 20000/portTICK_PERIOD_MS, pdFALSE, (void*)2, Highway_Green);

  //creating tasks
  xTaskCreatePinnedToCore(State_Machine, "STATEMACHINE", 2048, NULL, 2, &StateMachine, app_cpu);
  xTaskCreatePinnedToCore(HighWay, "HIGWAY", 2048, NULL, 1, &highway, app_cpu);
  xTaskCreatePinnedToCore(FarmRoad, "FARMROAD", 2048, NULL, 1, &farmroad, app_cpu);
  xTaskCreatePinnedToCore(Web_Logger, "WEBSITE", 2048, NULL, 2, &WebLogger, pro_cpu);
  xTaskCreatePinnedToCore(Traffic_Status, "TRAFFIC", 2048, NULL, 1, &TrafficStatus, pro_cpu);

  //starting the task scheduler
  //vTaskStartScheduler(); not necessary for ESP32 FreeRTOS as per SMP changes
}

void loop() 
{
  
}