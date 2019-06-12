#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

//uncomment below to communicated with IDE over serial
#define IDE

//either communicate over serial with arduino ide or with science journal with BLE
//doing both would take too much time...
#ifndef IDE
   //uncomment below to communicate with science journal
   //#define GSJ
#endif

//uncomment below to store data to sd card
//#define SDCARD

//uncomment below to enable the 3115A2 altitude/pressure sensor
#define BAROMETER 
//uncomment one of the two below to enable pressure sensor or altitude sensor
#define READALTITUDE
//#define READPRESSURE

//give your current altitude in meters from sea level to get more accurate readings
#define CURRENT_ALTITUDE 0 

//uncomment below to enable the shtc3 humidity sensor
#define HUMIDITY

//uncomment below to enable the sgp30 air quality sensor
#define AIR_QUALITY

//uncomment below to enable the DS18B20 external temperature sensor, remember to plug it in!
#define EXTERNAL_TEMP 

//uncomment below to enable the MPU9250 intertial measurement unit
#define IMU

//uncomment below to enable the VEML6075 ultraviolet sensor
#define UV

//uncomment below to enable the APDS9301 light sensor
#define LIGHT

//set to how often you want to grab sensor data in milliseconds
#define LOOP_TIME_MS 1000

unsigned long loopStartTime = 0;

String myfile = "sensors.txt";
DynamicJsonDocument packet(200);

#ifdef BAROMETER
MPL3115A2 barometer;
#endif

#ifdef SDCARD
OpenLog myLog;
#endif

#ifdef HUMIDITY
SHTC3 shtc3;
#endif

#ifdef AIR_QUALITY
SGP30 sgp30;
#endif

#ifdef EXTERNAL_TEMP 
OneWire oneWire(4);
DallasTemperature tempsensor(&oneWire);
#endif

#ifdef IMU
MPU9250_DMP myIMU;
#endif

#ifdef UV
VEML6075 uv;
#endif

#ifdef LIGHT
APDS9301 apds;
#endif

// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);
  
  pinMode(A3, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(A3, LOW);
  digitalWrite(13, LOW);

  Serial.begin(9600);

  setupSensors();

  //open our log file and begin writing
  #ifdef SDCARD
  myLog.append(myfile);
  long sizeOfFile = myLog.size(myfile);
  myLog.append(myfile);
  myLog.println(F(""));
  myLog.println(F("--------------------------------------------"));
  myLog.println(F("--------New Data Bot Logging Session--------"));
  myLog.println(F("--------------------------------------------"));
  myLog.syncFile();
  #endif

  delay(1000);

}


// the loop function runs over and over again forever
void loop() {
    
    loopStartTime = millis();
    
    updateSensors();
    handleIMU();
    updateJson();

    #ifdef IDE
    ideJson();
    #endif
    
    #ifdef GSJ
    sendPacket(packet);
    #endif

    #ifdef SDCARD
    logData(myLog, packet);
    #endif

    //delay until we reach our desired loop time
    while(millis() < loopStartTime + LOOP_TIME_MS)
    {
      delay(5);
    }

}

void handleIMU(){
  #ifdef IMU
    packet.clear();
    
    packet[F("a.x")] = myIMU.calcAccel(myIMU.ax);
    packet[F("a.y")] = myIMU.calcAccel(myIMU.ay);
    packet[F("a.z")] = myIMU.calcAccel(myIMU.az);
    packet[F("g.x")] = myIMU.calcGyro(myIMU.gx);
    packet[F("g.y")] = myIMU.calcGyro(myIMU.gy); 
    packet[F("g.z")] = myIMU.calcGyro(myIMU.gz);
    packet[F("m.x")] = myIMU.calcMag(myIMU.mx);
    packet[F("m.y")] = myIMU.calcMag(myIMU.my);
    packet[F("m.z")] = myIMU.calcMag(myIMU.mz);
    packet[F("time")] = millis();
  
    #ifdef IDE
    ideJson();
    #endif
    
    #ifdef GSJ
    sendPacket(packet);
    #endif
  
    #ifdef SDCARD
    logData(myLog, packet);
    #endif
  
    packet.clear();

  #endif
}

void ideJson() {
  String logoutputjson = "";
  serializeJson(packet, logoutputjson);
  Serial.println(logoutputjson);
}

float smoothAltitude = 0;
float smoothPressure = 0;
#define LPF_beta 0.100

float relhumidity = 0;

void updateJson() {
  #ifdef BAROMETER 

    #ifdef READALTITUDE
    packet[F("alt")] = smoothAltitude;
    #endif

    #ifdef READPRESSURE
    packet[F("pres")] = smoothPressure;
    #endif

  #endif

  #ifdef HUMIDITY
  shtc3.update();
  shtc3.sleep(true);
  packet[F("humidity")] = shtc3.toPercent();
  #endif

  #ifdef AIR_QUALITY
  sgp30.measureAirQuality();
  packet[F("CO2")] = (float)sgp30.CO2;
  packet[F("TVOC")] = (float)sgp30.TVOC;
  #endif

  #ifdef EXTERNAL_TEMP
  tempsensor.requestTemperatures();
  packet[F("temp")] = tempsensor.getTempCByIndex(0);
  #endif

  #ifdef UV
  float uva = uv.uva();
  float uvb = uv.uvb();
  packet[F("UV A")] = uv.uva();
  packet[F("UV B")] =  uv.uvb();
  packet[F("UV Index")] = uv.index( uva, uvb );
  #endif

  #ifdef LIGHT
  packet[F("lum")] = apds.readLuxLevel();
  #endif

  packet[F("time")] = millis();

}

void updateSensors() {

  #ifdef BAROMETER 
  float rawReading; 

    #ifdef READALTITUDE
    rawReading = barometer.readAltitude();
    smoothAltitude = smoothAltitude - (LPF_beta * (smoothAltitude - rawReading)); 
    #endif

    #ifdef READPRESSURE
    rawReading = barometer.readPressure();
    smoothPressure = smoothPressure - (LPF_beta * (smoothAltitude - rawReading);
    #endif
  #endif

  #ifdef HUMIDITY

  #endif

  #ifdef AIR_QUALITY

  #endif

  #ifdef EXTERNAL_TEMP

  #endif

  #ifdef IMU
  updateImu(myIMU);
  #endif

  #ifdef UV

  #endif

  #ifdef LIGHT

  #endif

}

void setupSensors() {

  #ifdef BAROMETER 

    #ifdef READALTITUDE
    setupMPL3115A2(barometer, ALTITUDE);
    barometer.trimAltitudeMeters(CURRENT_ALTITUDE);
    #endif
    
    #ifdef READPRESSURE
    setupMPL3115A2(barometer, PRESSURE);
    #endif

  #endif

  #ifdef SDCARD
  myLog.begin();
  #endif

  #ifdef HUMIDITY
  shtc3.begin();
  shtc3.setMode(SHTC3_CMD_CSE_RHF_LPM);
  shtc3.sleep(true);
  shtc3.update();
  #endif

  #ifdef AIR_QUALITY
  sgp30.begin();
  sgp30.initAirQuality();
    #ifdef HUMIDITY
    sgp30.setHumidity(doubleToFixedPoint(RHtoAbsolute(shtc3.toPercent(),shtc3.toDegC())));
    #endif
  #endif

  #ifdef EXTERNAL_TEMP
  //needs no extra setup
  #endif

  #ifdef IMU
    #ifdef IDE
      setupIMU(myIMU, 4, 1000, 50, 50);
    #endif

    #ifdef GSJ
      setupIMU(myIMU, 4, 1000, 4, 4);
    #endif
  #endif

  #ifdef UV
  setupVEML6075(uv);
  #endif

  #ifdef LIGHT
  setupApds(apds);
  #endif

}
