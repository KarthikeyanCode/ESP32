#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "WiFi.h"

#define LED_pin 2

const char* ssid = "Enter your ssid here";
const char* pass = "Enter your pass here";

AsyncWebServer server(80);

void setup() 
{
  Serial.begin(115200);
  pinMode(LED_pin, OUTPUT);
  SPIFFS.begin(true);

  //connecting to WiFi 
  WiFi.begin(ssid, pass);
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

  //Specifying API requests (HTTP_GET requests) and starting the server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/test.html", "text/html", false);
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/sample.jpg", "text", true);
  });
  
  server.begin();
  Serial.println("Server started at port 80");
}

void loop() 
{
  while(1)
  {
    //IDLE
  }
}
