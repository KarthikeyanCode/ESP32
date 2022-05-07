// ESP32 Code for the surveillance module of spy camera
// This module monitors the area and sends request to capture module
// Capture module then responds and captures the image

//Libraries
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
//#include <Bridge.h>

//Preprocessor Directives
#define ProCpu 0 //1st Core
#define AppCpu 1 //2nd Core
#define TrigPin 14
#define EchoPin 27

//Threshold distance
#define Lower_Limit 10
#define Upper_Limit 80

//SMD RGB and Buzzer
#define Red 33
#define Green 25
#define Blue 32
#define Buzzer 26

//Class Instantiations
LiquidCrystal_I2C Lcd(0x27, 16, 2);

//Global Variables
int distance=0;
unsigned int PrevTime;

//WiFi 
const char* ssid = "ssir";
const char* pass = "pass";

//Http Client
const char* ServerPath = "Camera Module Server Path";
unsigned int ResponseCode;
String Response;

//Task Handlers
static TaskHandle_t WiFiHandler;
static TaskHandle_t LcdTaskHandler;
static TaskHandle_t SensorHandler;

//Functions
void WiFiFunc(void* parameters)
{
  /*
  while(1)
  {
    vTaskDelay(100/portTICK_PERIOD_MS);
  }*/
  
  WiFiConnect:

  //Bridge.begin();

  //Connecting to WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ");
  
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
  Serial.println();
  Serial.println("Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  while(1)
  {
    if(WiFi.status() == WL_CONNECTED) //if WiFi is connected send HTTP Post 
    {
      if((distance>=Lower_Limit) && (distance <= Upper_Limit) && (millis()-PrevTime>25000))
      {
        
        // Initialize the client library
        
        HTTPClient client;
      
        // Make a HTTP request:
        client.begin(ServerPath);
      
        // if there are incoming bytes available
        // from the server, read them and print them:

        ResponseCode = client.GET();
        Response = "";

        if(ResponseCode)
        {
          Response = client.getString();
          Serial.println(ResponseCode);
          Serial.println(Response);
        }
        else
        {
          Serial.println("Error");
        }
       
        client.end();
         
        Serial.println("GET request Sent");

        PrevTime = millis();
        
      }
      else
      {
        //Pass
      }
    }
    else //else try to connect to wifi
    {
      goto WiFiConnect;
    }
    
    vTaskDelay(3000/portTICK_PERIOD_MS); //Check every 2 seconds to send requests as queue should not overflow which will lead to loss of packets 
  }
}

void LcdTaskFunc(void* parameters) //LcdTask Function
{
  int BufferLength = 0;
  while(1)
  {
    
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("Distance: ");
    Lcd.setCursor(10, 0);
    Lcd.print(distance);

    if(distance >= 100)
    {
      Lcd.setCursor(14,0);
      Lcd.print("cm");
    }
    else 
    {
      Lcd.setCursor(13,0);
      Lcd.print("cm");
    }
    
    
    
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void SensorTaskFunc(void* parameters)
{

  int duration;
  
  while(1)
  {
     
    digitalWrite(TrigPin, LOW);
    delayMicroseconds(2);
    
    digitalWrite(TrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(TrigPin, LOW);
    
    duration = pulseIn(EchoPin, HIGH);
    
    // Calculating the distance
    distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
    
    // Displays the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if(distance>Lower_Limit && distance<Upper_Limit)
    {
      digitalWrite(Green, LOW);
      digitalWrite(Red, HIGH);
      digitalWrite(Buzzer, HIGH);
      vTaskDelay(500/portTICK_PERIOD_MS);
      digitalWrite(Buzzer, LOW);
      
    }
    else
    {
      digitalWrite(Red, LOW);
      digitalWrite(Green, HIGH);
      digitalWrite(Buzzer, LOW);
    }
    
    vTaskDelay(800/portTICK_PERIOD_MS);
  }
}

void setup() 
{
  //Initialize Serial Monitor
  Serial.begin(115200);
  while(!Serial){}

  vTaskDelay(100/portTICK_PERIOD_MS); //small delay

  //while(1){}

  //GPIO
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);
  pinMode(Buzzer, OUTPUT);

  digitalWrite(Blue, HIGH);

  //Initialize Lcd
  Lcd.init();
  Lcd.backlight();

  //Create Tasks
  xTaskCreatePinnedToCore(LcdTaskFunc, "LCD", 3000, NULL, 1, &LcdTaskHandler, AppCpu);
  xTaskCreatePinnedToCore(SensorTaskFunc, "Sensor", 3000, NULL, 2, &SensorHandler, AppCpu);
  xTaskCreatePinnedToCore(WiFiFunc, "WiFi", 6000, NULL, 1, &WiFiHandler, ProCpu);

  digitalWrite(Blue, LOW);

  vTaskDelete(NULL); //Delete Setup and Main Task
}

void loop() 
{
  //Executing wont reach here
}
