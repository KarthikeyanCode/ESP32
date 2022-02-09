//Firebase Storage 
//Header Files
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "SPIFFS.h"

//Preprocessor Directives
#define ssid "Enter SSID here"
#define pass "Enter Pass here"
#define api_key_ "Project API Key"
#define user_email "enter the user email for auth"
#define user_pass "user passowrd for auth"
#define StorageBucketId "The storage URL given in storage > Files section" //Storage Bucket ID
#define ImagePath "image_1.jpg" //SPIFFS Image Path
#define BAUD 115200

//Firebase Objects
FirebaseData FbDO;
FirebaseAuth FbAuth;
FirebaseConfig FbConfig;

//Global Variables
bool TaskDone = false;

void SerialInit()
{
  Serial.begin(BAUD);
  while(!Serial){}
  Serial.printf("Connected to serial monitor with baud: %d\n", BAUD);
  Serial.println();
}

void WiFiInit()
{
  WiFi.begin(WiFiSSID, WiFiPass);
  while(WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(500/portTICK_PERIOD_MS);
    Serial.print(".");
  }
  Serial.printf("Connected to %s\n", WiFiSSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void SPIFFSInit()
{
  if(!SPIFFS.begin(true))
  {
    Serial.println("Error in initializing SPIFFS");
    Serial.println("Restarting..");
    ESP.restart();
  }
  else
  {
    Serial.println("SPIFFS initialized..");
  }
  Serial.println();
}

void FirebaseInit()
{
  //Setting the configuration and authentication objects
  FbConfig.api_key = ApiKey;
  FbAuth.user.email = UserEmail;
  FbAuth.user.password = UserPass;
  FbConfig.token_status_callback = tokenStatusCallback;
  FbConfig.max_token_generation_retry = 5;

  //Start command
  Firebase.begin(&FbConfig, &FbAuth);
  Firebase.reconnectWiFi(true);
}

void SendImage()
{
  if(Firebase.Storage.upload(&FbDO, StorageBucketId, ImagePath, mem_storage_type_flash, ImagePath, "image/jpeg"))
  {
    Serial.printf("Image Download URL: %s\n", FbDO.downloadURL().c_str());
    TaskDone = true;
  }
  else
  {
    TaskDone = false;
    Serial.println(FbDO.errorReason());
  }
  Serial.println();
}

void setup() 
{
  SerialInit();
  WiFiInit();//Initialize WiFi
  SPIFFSInit();//Initialize SPIFFS
  FirebaseInit();//Initialize Firebase Objects and start connection
}

void loop() 
{
  if(Firebase.ready())
  {
    Serial.println("Uploading image .. ");
    SendImage();
  }
  if(TaskDone)
  {
    Serial.println("Task Successfully Completed..");
    Serial.println();
    while(1){}
  }
  Serial.println("Trying again..");
  vTaskDelay(1000/portTICK_PERIOD_MS);
}
