#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

//OneWire on pin 4 of the databot
OneWire oneWire(4);
DallasTemperature tempsensor(&oneWire);

// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);

  Serial.begin(9600);

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {

  float temperature = getExternalTemperature(tempsensor);

  Serial.println(temperature);

  //sleep 250 milliseconds second every loop
  Serial.flush();
  LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
}
