#include<WiFi.h>
#include "SPIFFS.h"


//File pointers (File stream)
File file_html;
File file_header;

//global variables
char* ssid = "wifi ssid";
char* pass = "wifi pass";
char c;
char prev_c = 's';
char header_read;
char html_read;
char header[5000];
char html[5000];
int iter1=0;

//Web server will be hosted at port 80
WiFiServer server(80);

void setup() 
{
  //serial begin
  Serial.begin(115200);

  //Startign SPIFFS
  SPIFFS.begin(true);

  //Connecting to WiFi snd starting the server
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
  Serial.println();
  Serial.print("Connected to WiFi ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server started");
}

void loop() 
{
  WiFiClient client = server.available();
  if(client)
  {
    Serial.println("New Client");
    String client_message = "";
    file_html = SPIFFS.open("/page.html");
    file_header = SPIFFS.open("/header.txt");
    if(client.connected())
    {
      //reading the HTTP request of the client
      while(client.available())
      {
        c = client.read();
        client_message += c;
        if(c=='\n' && prev_c=='\n')//two \n characters marks end of HTTP communication
        {
          break;
        }
        prev_c = c;
      }
      Serial.println(client_message);
      //reading using File stream from both the files and storing into buffer
      while(file_header.available())
      {
        header_read = file_header.read();
        header[iter1] = header_read;
        iter1++;
      }
      header[iter1] = '\0';
      iter1=0;
      while(file_html.available())
      {
        html_read = file_html.read();
        html[iter1] = html_read;
        iter1++;
      }
      html[iter1] = '\0';
      iter1=0;

      //sending the buffer to the client
      client.println(header);
      client.println();
      client.println(html);
      client.println();

      //closing the connection
      //closing the files so that when opened again the file pointers are reset
      client.stop();
      file_header.close();
      file_html.close();

      //clearing the buffer
      memset(header, 0, sizeof(header));
      memset(html, 0, sizeof(html));
    }
  }
  vTaskDelay(500/portTICK_PERIOD_MS);
}
