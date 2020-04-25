#include <databot.h>

#define CURRENT_ALTITUDE 824

float smoothAltitude = 824;

#define LPF_beta 0.100

#define SERIAL_PORT Serial

DynamicJsonDocument packet(200);

unsigned long timestamp;

// Sensor array

MPL3115A2 barometer;
APDS9301 apds;
SGP30 sgp30;
SHTC3 shtc3;
VEML6075 uv;

void setup() {

  pinMode(A3, OUTPUT);

  digitalWrite(A3, LOW);

  Serial.begin(9600);
  
  Serial.flush();

  delay(1000);  // pause while sensors finish starting up

  timestamp = millis();

  randomSeed(analogRead(0));

  setupMPL3115A2(barometer, ALTITUDE);

  barometer.trimAltitudeMeters(CURRENT_ALTITUDE);

  setupMPL3115A2(barometer, PRESSURE);

  setupApds(apds);

  shtc3.begin();
  shtc3.setMode(SHTC3_CMD_CSE_RHF_LPM);
  shtc3.sleep(true);
  shtc3.update();

  sgp30.begin();
  sgp30.initAirQuality();
  sgp30.setHumidity(doubleToFixedPoint(RHtoAbsolute(shtc3.toPercent(),shtc3.toDegC())));

  setupVEML6075(uv);

}

void loop() {

  // Query the sensors
  
  smoothAltitude = smoothAltitude - (LPF_beta * (smoothAltitude - barometer.readAltitude()));
  
  shtc3.update();
  shtc3.sleep(true);

  float temperature = shtc3.toDegC();
  float relhumidity = shtc3.toPercent();
  float abshumidity = RHtoAbsolute(relhumidity,temperature);

  sgp30.measureAirQuality();

  float co2 = sgp30.CO2;
  float tvoc = sgp30.TVOC;
  float pressure = barometer.readPressure();

  float lumflux = apds.readLuxLevel();
  float uva = uv.uva();
  float uvb = uv.uvb();
  float uvindex = uv.index( uva, uvb);

  packet[F("m")] = ((millis()-timestamp) / 1000) * 1.0;

  packet[F("c")] = co2; // CO2
  packet[F("v")] = tvoc; // VOC
  packet[F("h")] = relhumidity; // Humidity
  packet[F("t")] = temperature; // Temperature
  packet[F("p")] = pressure; // Air pressure
  packet[F("e")] = smoothAltitude; // Altitude
  packet[F("u")] = uva; // UVa
  packet[F("b")] = uvb; // UVb
  packet[F("i")] = uvindex; // Index
  packet[F("x")] = lumflux; // Lux

  sendPacketEx("mcvhtpeubix", packet);

  delay(1000);

}
