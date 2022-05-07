// Spy Camera Photo Capture Module 
// This module captures the photo and stores in SD card
// Then it uses flash as a buffer to send the captured photo to databse
// It captures high quality images

#include <WiFi.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <EEPROM.h>            // read and write from flash memory
#include <Firebase_ESP_Client.h>
//#include "Firebase.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "SPIFFS.h"
#include <addons/SDHelper.h>

#define ProCpu 0
#define AppCpu 1

// define the number of bytes you want to access
#define EEPROM_SIZE 1

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//Firebase Credentials 
#define ApiKey "API Key"
#define UserEmail "email"
#define UserPass "pass" 
#define StorageBucketId "storage bucket ID" //Storage Bucket ID

#define ImagePath "/Picture.jpg" //SPIFFS Image Path
#define BAUD 115200

//Firebase Objects
FirebaseData FbDO;
FirebaseAuth FbAuth;
FirebaseConfig FbConfig;

int pictureNumber = 0;
String path;
static TaskHandle_t HttpServer;
static TaskHandle_t Camera;
const char* ssid = "ssid";
const char* pass = "pass";
bool TakePic = false;

//webserver at port 80
AsyncWebServer server(80);

//Function Declaration
void FirebaseInit(void);
void fcsUploadCallback(FCS_UploadStatusInfo info);
void listAllFiles(void);

//Listing SPIFFS files
void listAllFiles(){
 
  File root = SPIFFS.open("/");
 
  File file = root.openNextFile();
 
  while(file){
 
      Serial.print("FILE: ");
      Serial.println(file.name());
 
      file = root.openNextFile();
  }
 
}

//Firebase Initialization Function
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
  //Firebase.reconnectWiFi(true);
}

//The Firebase Storage upload callback function
void fcsUploadCallback(FCS_UploadStatusInfo info)
{
    if (info.status == fb_esp_fcs_upload_status_init)
    {
        Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.fileSize, info.remoteFileName.c_str());
    }
    else if (info.status == fb_esp_fcs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_fcs_upload_status_complete)
    {
        Serial.println("Upload completed\n");
        FileMetaInfo meta = FbDO.metaData();
        Serial.printf("Name: %s\n", meta.name.c_str());
        Serial.printf("Bucket: %s\n", meta.bucket.c_str());
        Serial.printf("contentType: %s\n", meta.contentType.c_str());
        Serial.printf("Size: %d\n", meta.size);
        Serial.printf("Generation: %lu\n", meta.generation);
        Serial.printf("Metageneration: %lu\n", meta.metageneration);
        Serial.printf("ETag: %s\n", meta.etag.c_str());
        Serial.printf("CRC32: %s\n", meta.crc32.c_str());
        Serial.printf("Token: %s\n", meta.downloadTokens.c_str());
        Serial.printf("Download URL: %s\n\n", FbDO.downloadURL().c_str());
    }
    else if (info.status == fb_esp_fcs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

void HttpFunc(void* parameters)
{
  WiFiConnect:

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send(200, "text/plain", "ok");
    TakePic = true;
  });
  
  server.begin();
  
  while(1)
  {
    if(WiFi.status() != WL_CONNECTED)
    {
      goto WiFiConnect;
    }
  
    vTaskDelay(300/portTICK_PERIOD_MS);
  }
}

void CameraFunc(void* parameters)
{

  //Configure camera and psram
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound())
  {
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } 
  else 
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  //Init camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  //Serial.println("Starting SD Card");
  /*
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }*/
  
  SPIFFS.begin();
  EEPROM.begin(EEPROM_SIZE); //EEPROM begin
  SD_MMC.begin();
  fs::FS &fs = SD_MMC; //SD Card file storage object 

  while(WiFi.status() != WL_CONNECTED) { vTaskDelay(100/portTICK_PERIOD_MS); }
  FirebaseInit();
    
  while(1)
  {
      if(TakePic == true)
      {
        //Take pic and save to sd card
        // Take Picture with Camera
        //rtc_gpio_hold_dis(GPIO_NUM_4);
        
        camera_fb_t* fb = NULL;//camera class pointer object 
        fb = esp_camera_fb_get(); //capture and store the related information w.r.t fb object in the camera_fb_t class 
        
        if(!fb) {
          Serial.println("Camera capture failed");
          return;
        }
        // initialize EEPROM with predefined size
        pictureNumber = EEPROM.read(0) + 1; //Read the old picture number from EEPROM 
      
        // Path where new picture will be saved in SD Card - path global variable 
        path = "/picture" + String(pictureNumber) +".jpg";
      
        
        Serial.printf("Picture file name: %s\n", path.c_str());
        
        File file = fs.open(path.c_str(), FILE_WRITE);//if the file is not present it will create one 
        if(!file){
          Serial.println("Failed to open file in writing mode");
        } 
        else {
          file.write(fb->buf, fb->len); // payload (image), payload length (accessing the class variables using pointer object)
          Serial.printf("Saved file to path: %s\n", path.c_str());
          EEPROM.write(0, pictureNumber); //write the new picture number from EEPROM 
          EEPROM.commit(); //Commit
        }
        file.close();
        esp_camera_fb_return(fb); 
        
        // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
        /*
        pinMode(4, OUTPUT);
        digitalWrite(4, LOW);
        rtc_gpio_hold_en(GPIO_NUM_4);
        */
        
        Serial.println("Picture taken and saved succesfully");
        TakePic = false;

        //----------------------------------------------------------------------------------------------------------------------------------------
        /*
        * Upload the image to firebase
        * Bring the image file from SD Card to SPIFFS then upload it to firebase 
        */

        file.close();
        File SourceFile = fs.open(path.c_str(), FILE_READ);
        File DestFile = SPIFFS.open(ImagePath, FILE_WRITE);

        Serial.println("Transfering Files to SPIFFS via Buffer...");
        uint8_t Buff[3000];
        memset(Buff, '\0', 3000); //Initializing the buffer
        while(SourceFile.read(Buff, 3000) && SourceFile.available())
        {
          DestFile.write(Buff, 3000);
        }

        Serial.println("File transfer done!");
  
        SourceFile.close();
        DestFile.close();

        Serial.println("Uploading to Firebase Storage ...");
                        
        while(!Firebase.ready()){}
        if(!Firebase.Storage.upload(&FbDO, StorageBucketId /* Firebase Storage bucket id */, ImagePath /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, path.substring(1, path.length()+1) /* path of remote file stored in the bucket */, "Image/jpg" /* mime type */, fcsUploadCallback /* callback function */))
        {  
          Serial.print("Error in Uploading, Error: ");
          Serial.println(FbDO.errorReason());
        }
        else
        {
          Serial.println("Image Successfully Uploaded to Firebase Storage");
        }
        Serial.print("File in Storage: ");
        Serial.println(path.substring(1, path.length()+1));

        Serial.println("All SPIFFS Files (before removal):");
        listAllFiles();
        
        Serial.println(SPIFFS.remove(ImagePath));
        Serial.println("SPIFFS File removed!");

        Serial.println("All SPIFFS Files: (after removal)");
        listAllFiles();

        Serial.println();        
        //----------------------------------------------------------------------------------------------------------------------------------------
        
      }
      
      vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  //Serial monitor
  Serial.begin(115200);
  vTaskDelay(500/portTICK_PERIOD_MS); 
  
  //Tasks
  xTaskCreatePinnedToCore(HttpFunc, "server", 3000, NULL, 1, &HttpServer, ProCpu); //Server Task 
  xTaskCreatePinnedToCore(CameraFunc, "Camera", 8000, NULL, 1, &Camera, AppCpu); //Camera Task

  vTaskDelete(NULL);
}

void loop() 
{
  
}
