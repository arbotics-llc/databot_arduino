#include <databot.h>

#define SERIAL_PORT Serial

#define I2Cclock 400000
#define I2Cport Wire

SHTC3 shtc3;

SGP30 sgp30;

DynamicJsonDocument packet(200);

unsigned long timestamp;

void setup() {

  //setup i2c and set clock speed to 40khz

  Wire.begin();
  Wire.setClock(I2Cclock);

  pinMode(A3, OUTPUT);
  pinMode(13, OUTPUT);

  digitalWrite(A3, LOW);
  digitalWrite(13, LOW);

  Serial.begin(9600);
  
  Serial.flush();

  shtc3.setMode(SHTC3_CMD_CSE_RHF_LPM);

  shtc3.begin();

  sgp30.begin();

  sgp30.initAirQuality();

  sgp30.setHumidity(doubleToFixedPoint(RHtoAbsolute(shtc3.toPercent(),shtc3.toDegC())));

  delay(1000);  // pause while sensors finish starting up

  timestamp = millis();

}

void loop() {

  SHTC3_Status_TypeDef result = shtc3.update();

  packet[F("m")] = ((millis()-timestamp) / 1000) * 1.0; // start with the "seconds" count of the sample
  
  packet[F("h")] = shtc3.toPercent();

  sgp30.measureAirQuality();

  packet[F("c")] = sgp30.CO2;

  packet[F("v")] = sgp30.TVOC;

  sendPacketFixedStringsFormat("mhcv", packet);

  delay(1000);

}
