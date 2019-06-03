#include <databot.h>
#include <Wire.h>

#define I2Cclock 400000
#define I2Cport Wire

MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);


// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);

  Serial.begin(115200);

  setupIMU(myIMU);
  
  Serial.flush();

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {
  
  updateImuAcceleration(myIMU);
  updateImuGyro(myIMU);
  updateImuMag(myIMU);
  
  Serial.print(myIMU.ax);
  Serial.print(",");
  Serial.print(myIMU.ay);
  Serial.print(",");
  Serial.println(myIMU.az);

  //wait for serial to finish then sleep for 15 milliseconds every loop
  Serial.flush();
  LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF);

}
