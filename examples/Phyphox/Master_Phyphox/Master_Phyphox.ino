// See https://databot.us.com/geek for full sensor information and spec sheets

#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

#define CURRENT_ALTITUDE 0 // meters

float smoothPressure = 0;

#define LPF_beta 0.100

#define SERIAL_PORT Serial

#define accel_range MPU9250::AFS_4G //AFS_2G, AFS_4G, AFS_8G, AFS_16G
#define gyro_range MPU9250::GFS_1000DPS //GFS_250DPS, GFS_500DPS, GFS_1000DPS, GFS_2000DPS
#define mag_range MPU9250::MFS_16BITS //MFS_14BITS, MFS_16BITS

DynamicJsonDocument packet(200);

unsigned long timestamp;

// Sensor array

MPL3115A2 barometer;

APDS9301 apds;

SGP30 sgp30;

SHTC3 shtc3;

VEML6075 uv;

MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);

OneWire oneWire(4);

DallasTemperature tempsensor(&oneWire);

void setup() {

  pinMode(A3, OUTPUT);

  digitalWrite(A3, LOW);

  Serial.begin(9600);
  
  Serial.flush();

  delay(1000);  // pause while sensors finish starting up

  timestamp = millis();

  randomSeed(analogRead(0));

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

  setupIMU(myIMU, 255, accel_range, gyro_range, mag_range);
  
  smoothPressure = barometer.readPressure();

  Serial.flush();

}

void loop() {

  // Query the sensors
  
  float rawReading; 

  rawReading = barometer.readPressure();

  smoothPressure = smoothPressure - (LPF_beta * (smoothPressure - rawReading));

  //calculate altitude from pressure

  shtc3.update();
  shtc3.sleep(true);

  float relhumidity = shtc3.toPercent();
  float abshumidity = RHtoAbsolute(relhumidity,shtc3.toDegC());

  sgp30.measureAirQuality();

  float co2 = sgp30.CO2;
  float tvoc = sgp30.TVOC;

  float lumflux = apds.readLuxLevel();
  float uva = uv.uva();
  float uvb = uv.uvb();
  float uvindex = uv.index( uva, uvb);

  updateImuAcceleration(myIMU);
  updateImuGyro(myIMU);
  updateImuMag(myIMU);

  packet[F("m")] = ((millis()-timestamp));

  packet[F("c")] = co2; // CO2
  packet[F("v")] = tvoc; // VOC
  packet[F("h")] = relhumidity; // Humidity
  packet[F("t")] = getExternalTemperature(tempsensor); // Temperature

  packet[F("p")] = smoothPressure / 100.0; // Air pressure
  packet[F("e")] = 44330.77 * (1-pow((smoothPressure/101326), 0.1902632)); // Altitude
  packet[F("u")] = uva; // UVa
  packet[F("b")] = uvb; // UVb
  packet[F("i")] = uvindex; // Index
  packet[F("x")] = lumflux; // Lux

  packet[F("a")] = myIMU.ax * 9.8;
  packet[F("d")] = myIMU.ay * 9.8;
  packet[F("f")] = myIMU.az * 9.8;
  packet[F("g")] = myIMU.gx * DEG_TO_RAD;
  packet[F("j")] = myIMU.gy * DEG_TO_RAD; 
  packet[F("k")] = myIMU.gz * DEG_TO_RAD;
  packet[F("l")] = myIMU.mx;
  packet[F("n")] = myIMU.my;
  packet[F("o")] = myIMU.mz;

  sendPacketEx("mcvhtpeubixadfgjklno", packet);

  delay(100);

}
