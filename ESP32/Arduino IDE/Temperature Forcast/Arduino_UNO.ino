//Arduino UNO code to read values from KY001 temprature sensor and transmit values upon request by I2C protocl
//I2C slave - temperature forcast
#include<OneWire.h>
#include<DallasTemperature.h>
#include<Wire.h>

#define ONE_WIRE_BUS 2

float temp = 0;
char temp_string[7];
OneWire onewire(ONE_WIRE_BUS); // creating one wire class object and passing argument during instantiation

DallasTemperature sensor(&onewire);// passing the onewire object address as argument to DT class

void setup() {
  Wire.begin(1);
  Wire.onRequest(onreq);
  Serial.begin(115200);
  Serial.println("Dallas temp IC control library demo:");
  sensor.begin();
}

void loop() {
  Serial.print("Requesting temperatures");
  sensor.requestTemperatures();//command to get temps
  Serial.println("");
  temp = sensor.getTempCByIndex(0);
  dtostrf(temp, 7, 2, temp_string);//converting to string to send by i2c
  Serial.print("Temperature: ");
  Serial.println(temp); // we can have more than 1 ic connected to the same bus
  //so with 0 we can access the first sensor
  delay(2000);
}

void onreq()
{
  Wire.write(temp_string, sizeof(temp_string));//sending the temp_string to master on request
}
