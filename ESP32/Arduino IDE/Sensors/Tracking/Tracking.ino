//interfacing tracking sensor module KY-033

#include<LiquidCrystal_I2C.h>

#define sensor 15
#define buzzer 4

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() 
{
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(sensor, INPUT);
  pinMode(buzzer, OUTPUT);
}

void loop() 
{
  bool sense = digitalRead(sensor);
  Serial.println(sense);
  lcd.clear();
  if(!sense)
  {
    digitalWrite(buzzer, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Actiavted");
  }
  else
  {
    digitalWrite(buzzer, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Deactivated");
  }
  vTaskDelay(1000/portTICK_PERIOD_MS);
}
