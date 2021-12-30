//ESP32 I2C master
//I2C master code to get temps from arduino 
#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

String temp_string;
float temp;

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
}

void loop() {
  temp_string = "";//reinitialize string each time getting temp value 
  Wire.requestFrom(1, 7);
  while(Wire.available())
  {
    char t = Wire.read();
    temp_string += t;
  }
  temp = temp_string.toFloat();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(temp);
  Serial.println(temp);
  delay(5000);
}
