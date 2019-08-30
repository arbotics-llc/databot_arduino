#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

//give your current altitude in meters from sea level to get more accurate readings
#define CURRENT_ALTITUDE 0 

//uncomment below to communicated with IDE over serial
#define IDE

//uncomment below to store data to sd card
#define SDCARD

//uncomment below to enable the 3115A2 altitude/pressure sensor
#define BAROMETER 

//uncomment below to enable the shtc3 humidity sensor
#define HUMIDITY

//uncomment below to enable the sgp30 air quality sensor
#define AIR_QUALITY

//uncomment below to enable the DS18B20 external temperature sensor, remember to plug it in!
#define EXTERNAL_TEMP 

//uncomment below to enable the MPU9250 intertial measurement unit
#define IMU

#define accel_range MPU9250::AFS_4G //AFS_2G, AFS_4G, AFS_8G, AFS_16G
#define gyro_range MPU9250::GFS_1000DPS //GFS_250DPS, GFS_500DPS, GFS_1000DPS, GFS_2000DPS
#define mag_range MPU9250::MFS_16BITS //MFS_14BITS, MFS_16BITS

//uncomment below to enable the VEML6075 ultraviolet sensor
#define UV

//uncomment below to enable the APDS9301 light sensor
#define LIGHT

//set to how often you want to grab sensor data in milliseconds
#define LOOP_TIME_MS 100

unsigned long loopStartTime = 0;

String myfile = "databot";
DynamicJsonDocument packet(200);

#ifdef BAROMETER
MPL3115A2 barometer;
float pressure = 0;
float altitude = 0;
#define LPF_beta 0.90
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
MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);
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

  pressure = barometer.readPressure();

  //open our log file and begin writing
  #ifdef SDCARD
  Wire.setTimeout(100000);
  myLog.append(myfile);
  long sizeOfFile = myLog.size(myfile);
  myLog.append(myfile);
  myLog.println(F(""));
  myLog.println(F("--------------------------------------------"));
  myLog.println(F("--------New Data Bot Logging Session--------"));
  myLog.println(F("--------------------------------------------"));
  myLog.println(generateCsvHeader());
  myLog.syncFile();
  Wire.setTimeout(1000);

  #endif

  delay(1000);

  #ifdef IDE
  Serial.println(generateCsvHeader());
  #endif

}

// the loop function runs over and over again forever
void loop() {
    
    loopStartTime = millis();
    
    updateSensors();
    String csv_sensors = generateCsvRecord();
    #ifdef SDCARD
    logData(myLog, csv_sensors);
    #endif

    #ifdef IDE
    Serial.println(csv_sensors);
    #endif
    
    //delay until we reach our desired loop time
    while(millis() < loopStartTime + LOOP_TIME_MS)
    {
      delay(5);
    }

}

void ideJson() {
  String logoutputjson = "";
  serializeJson(packet, logoutputjson);
  Serial.println(logoutputjson);
}

void updateSensors() {

  #ifdef BAROMETER 
  float rawReading; 
  rawReading = barometer.readPressure();
  pressure = pressure - (LPF_beta * (pressure - rawReading));

  //calculate altitude from pressure
  altitude = 44330.77 * (1-pow((pressure/101326), 0.1902632));
  #endif

  #ifdef HUMIDITY

  #endif

  #ifdef AIR_QUALITY

  #endif

  #ifdef EXTERNAL_TEMP

  #endif

  #ifdef IMU
    updateImuAcceleration(myIMU);
    updateImuGyro(myIMU);
    updateImuMag(myIMU);
  #endif

  #ifdef UV

  #endif

  #ifdef LIGHT

  #endif

}

void setupSensors() {

  #ifdef BAROMETER 
  setupMPL3115A2(barometer, PRESSURE);
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
      setupIMU(myIMU, 47, accel_range, gyro_range, mag_range);
    #endif
  #endif

  #ifdef UV
  setupVEML6075(uv);
  #endif

  #ifdef LIGHT
  setupApds(apds);
  #endif

}

String generateCsvHeader(){
  String csvHeader = "";
  #ifdef BAROMETER 
    csvHeader += ",altitude";
    csvHeader += ",pressure";
  #endif

  #ifdef HUMIDITY
    csvHeader += ",humidity";
  #endif

  #ifdef AIR_QUALITY
    csvHeader += ",CO2,TVOC";
  #endif

  #ifdef EXTERNAL_TEMP
    csvHeader += ",temperature";
  #endif

  #ifdef IMU
    csvHeader += ",a.x,a.y,a.z,g.x,g.y,g.z,m.x,m.y,m.z";
  #endif

  #ifdef UV
    csvHeader += ",'UV A','UV B','UV Index'";
  #endif

  #ifdef LIGHT
    csvHeader += ",lux";
  #endif

  csvHeader += ",time";

  //if leading character is , replace with whitespace
  if(csvHeader[0] == ','){
    csvHeader[0] = ' ';
  }
  //remove leading and trailing whitespace
  csvHeader.trim();

  return csvHeader;

}

String generateCsvRecord(){
  String csvRecord = "";
  String comma = ",";
  #ifdef BAROMETER 
  csvRecord += comma + altitude;
  csvRecord += comma + pressure;
  #endif

  #ifdef HUMIDITY
  shtc3.update();
  shtc3.sleep(true);
  csvRecord += comma + shtc3.toPercent();
  #endif

  #ifdef AIR_QUALITY
  sgp30.measureAirQuality();
  csvRecord += comma +   (float)sgp30.CO2;
  csvRecord += comma +  (float)sgp30.TVOC;
  #endif

  #ifdef EXTERNAL_TEMP
  csvRecord += comma + getExternalTemperature(tempsensor); 
  #endif

  #ifdef IMU
  csvRecord += comma + myIMU.ax;
  csvRecord += comma + myIMU.ay;
  csvRecord += comma + myIMU.az;
  csvRecord += comma + myIMU.gx * DEG_TO_RAD;
  csvRecord += comma + myIMU.gy * DEG_TO_RAD; 
  csvRecord += comma + myIMU.gz * DEG_TO_RAD;
  csvRecord += comma + myIMU.mx;
  csvRecord += comma + myIMU.my;
  csvRecord += comma + myIMU.mz;
  #endif

  #ifdef UV
  float uva = uv.uva();
  float uvb = uv.uvb();
  csvRecord += comma + uva;
  csvRecord += comma + uvb;
  csvRecord += comma + uv.index( uva, uvb );
  #endif

  #ifdef LIGHT
  csvRecord += comma + apds.readLuxLevel();
  #endif

  csvRecord += comma + millis();

  //if leading character is , replace with whitespace
  if(csvRecord[0] == ','){
    csvRecord[0] = ' ';
  }
  //remove leading and trailing whitespace
  csvRecord.trim();

  return csvRecord;
}
