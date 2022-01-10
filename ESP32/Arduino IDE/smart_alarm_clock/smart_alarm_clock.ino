//ESP32 IoT based smart clock  
//header files
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "ArduinoJson.h"

//ESP32 CPU Cores
#define pro_cpu 0
#define app_cpu 1

//default pins
#define buzzer 15
#define dismiss_button 2
#define snooze_button 4

//class instantiaion
WiFiUDP ntp_udp;
NTPClient ntp_client(ntp_udp);
LiquidCrystal_I2C lcd(0x27, 16, 2);
AsyncWebServer server(80);//pnning the async web server to port 80

//timer specifics
static const uint16_t timer_divider = 80;//timer frequency for all timers
//static const uint64_t sync_timer_max_count = 10000000;//sync timer max count
static const uint64_t snooze_timer_def_max_count = 60000000;//alarm timer max count for snooze
static uint64_t alarm_timer_exact_count;//the actual timer alarm count 

//task Hanlders
static TaskHandle_t clock_task_handler;//main clock task 
static TaskHandle_t alarm_task_handler;//alarm task
static TaskHandle_t web_server_task_handler;

//timer handlers
static hw_timer_t *sync_timer_handler = NULL;
static hw_timer_t *snooze_timer_handler = NULL;

//global flags
uint8_t sync_flag = false;//for sync of ntp client to obtain present time 
uint8_t alarm_set_flag = false;//used to see if alarm is set or not 
uint8_t alarm_dismiss_flag = false;//used to dismiss alarm
uint8_t alarm_snooze_flag = false;//used to snooze alarm
uint8_t snooze_loop = false;//flag used for logical boundary conditions

//global variables
const char* ssid = "wifi ssid";
const char* pass = "wifi pass";
const char* alarm_time = NULL;
char present_time[6];//stores hours and minutes of present time
String _date, _time, _date_formatted;

void clock_task_function(void* parameters)
{
  while(1)
  {
    while(!ntp_client.update())
    {
      ntp_client.forceUpdate();
    }
    _date_formatted = ntp_client.getFormattedDate();
    int split = _date_formatted.indexOf('T');
    _date = _date_formatted.substring(0, split);
    _time = _date_formatted.substring(split+1, _date_formatted.length()-1);
    _time.substring(0, 5).toCharArray(present_time, 6);
    //Serial.println(present_time);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(_time);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void alarm_task_function(void* parameters)
{
  while(1)
  {
    if(alarm_set_flag == true)
    {
      if(memcmp(present_time, alarm_time, 5)==0)
      {
        while(1)
        {
          for(int i=0; i<3; i++)
          {
            digitalWrite(buzzer, HIGH);
            vTaskDelay(300/portTICK_PERIOD_MS);
            digitalWrite(buzzer, LOW);
            vTaskDelay(300/portTICK_PERIOD_MS);
          }
          if(alarm_dismiss_flag == true)
          {
            alarm_set_flag = false;
            alarm_dismiss_flag = false;
            Serial.println("Alarm dismissed");
            break;
          }
          if(alarm_snooze_flag == true)
          {
            timerAlarmEnable(snooze_timer_handler);
            alarm_set_flag = false;
            snooze_loop = true;
            Serial.println("Alarm snoozed");
            break;
          }
          vTaskDelay(2000/portTICK_PERIOD_MS);
        }
      }
    }
    if(snooze_loop == false)
    {
      if((alarm_snooze_flag == true) || (alarm_dismiss_flag == true))//if these flags are turned on without the alarm being set, simply turn them off
      {
        alarm_snooze_flag = false;
        alarm_dismiss_flag = false;
      }
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void web_server_task_function(void* paramters)
{
  //----------------------------------------------------------------------------------------------------------------

  //setting up the async web server
  //setting up the get request and responses 
  
  Serial.println("Setting up the HTTP server");
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("got get request for html");
    request->send(SPIFFS, "/index.html", "text/html", false);
  });

  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("got get request for js");
    request->send(SPIFFS, "/index.js", "text/javascript", false);
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("got get request for css");
    request->send(SPIFFS, "/index.css", "text/css", false);
  });

  /*
  //refer link: https://github.com/jeremypoulter/SmartPlug/blob/1cb2bc267d1db79975ab84b385887962dff10f05/src/web_ui.cpp#L111:L151
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("Got something");
  });
  */

  //Handling the post request and responses
  /*
  server.on("/index", HTTP_POST, [](AsyncWebServerRequest *request){
    int params = request->params();
    Serial.println(request);
    request->send(200, "text/plain", "post done");
    Serial.printf("Save settings, %d params", params);
    for(int i = 0; i < params; i++) 
    {
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile())
      {
        Serial.printf("_FILE[%s]: %s, size: %u", p->name().c_str(), p->value().c_str(), p->size());
      } 
      else if(p->isPost())
      {
        Serial.printf("_POST[%s]: %s", p->name().c_str(), p->value().c_str());
      } 
      else 
      {
        Serial.printf("_GET[%s]: %s", p->name().c_str(), p->value().c_str());
      }
    }
   
    if(request->hasParam("hello", true))
    {
      Serial.println("hello paramter present in post request");
    }
  });*/

   //Handling the post request and responses
   //body data handling 
   server.on(
    "/index",
    HTTP_POST,
    [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

    //reading and parsin the json object 
    char json_obj[100];
    
    memset(json_obj, 0, sizeof(json_obj));

    for (size_t i = 0; i < len; i++) 
    {
      json_obj[i] += data[i];
    }
    json_obj[len] = '\0';
    Serial.print(json_obj);
    Serial.println();

    //Deserializing and parsing the JSON

    StaticJsonDocument<300> json_doc;
    DeserializationError error = deserializeJson(json_doc, json_obj, len);
    
    if(error)
    {
      Serial.println("Parsing failed");
      Serial.println();
    }
    else
    {
      JsonVariant parse_res_alarm_time = json_doc["time"];
      alarm_time = parse_res_alarm_time.as<char*>();//typecast to char*
      Serial.print("Alarm time: ");
      Serial.println(alarm_time);
      Serial.println();
      alarm_set_flag = true;
    }
    
    request->send(200, "text/plain", "post done");
  });
  
  server.begin();//starting the server
  Serial.println("HTTP server setup completed");

  //----------------------------------------------------------------------------------------------------------------

  vTaskDelete(NULL);
}

//Interrupt Service Routine Functions

void IRAM_ATTR snooze_timer_isr()
{
  if(alarm_snooze_flag == false)
  {
    timerWrite(snooze_timer_handler, 0);
    snooze_loop = false;
    return;
  }
  for(int j=0; j<3; j++)
  {
    digitalWrite(buzzer, HIGH);
    vTaskDelay(300/portTICK_PERIOD_MS);
    digitalWrite(buzzer, LOW);
    vTaskDelay(300/portTICK_PERIOD_MS);
  }
}

//alarm dismiss button ISR
void IRAM_ATTR alarm_dismiss_isr()
{
  alarm_dismiss_flag = true;
  alarm_snooze_flag = false;
  snooze_loop = false;
}

//alarm snooze button ISR
void IRAM_ATTR alarm_snooze_isr()
{
  alarm_snooze_flag = true;
}


void setup() 
{
   //----------------------------------------------------------------------------------------------------------------
  
  //starting the serial monitor 
  Serial.begin(115200);
  while(!Serial) 
  {
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
  vTaskDelay(100/portTICK_PERIOD_MS);
  Serial.println("executing the setup task..");
  Serial.println("('_')7");

  //----------------------------------------------------------------------------------------------------------------

  //setting up the WiFi connection
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(250/portTICK_PERIOD_MS);
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.printf("WiFi: %s\n", ssid); 
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //----------------------------------------------------------------------------------------------------------------
  
  //starting the SPI flash file system
  SPIFFS.begin(true);

  //----------------------------------------------------------------------------------------------------------------
  
  //initialising GPIO pins
  pinMode(buzzer, OUTPUT);
  pinMode(dismiss_button, INPUT_PULLUP);
  pinMode(snooze_button, INPUT_PULLUP);

  //----------------------------------------------------------------------------------------------------------------

  //initialising GPIO interrupts
  attachInterrupt(dismiss_button, alarm_dismiss_isr, RISING);
  attachInterrupt(snooze_button, alarm_snooze_isr, RISING);
  
  //----------------------------------------------------------------------------------------------------------------

  //creating timers 
  //synchronization timer
  /*
  sync_timer_handler = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(sync_timer_handler, &sync_timer_isr_function, true);
  timerAlarmWrite(sync_timer_handler, sync_timer_max_count, true);
  */

  //alarm timer
  snooze_timer_handler = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(snooze_timer_handler, &snooze_timer_isr, true);
  timerAlarmWrite(snooze_timer_handler, snooze_timer_def_max_count, true);

  //----------------------------------------------------------------------------------------------------------------
   
  //create tasks
  xTaskCreatePinnedToCore(clock_task_function, "clock_task", 2048, NULL, 1, &clock_task_handler, app_cpu);//clock task
  xTaskCreatePinnedToCore(alarm_task_function, "alarm_task", 2048, NULL, 1, &alarm_task_handler, app_cpu);//alarm task
  xTaskCreatePinnedToCore(web_server_task_function, "server_task", 6000, NULL, 1, &web_server_task_handler, pro_cpu);//web server tasks

  //----------------------------------------------------------------------------------------------------------------

  //NTPClient initiation
  ntp_client.begin();
  ntp_client.setTimeOffset(19800);

  //----------------------------------------------------------------------------------------------------------------

  vTaskDelay(100/portTICK_PERIOD_MS);

  //----------------------------------------------------------------------------------------------------------------
  
  //lcd initiation
  lcd.init();
  lcd.backlight();

  //----------------------------------------------------------------------------------------------------------------

  //finally enabling the timers
  //timerAlarmEnable(sync_timer_handler);
  //timerAlarmEnable(alarm_timer_handler);

  //----------------------------------------------------------------------------------------------------------------

  vTaskDelete(NULL);//delete main task     
}

void loop() {}
