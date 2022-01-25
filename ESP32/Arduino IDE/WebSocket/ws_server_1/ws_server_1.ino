#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

//preprocessor directives
#define onboard_led 2

//class instantiations
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//global variables
bool led_state = false;

//WiFi credentials
const char* ssid = "Aravind";
const char* pass = "aravind@2006";

//processor function for server
String processor(const String& var)
{
  if(var == "state")
  {
    if(digitalRead(onboard_led))
    {
      return "on";
    }
    else
    {
      return "off";
    }
  }
  return String();
}

//function to finally handle websocket message from client
void handle_websocket_message(void* args, uint8_t* data, size_t len)
{
  Serial.println("Message Received");
  data[len] = 0;//setting the final character to end-of-string '\0'
  char* _data = (char*)data;
  Serial.printf("data received: %s", _data);
  Serial.println();

  //changing the state upon request
  if(String(_data).indexOf("toggle") != -1)
  {
    led_state = !led_state;
    digitalWrite(onboard_led, led_state);  
  }

  //sending the current state as message through socket to all the clients
  if(digitalRead(onboard_led))
  {
    ws.textAll("high");
  }
  else
  {
    ws.textAll("low");
  }
}

//websocket onEvent function, fallback function for any event related to web socket
void on_event(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{
  switch (type)//switch according to the type of event 
  {
    case WS_EVT_CONNECT://client connected
    {
      Serial.printf("Websocket client\nID: #%u\nIP: %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    }
    case WS_EVT_DISCONNECT://client disconnected 
    {
      Serial.printf("Websocket client disconnected\nID: #%u\n", client->id());
      break;
    }
    case WS_EVT_DATA:
    {
      handle_websocket_message(arg, data, len);//handle in case of message transfer
      break;
    }
    case WS_EVT_PONG://ping
    {
      break;
    }
    case WS_EVT_ERROR://in case of any error
    {
      break;
    }
    default://uncovered case
    {
      
    }
  }
}

//websocket initialization
void websocket_init()
{
  ws.onEvent(on_event);
  server.addHandler(&ws);
}

void setup() 
{
  //initialize serial monitor 
  Serial.begin(115200);
  while(!Serial){}

  //GPIO setup
  pinMode(onboard_led, OUTPUT);

  //initializing SPIFFS
  SPIFFS.begin(true);

  //connecting to WiFi
  WiFi.begin(ssid, pass);
  Serial.printf("connecting to %s ", ssid);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(500);
  }
  Serial.println();
  Serial.printf("Connected to %s\n", ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //setup the websocket
  Serial.println("Setting up the socket and server ..");
  websocket_init();

  //setup HTTP server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send(SPIFFS, "/index.css", "text/css");
  });

  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send(SPIFFS, "/index.js", "text/javascript");
  });
  
  server.begin();

  Serial.println("socket and server setup");
}

void loop() 
{
  ws.cleanupClients();
}
