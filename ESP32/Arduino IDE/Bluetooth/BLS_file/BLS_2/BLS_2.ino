#include "SPIFFS.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! 
#endif

//object creation (class instantiation)
BluetoothSerial SerialBT;

void setup() 
{
  //Starting the Serial monitor
  Serial.begin(115200);
  while(!Serial) {}

  //starting SPIFFS
  Serial.println("Starting SPIFFS");
  SPIFFS.begin(true);

  //starting BluetoothSerial 
  Serial.println("Starting Bluetooth Serial");
  SerialBT.begin("ESP32");
  Serial.println("Bluetooth initiated, visible to other devices as ESP32");  
}

void loop() 
{
  //creating and initializing response buffer and input command buffer
  char resp_buffer[100];
  char cmd_buffer[100];
  memset(resp_buffer, 0, sizeof(resp_buffer));
  memset(cmd_buffer, 0, sizeof(cmd_buffer));
  
  //iterative variables
  int iter1 = 0, iter2 = 0;

  //variables
  bool flag = false;
  bool bls_read_flag = false;
  bool bls_write_flag = false;

  //if serial data available to read
  while(SerialBT.available())
  {
    //reading from serial buffer to cmd_buffer
    cmd_buffer[iter1] = SerialBT.read();
    iter1++;
    bls_read_flag = true;
  }
  cmd_buffer[iter1] = '\0';
  
  //processing the commands
  if(bls_read_flag == true)
  {
    /*
      file is the file pointer to read from SPIFFS file
      file1 is the file pointer to write to SPIFSF file 
    */
    if(memcmp(cmd_buffer, "file", 4) == 0)//if command is "file"
    {
      Serial.println("File content requested by BLS client");
      
      //opening SPIFFS file in read mode
      File file = SPIFFS.open("/file.txt");
      if(!file)
      {
        Serial.println("SPIFFS file could not be opened");
        return;
      }
      
      Serial.println("Sending the file content...");
      while(file.available())
      {
        flag = true;
        SerialBT.write(file.read());//reading from file and writing to BLS clients
      }
      if(flag == true)
      {
        Serial.println("File contents successfully read and sent");
      }
      else
      {
        Serial.println("Error while reading the file contents");
      }
      
      //closing the opened file from SPIFFS
      file.close();
    }
    else if(memcmp(cmd_buffer, "write", 5) == 0)//command is "write"
    {
      Serial.println("Write command issued by bls client");

      //Asking BLS client to enter the information to be stored into file
      memcpy(resp_buffer, "pls enter the info", 18);
      resp_buffer[18] = '\n';
      resp_buffer[19] = '\0';

      for(int i = 0; i <= 18; i++)
      {
        SerialBT.write(resp_buffer[i]);
      }
      
      //openning the file in write mode
      File file1 = SPIFFS.open("/file.txt", FILE_WRITE);
      if(!file1)
      {
        Serial.println("SPIFFS file could not be opened");
        return;
      }

      Serial.println("performing the write operation..");
      //Performing the write operation
      while(1)
      {
        //writing from BLS client to file 
        while(SerialBT.available())
        {
          file1.write(SerialBT.read());
          bls_write_flag = true;
        }

        //exit only if flag is set
        if(bls_write_flag == true)
        {
          break;
        }
      }

      //write operation completed & indicating the client
      Serial.println("write operation completed");
      memset(resp_buffer, 0, sizeof(resp_buffer));
      memcpy(resp_buffer, "write operation competed", 24);
      resp_buffer[24] = '\n';
      resp_buffer[25] = '\0';

      for(int i = 0; i <= 24; i++)
      {
        SerialBT.write(resp_buffer[i]);
      }
      
      //closing the file 
      file1.close();
    }
    else//all other commands
    {
      //process all other commands from BLS buffer (stored in cmd_buffer array)
      Serial.printf("Received Command: %s\n", cmd_buffer);
      if(memcmp(cmd_buffer, "thanks", 6) == 0)
      {
        //loading the response buffer 
        memcpy(resp_buffer, "NP! :D", 6);
        resp_buffer[6] = '\n';//newline
        resp_buffer[7] = '\0';//end of string 

        //sending the response to BLS client
        for(int j=0;j<=6;j++)
        {
          SerialBT.write(resp_buffer[j]);//we have to send byte by byte (uint8_t or char) 
        }
        
      }
    }
    Serial.println();
  }
  
  //short delay
  vTaskDelay(100/portTICK_PERIOD_MS);
}
