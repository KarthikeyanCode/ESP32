//code to interface i2c lcd and display text

#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);//address(hex), columns, rows 

void setup(){
  Wire.begin();
  Serial.begin(115200);
  lcd.init();//initialize the lcd
  lcd.backlight();//turn on lcd backlight
}

void loop(){
    lcd.setCursor(0,0);//column, rows
    lcd.print("Hello Everybody");
    lcd.setCursor(0,1);
    lcd.print("Nice Code");
   
}
