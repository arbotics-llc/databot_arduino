#include <databot.h>
#include <Wire.h>

#define I2Cclock 400000
#define I2Cport Wire

MPU9250_DMP myIMU;

// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);

  Serial.begin(9600);

  setupIMU(myIMU, 4, 1000, 50, 10);
  
  Serial.flush();

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {

  updateImu(myIMU);

  Serial.print(myIMU.calcAccel(myIMU.ax));
  Serial.print(",");
  Serial.print(myIMU.calcAccel(myIMU.ay));
  Serial.print(",");
  Serial.println(myIMU.calcAccel(myIMU.az));

  //wait for serial to finish then sleep for 15 milliseconds every loop
  Serial.flush();
  LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF);

}
