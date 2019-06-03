#include <Wire.h>
#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

APDS9301 apds;

// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);
  Serial.begin(115200);
  
  setupApds(apds);

  Serial.flush();

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {
    
  float lumflux = apds.readLuxLevel();

  Serial.println(lumflux);

  //wait for serial to finish and sleep for 250 milliseconds every loop
  Serial.flush();
  LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
    
}
