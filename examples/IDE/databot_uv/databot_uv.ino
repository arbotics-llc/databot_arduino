#include <Wire.h>
#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

VEML6075 uv;

// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);
  Serial.begin(9600);
  
  setupVEML6075(uv);

  Serial.flush();

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {
    
  float uva = uv.uva();
  float uvb = uv.uvb();
  float uvindex = uv.index(uva, uvb);

  Serial.println(uva);

  //wait for serial to finish and sleep for 250 milliseconds every loop
  Serial.flush();
  LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
    
}
