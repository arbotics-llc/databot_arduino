#include <databot.h>

// Start by finding your altitude from https://www.freemaptools.com/elevation-finder.htm or similar, then convert to meters
#define CURRENT_ALTITUDE_METERS 103.3

#define I2Cclock 400000
#define I2Cport Wire

MPL3115A2 barometer;

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
  
  setupMPL3115A2(barometer, ALTITUDE);

  barometer.trimAltitudeMeters(CURRENT_ALTITUDE_METERS);
  
  Serial.flush();

  delay(1000);  // pause while all sensors finish starting up

  timestamp = millis();

}

void loop() {
    
    // Place the altitude into field "a", and timestamp into field "b", as that's the
    // data exchange format by the packet sending code
    
    packet[F("m")] = ((millis()-timestamp) / 1000) * 1.0; // start with the "seconds" count of the sample
    
    packet[F("a")] = barometer.readAltitude();
    
    sendPacketFixedStringsFormat("ma", packet);

    delay(1000);

}
