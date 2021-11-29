//LCD backlight turns on only for a period of time during which the text entered in serial monitor 
//will be displayed 
//Thus saving the power required to run the LCD
#if CCONFIG_FREERTOS_UNICRE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#include <LiquidCrystal_I2C.h>

static TimerHandle_t timer;

static TaskHandle_t task_1;

LiquidCrystal_I2C lcd(0x27, 16, 2);

//call back function
void timerCallBack(TimerHandle_t xTimer)
{
  Serial.println("Power Saving - Dimming Backlights");
  vTaskResume(task_1);
}

//function for task1 - task working for lcd functioning and  power saving
void task1(void *parameters)
{
  String text;
  byte flag;
  while(1)
  {
    text = "";
    flag = 0;
    while(Serial.available())
    {
      char t = Serial.read();
      text += t;
      flag = 1;
    }
    if(flag == 1)
    {
      if(text.length() > 16)
      {
        lcd.backlight();//turn on backlight
        lcd.setCursor(0,0);//set cursor at 0th column and 0th row
        lcd.print(text.substring(0, 15));
        lcd.setCursor(0,1);
        lcd.print(text.substring(16, text.length()-1));
        xTimerStart(timer, portMAX_DELAY); 
      }
      else
      {
        lcd.backlight();//turn on backlight
        lcd.setCursor(0,0);//set cursor at 0th column and 0th row
        lcd.print(text.substring(0, text.length()-1));
        xTimerStart(timer, portMAX_DELAY);
      }
      Serial.println("Text has been displayed in LCD");
      vTaskSuspend(NULL);
      lcd.clear();
    }
    lcd.noBacklight();
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void setup() 
{
 Serial.begin(115200);
 lcd.init();
 timer = xTimerCreate("LCD_PS", 5000/portTICK_PERIOD_MS, pdFALSE, (void*)0, timerCallBack);//initializing the timer

 if(timer == NULL)
 {
  Serial.println("Timer could not be created");
 }

 xTaskCreatePinnedToCore(task1, "TASK1", 60000, NULL, 1, &task_1, app_cpu);

 Serial.println("-------------------------------------------------------");
 Serial.println("---------------Welcome to LCD power save --------------");
 Serial.println("-----Enter a maximum of 32 characters at a time--------");
 Serial.println("-------------------------------------------------------");

 vTaskDelete(NULL);
}

void loop() 
{
  
}
