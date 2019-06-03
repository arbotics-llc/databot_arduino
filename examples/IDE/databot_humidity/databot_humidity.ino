#include <Wire.h>
#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

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

  Serial.flush();

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {

  shtc3.update();
  shtc3.sleep(true);
  float temperature = shtc3.toDegC();
  float relhumidity = shtc3.toPercent();
  float abshumidity = RHtoAbsolute(relhumidity,temperature);

  Serial.println(abshumidity);

  //wait for serial to finish and sleep for 250 milliseconds every loop
  Serial.flush();
  LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
}
