#include <Wire.h>
#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

SGP30 sgp30;
SHTC3 shtc3;


// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);

  Serial.begin(115200);

  shtc3.begin();
  shtc3.setMode(SHTC3_CMD_CSE_RHF_LPM);
  shtc3.sleep(true);
  shtc3.update();
  sgp30.begin();
  sgp30.initAirQuality();
  sgp30.setHumidity(doubleToFixedPoint(RHtoAbsolute(shtc3.toPercent(),shtc3.toDegC())));
  
  Serial.flush();

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {

  sgp30.measureAirQuality();
  float co2 = sgp30.CO2;

  Serial.println(co2);

  //flush serial and sleep for 1 second every loop
  Serial.flush();
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
}
