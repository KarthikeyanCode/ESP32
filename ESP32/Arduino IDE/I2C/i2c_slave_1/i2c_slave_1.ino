#include<Wire.h>
#include<WiFi.h>

const char* ssid = "Aravind"
const char* password = "aravind@2006"
String temp = "";
String header;//to store the http request

WiFi server(80); //initializing class with object and argument 80 (port)

//time out mechanism 
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;//setting the time out time as 2 seconds

void setup() {
  Wire.begin(1);
  Serial.begin(9600);
  Wire.onReceive(onRec);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)//wifi.status() returns WL_CONNECTED when connected to a wifi network
  {
    delay(500);
    Serial.print(".");
  }
  //once connected
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("print: ");
  Serial.print(WiFi.localIP());//printing the IP address of the connected WiFi
  server.begin();//starting the server at port 80
}

void loop() 
{
   WiFiClient client = server.available();
   if(client)
   String currentLine = "";
   currentTime = millis();
   previousTime = currentTime;
   {
      while(client.connected() && (currentTime-previousTime<=timeoutTime))//when the cient is connected
      {
        currentTime = millis();
        if(client.available()) //if there are bytes sent by client
        {
          char c = client.read();
          header += c;

          if(c == '\n')
          {
            if(currentLine.length() == 0)//getting two newline characters in a row, this marks the end of HTTP request
            //since we received the http request we can send OK HTTP status response code 
            //now we send HTTP/1.1 200 OK as response and then send information regarding content type 
            //then a newline to end the information 
            {
              client.println("HTTP/1.1 200 OK");
              client.println("Conent-type:text/html");
              client.println("Connection: close");
              client.println();

              //sending the client information in html as the server is HTTP (hyper text transfer)
              //bg color, style, link, head ....
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, inital-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".button2 {background-color: #555555;}</style></head>");

              //body of html, webpage heading
              client.println("<body><h1>ESP32 Web Server</h1>");
              
              
            }
          }
        }
      }
   }
}

void onRec(int bytes)
{
  temp = "";
  while(Wire.available())
  {
    char c = Wire.read()
    temp += c;
  }
  temp.toFloat();
  Serial.println(temp);
}
