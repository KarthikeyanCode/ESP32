//ESP32 program - temprature forcast 
//collect tempratures from arduino board which is connected to sensor and display it in website

#include<Wire.h>
#include<WiFi.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

//WiFi ssid and password
const char* ssid = "Enter your ssid here";
const char* password = "Enter your password here";
String header; //header to store the request from the client
String temp_string; //temp string to store the string value of temprature sent through I2C
float temp;//actual float value of temp to be displayed in the website
//pointer to html code
const char* code1 = "<!DOCTYPE html>\n"
                   "<html>\n"
                   "<head>\n"
                   "<style>\n"
                   ".button\n"
                   "{\n"
                   "  border: none;\n"
                   "  color: white;\n"
                   "  padding: 15px 32px;\n"
                   "  text-align: center;\n"
                   "  display: inline-block;\n"
                   "  font-size: 16px;\n"
                   "  margin: 4px 2px;\n"
                   "  cursor: pointer;\n"
                   "  background-color: #4CAF50;\n"
                   "}\n"
                   "</style>\n"
                   "</head>\n"
                   "<body>\n"
                   "<p>Temperature in the bedroom:</p>\n"
                   "<ul>\n"
                   "<li>";

const char* code2 = "</li>\n";

const char* refresh = "<a href = \"/temp\"><button class \"button\">Refresh</button></a>\n";

const char* code3 = "</body>\n"
                     "</html>\n";

WiFiServer server(80); //port to host the http server


void setup() {
  pinMode(2, OUTPUT);

  Serial.begin(115200);

  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  
  //WiFi http server initialization
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
  server.begin();
  
}

void loop() {
  WiFiClient client = server.available(); //listening for client connection
  Wire.requestFrom(1, 7);
  temp_string = "";
  while(Wire.available())
  {
    char t = Wire.read();
    temp_string += t;
  }
  temp = temp_string.toFloat();
  lcd.clear();
  lcd.setCursor(0,0);//0 column and 0 low
  lcd.print(temp);
  if(client)
  {
    //blink LED to represent client connection 
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    Serial.println("New Client");
    String currentLine = ""; //making string to store the data from the client which is of one line
    while(client.connected()) //as long as client is connected and there is no timeout
    {
      if(client.available()) //bytes available to be read from client 
      {
        char c = client.read();
        Serial.write(c);
        header += c;
        if(c == '\n')
        {
          if(currentLine.length() == 0)//if two current lines are received then it marks the end of HTTP request (2 \n in a row)
          //therefore send the html data to the client 
          {
            //HTTP headers start with response code, content type and ends with blank line (2 \n in a row)
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: keep-alive");
            client.println();
            
            //html code for the front end of the web server
            //HTTP response will be a html code and ends with again blank line (2 \n in a row)
            if(header.indexOf("GET /temp") >= 0)//if http header(request) has "GET /ON" string in it
            {
              client.print(code1);
              client.print(temp);
              client.print(code2);
              client.print(refresh);
              client.print(code3);//ends with \n
              client.println();//another \n to end the http response 
              
            }
            else
            {
              client.print(code1);
              client.print(temp);
              client.print(code2);
              client.print(refresh);
              client.print(code3);//ends with \n
              client.println();//another \n to end the http response
              //Serial.println("LED is OFF");
              //lcd.clear();//clearing the lcd
              //lcd.setCursor(0,0);//setting cursor
              //lcd.print("LED is OFF");//printing to the LCD
            }
            break;
          }
          else//if new line is received we need to clear currentLine
          {
            currentLine = "";
          }
        }
        else if(c != '\r')
        {
          currentLine += c; 
        }
      }
    }
    //Serial.println(header);
    header = "";//the total http request is concatenated into header and for new request header is reinitialized to 0
    client.stop();
    Serial.println("Client disconnected");
    Serial.println("");
  }
  delay(1000);
}
