//RTDB implementation using ESP32 and Firebase
//header files
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "SPIFFS.h"

#define ssid "Enter SSID here"
#define pass "Enter Pass here"
#define api_key_ "Project API Key"
#define user_email "enter the user email for auth"
#define user_pass "user passowrd for auth"
#define database_url_ "RTDB url"

//firebase objects
FirebaseData fbdo;
FirebaseAuth fbauth;
FirebaseConfig fbconfig;

//global variables
String uid;//user id
String database_path;
String data_path;
float sample_data = 25.69;
unsigned long int prev_sent_millis = 0;
unsigned long int time_delay = 20000;//20 seconds

void send_float_data(String path, float _data)
{
   if(Firebase.RTDB.setFloat(&fbdo, path.c_str(), _data))//writing the float data and checking if the operation was successful
   {
    Serial.print("Writing value: ");
    Serial.print (_data);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void setup() 
{
  //connecting to serial monitor
  Serial.begin(115200);
  while(!Serial){}

  //start SPIFFS
  SPIFFS.begin(true);

  //initializing WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
  Serial.printf("Connected to %s\n", ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //assigning the required fields using firebase objects
  //assigning the api key
  fbconfig.api_key = api_key_;

  //assigning the user sign in credentials
  fbauth.user.email = user_email;
  fbauth.user.password = user_pass; 

  //assign the RTDB url
  fbconfig.database_url = database_url_;

  //assign callback function for token generation task
  fbconfig.token_status_callback = tokenStatusCallback;//inbuilt function of addons/TokenHelper.h

  //assign max retry of token generation
  fbconfig.max_token_generation_retry = 5;

  //initialize auth and config
  Firebase.begin(&fbconfig, &fbauth);

  //it will then fetch the uid .. this might take some time
  Serial.print("Fetching the UID ..");
  while((fbauth.token.uid) == "")
  {
    Serial.print(".");
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
  Serial.println("fetched uid");
  uid = fbauth.token.uid.c_str();
  Serial.print("UID: ");
  Serial.println(uid);
  Serial.println();

  //assigning database_path with the obtained uid
  database_path = "/UsersData/" + uid;//concatenation

  data_path = database_path + "/data";
  
}

void loop() 
{
  if(Firebase.ready() && (millis() - prev_sent_millis > time_delay || prev_sent_millis==0))
  {
    prev_sent_millis = millis();

    //send the float data
    send_float_data(data_path, sample_data);
  }
  vTaskDelay(500/portTICK_PERIOD_MS);
}
