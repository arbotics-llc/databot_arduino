#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

MPL3115A2 barometer;

// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);

  Serial.begin(9600);

  setupMPL3115A2(barometer, PRESSURE);

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {

  float pressure = barometer.readPressure();
  float altitude = 44330.77 * (1-pow((pressure/101326), 0.1902632));

  Serial.println(altitude);

  //wait for serial to finish and then sleep for one second every loop
  Serial.flush();
  delay(1000);
}
