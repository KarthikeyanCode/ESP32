#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define ProCpu 0
#define AppCpu 1

String URL = "https://httpbin.org/get";
const char* ssid = "Aravind";
const char* pass = "aravind@2006";
bool flag=true;

static TaskHandle_t Client_TaskHandle;
static TimerHandle_t PeriodTimer_Handle;

const char* headerKeys[] = {"date", "server", "content-length", "content-type"}; //keys associated with each header - pointer to an array 
const size_t numberOfHeaders = 4; //number of headers 

void PeriodCallBack(TimerHandle_t TimerHandle)
{
  flag = false; //set the while loop flag 
}

void ClientFunc(void* Parameters)
{
  String Response;
  int StatusCode;
  while(1)
  {
    Response = "";
    HTTPClient Client1;
    Client1.begin(URL);
    Client1.collectHeaders(headerKeys, numberOfHeaders); //information to collect from the http response  
    StatusCode = Client1.GET();
    Serial.printf("Status Code from the http request: %d\n\n", StatusCode);
    if(StatusCode > 0)
    {
      //printing out the response headers
      Serial.println("Response Headers:");
      Serial.println(Client1.header((size_t)0));
      Serial.println(Client1.header((size_t)1));
      Serial.println(Client1.header((size_t)2));
      Serial.println(Client1.header((size_t)3));

      Serial.println();

      //Printing the response body
      Response = Client1.getString();
      Serial.println("Response Body:");
      Serial.println(Response);
    }
    else
    {
      Serial.println("Error");
    }
    Client1.end();
    //const int Capacity = 1*JSON_OBJECT_SIZE(4) + 1*JSON_OBJECT_SIZE(4) + 1*JSON_OBJECT_SIZE(0); //Computing the size of JSON document
    const int Capacity = 1000;
    StaticJsonDocument<Capacity> doc;
    DeserializationError Err = deserializeJson(doc, Response);
    if(Err)
    {
      Serial.println("deserializeJson failed with code");
      Serial.println(Err.c_str());
    }
    else //deserialization done
    {
      const char* url = doc["url"];
      const char* origin = doc["origin"];
      Serial.println(origin);
      Serial.println(url);
    }
    doc.clear(); //clear the json document
    xTimerStart(PeriodTimer_Handle, portMAX_DELAY); //start the timer 
    while(flag){vTaskDelay(1000/portTICK_PERIOD_MS);}
    flag = true;
  }
}

void setup()
{
  //while(1){}
  
  Serial.begin(115200);
  while(!Serial){}

  WiFi.begin(ssid, pass);
  Serial.printf("Connecting to %s...", ssid);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
  Serial.println();
  Serial.println("Connected");

  xTaskCreatePinnedToCore(ClientFunc, "HTTP CLIENT", 7000, NULL, 1, &Client_TaskHandle, ProCpu); //HTTP Client Task
  PeriodTimer_Handle = xTimerCreate("Period Timer", 10000/portTICK_PERIOD_MS, pdFALSE, (void*)0, PeriodCallBack); //Periodic Timer Init

  vTaskDelete(NULL);//Delete Main Task - Setup and Loop Funcs
}

void loop()
{
   
}
