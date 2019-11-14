#include <Adafruit_MPL3115A2.h>


Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

float pressure = -1;

void setup() {
  Serial.begin(9600);
  baro.begin();
}

void loop() {
  Serial.println("test");
  if(baro.getPressure(&pressure) == READ_SUCCESS) {
    Serial.println(pressure);
  }
}
