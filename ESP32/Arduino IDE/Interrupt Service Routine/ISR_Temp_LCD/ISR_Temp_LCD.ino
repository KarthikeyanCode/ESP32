//monitors ultrasonic sensor for door and interrupts with touch sensor to display temperature 

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#include <LiquidCrystal_I2C.h>
#include<DallasTemperature.h>
#include<OneWire.h>

//one wire bus
#define ONE_WIRE_BUS 4

//RGB LED
#define led_red 18
#define led_blue 5
#define led_green 19

LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire onewire(ONE_WIRE_BUS);
DallasTemperature sensor(&onewire);

//task handler(s)
static TaskHandle_t task_1;

struct touch_sensor
{
  const uint8_t pin = 15;
  bool touch = false;
}s1;

void IRAM_ATTR touch_temp_isr()
{
  s1.touch = true;
}

//task1 function
void task1(void *parameters)
{
  float temp;
  while(1)
  {
    if(s1.touch == true)
    {
       digitalWrite(led_red, HIGH);
       digitalWrite(led_blue, LOW);
       digitalWrite(led_green, LOW);
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("Tmp: ");
       lcd.print(temp);
       lcd.setCursor(0,1);
       lcd.print("Red LED");
       vTaskDelay(2000/portTICK_PERIOD_MS);
       s1.touch = false;
    }
    sensor.requestTemperatures();
    temp = sensor.getTempCByIndex(0);
    Serial.println(temp);
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Green LED");
    digitalWrite(led_red, LOW);
    digitalWrite(led_blue, LOW);
    digitalWrite(led_green, HIGH);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void setup() 
{
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  pinMode(led_red, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(s1.pin, INPUT);
  sensor.begin();
  attachInterrupt(digitalPinToInterrupt(s1.pin), touch_temp_isr, FALLING);
  xTaskCreatePinnedToCore(task1, "TASK1", 3000, NULL, 1, &task_1, app_cpu);
  vTaskDelete(NULL);
}

void loop()
{
  
}
