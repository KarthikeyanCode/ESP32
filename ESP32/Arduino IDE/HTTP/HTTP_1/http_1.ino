//toggle LED via website and represent the status of LED in LCD using I2C
#include<WiFi.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);//adddress, columns, rows

const char* ssid = "Enter your ssid here";
const char* password = "Enter your password here";
String header;
String temp_string;
float temp;
String led_state = "";
//html code to display on the website
const char* code = "<!DOCTYPE html>\n"
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
                   "<body>\n";
                   
//code when LED is in on state
const char* on_code = "<a href=\"/OFF\"><button class = \"button\" onclick = \"alert('Turn the LED off?')\">OFF</button></a>\n";
//code when LED is in off state
const char* off_code = "<a href=\"/ON\"><button class = \"button\" onclick = \"alert('Turn the LED on?')\">ON</button></a>\n";
const char* code1 = "</body>\n"
                    "</html>\n";                   

WiFiServer server(80);

//timeout timers
/*
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;//time out 
*/
void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  lcd.init();//initializing lcd
  lcd.backlight();//turning on back light
  Serial.print("Connecting to: ");
  Serial.println(ssid);
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
  delay(4000);//delay 4 secs
  
  if(client)
  {
    //currentTime = millis();
    //previousTime = currentTime;
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
            client.println("HTTP/1.1 200 OK"); //status code response 
            client.println("Content-type:text/html"); //communication type html or text
            client.println("Connection: Keep Alive"); //connection Keep Alive
            client.println();
            
            //html code for the front end of the web server
            //HTTP response will be a html code and ends with again blank line (2 \n in a row)
            if(header.indexOf("GET /ON") >= 0)//if http header(request) has "GET /ON" string in it
            {
              digitalWrite(2, HIGH);
              client.print(code);
              client.print(on_code);
              client.print(code1);
              client.println();//http response ends with two newlines
              Serial.println("LED is ON");
              lcd.clear();//clearing the lcd
              lcd.setCursor(0,0);//setting cursor
              lcd.print("LED is ON");//printing to the LCD
            }
            else if(header.indexOf("GET /OFF") >= 0)
            {
              digitalWrite(2, LOW);
              client.print(code);
              client.print(off_code);
              client.print(code1);
              client.println();//http response ends with two newlines
              Serial.println("LED is OFF");
              lcd.clear();//clearing the lcd
              lcd.setCursor(0,0);//setting cursor
              lcd.print("LED is OFF");//printing to the LCD
            }
            else
            {
              client.print(code);
              client.print(off_code);
              client.print(code1);
              client.println();//http response ends with two newlines
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
    header = "";
    client.stop();
    Serial.println("Client disconnected");
    Serial.println("");
  }
}
