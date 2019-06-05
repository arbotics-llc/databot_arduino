#include <Wire.h>
#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

APDS9301 apds;
SGP30 sgp30;
SHTC3 shtc3;
MPL3115A2 barometer;

OpenLog myLog;
String myfile = "enviornmental.txt";

DynamicJsonDocument packet(200);

// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);

  //setup sdcard
  myLog.begin();
  
  //we must call packet["n"] = "packet"; before any other jsondocument calls
  //so that this text appears in the beginning of the packets so the app can
  //tell the difference between start, middle, or end packets
  pinMode(A3, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(A3, LOW);
  digitalWrite(13, LOW);

  Serial.begin(9600);

  setupMPL3115A2(barometer, PRESSURE);
  setupApds(apds);
  shtc3.begin();
  shtc3.setMode(SHTC3_CMD_CSE_RHF_LPM);
  shtc3.sleep(true);
  shtc3.update();
  sgp30.begin();
  sgp30.initAirQuality();
  sgp30.setHumidity(doubleToFixedPoint(RHtoAbsolute(shtc3.toPercent(),shtc3.toDegC())));
  
  Serial.flush();

  //open our log file and begin writing
  myLog.append(myfile);
  long sizeOfFile = myLog.size(myfile);
  //Serial.println(sizeOfFile);
  myLog.append(myfile);
  myLog.println(F(""));
  myLog.println(F("--------------------------------------------"));
  myLog.println(F("--------New Data Bot Logging Session--------"));
  myLog.println(F("--------------------------------------------"));
  myLog.syncFile();

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {

  shtc3.update();
  shtc3.sleep(true);
  float temperature = shtc3.toDegC();
  float relhumidity = shtc3.toPercent();
  float abshumidity = RHtoAbsolute(relhumidity,temperature);

  sgp30.measureAirQuality();
  float co2 = sgp30.CO2;
  float tvoc = sgp30.TVOC;
  float pressure = barometer.readPressure();

  //update our json packet with our new sensor values

  packet[F("pressure")] = pressure;
  packet[F("temperature")] = temperature;
  packet[F("relative humidity")] = relhumidity;
  packet[F("absolute humidity")] = abshumidity;
  packet[F("CO2")] = co2;
  packet[F("TVOC")] = tvoc;
  packet[F("time")] = millis();
  
  sendPacket(packet);
  logData(myLog, packet);

  //Sleep for one second every loop
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
}
