#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

#define NAME "databot"

//give your current altitude in meters from sea level to get more accurate readings
#define CURRENT_ALTITUDE 0 

//uncomment below to store data to sd card
//#define SDCARD

//uncomment below to enable the 3115A2 altitude/pressure sensor
#define BAROMETER 
//uncomment one of the two below to enable pressure sensor or altitude sensor
#define READALTITUDE
//#define READPRESSURE

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

#define SOUND

//set to how often you want to grab sensor data in milliseconds
#define LOOP_TIME_MS 100

unsigned long loopStartTime = 0;

String myfile = "gsj.txt";
DynamicJsonDocument packet(300);

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
MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);
#endif

#ifdef UV
VEML6075 uv;
#endif

#ifdef LIGHT
APDS9301 apds;
#endif

#ifdef SOUND
#define UINTMAX 653535
#define samplingFrequency 8000

const byte adcPin = 0;
volatile unsigned int adcReading = 0;
volatile boolean adcDone;
volatile unsigned int lowSample = UINTMAX;
volatile unsigned int highSample = 0;
#endif

float smoothPressure = 0;
#define LPF_beta 0.90

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
  String name = "databot";
  setName(name);
  Serial.print("AT+RESET");
  delay(1000);

  setupSensors();

  smoothPressure = barometer.readPressure();
  //open our log file and begin writing
  #ifdef SDCARD
  Wire.setTimeout(1000000);
  myLog.append(myfile);
  long sizeOfFile = myLog.size(myfile);
  myLog.append(myfile);
  myLog.println(F(""));
  myLog.println(F("--------------------------------------------"));
  myLog.println(F("--------New Data Bot Logging Session--------"));
  myLog.println(F("--------------------------------------------"));
  //myLog.println(generateCsvHeader());
  myLog.syncFile();
  Wire.setTimeout(1000);
  #endif

  delay(1000);

}


int everyfourloops = 0;

// the loop function runs over and over again forever
void loop() {
    loopStartTime = millis();
    
    handleIMU();

    everyfourloops++;
    if(everyfourloops % 10 == 0){
    everyfourloops = 0;
    updateJson();
    
    sendPacket(packet);

    #ifdef SDCARD
    logData(myLog, packet);
    #endif
    }
    //delay until we reach our desired loop time
    while(millis() < loopStartTime + LOOP_TIME_MS)
    {
      delay(5);
    }

}

void handleIMU(){
  packet.clear();
  #ifdef IMU

    uint16_t fifo_count = 0;
    while(fifo_count == 0){
      fifo_count = myIMU.readFIFOcount();
    }
    //read out the buffer
    while(fifo_count > 0){
      myIMU.updateFromFIFO();
      if(fifo_count < 5){
        packet[F("k")].add((int16_t) (myIMU.ax * 1000));
        packet[F("l")].add((int16_t) (myIMU.ay * 1000));
        packet[F("m")].add((int16_t) (myIMU.az * 1000));
        packet[F("n")].add((int16_t) (myIMU.gx * DEG_TO_RAD * 100));
        packet[F("o")].add((int16_t) (myIMU.gy * DEG_TO_RAD * 100));
        packet[F("p")].add((int16_t) (myIMU.gz * DEG_TO_RAD * 100));
      }
      fifo_count--;
    }
    updateImuMag(myIMU);
    packet[F("q")] = (int16_t) (myIMU.mx * 100);
    packet[F("r")] = (int16_t) (myIMU.my * 100);
    packet[F("s")] = (int16_t) (myIMU.mz * 100);
    //packet[F("time")] = millis();
      
    #ifdef SOUND
    cli();
    float peakToPeak = (highSample - lowSample) * 3.3 / 1024;
    highSample = 0;
    lowSample = UINTMAX;
    packet[F("t")] = (20*log10(peakToPeak/0.00095));
    sei();
    #endif
    
    sendPacket(packet);
  
    #ifdef SDCARD
    logData(myLog, packet);
    #endif
    packet.clear();

  #endif
}

float relhumidity = 0;

void updateJson() {
  #ifdef BAROMETER 
    float rawReading; 
    rawReading = barometer.readPressure();
    smoothPressure = smoothPressure - (LPF_beta * (smoothPressure - rawReading));

    //calculate altitude from pressure
    packet[F("a")] = 44330.77 * (1-pow((smoothPressure/101326), 0.1902632));
    packet[F("b")] = smoothPressure;
  #endif

  #ifdef HUMIDITY
  shtc3.update();
  shtc3.sleep(true);
  packet[F("c")] = shtc3.toPercent();
  #endif

  #ifdef AIR_QUALITY
  sgp30.measureAirQuality();
  packet[F("d")] = sgp30.CO2;
  packet[F("e")] = sgp30.TVOC;
  #endif

  #ifdef EXTERNAL_TEMP
  packet[F("f")] = getExternalTemperature(tempsensor); 
  #endif
  
  #ifdef UV
  float uva = uv.uva();
  float uvb = uv.uvb();
  packet[F("g")] = (uint16_t) uva;
  packet[F("h")] = (uint16_t) uvb;
  packet[F("i")] = uv.index( uva, uvb );
  #endif

  #ifdef LIGHT
  packet[F("j")] = apds.readLuxLevel();
  #endif

  //packet[F("time")] = millis();

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
  setupIMU(myIMU, 50, accel_range, gyro_range, mag_range);
  myIMU.enableFIFO();
  #endif

  #ifdef UV
  setupVEML6075(uv);
  #endif

  #ifdef LIGHT
  setupApds(apds);
  #endif

  #ifdef SOUND
  enableADCINT();
  FlexiTimer2::set(1, 1.0/samplingFrequency, startADCRead); // *samplingFrequency* hz
  FlexiTimer2::start();
  #endif

}

void setName(String &name){
  String nameCmd = F("AT+NAMEHMSoft");
  nameCmd += name;

  Serial.print(nameCmd);
  Serial.flush();
  delay(60);
}

#ifdef SOUND

void enableADCINT(){
  cli(); //stops any interrupts
  
  ADMUX = 0;            // measure from pin 0
  ADMUX |= bit(REFS0);  // set AVCC as voltage reference
  ADMUX |= bit(ADLAR);  // use 8 bit resolution

  ADCSRA |= bit(ADPS2) | bit(ADPS1) | bit(ADPS0); // 128 prescale, 16mhz / 128 = 104us reads, slowest option
  ADCSRA |= bit(ADEN);  // turn on ADC
  ADCSRA |= bit(ADIE) | bit(ADSC);  // turn on Interrupts and start one ADC conversion

  sei();  //enables interrupts again
}

void startADCRead()
{
  ADCSRA |= bit(ADSC);
}

ISR(ADC_vect) {
  byte low;
  byte high;

  // must read low bytes first, this keeps low and high bytes synced
  low = ADCL >> 6;
  high = ADCH;

  adcReading = (high << 2) | low;

  if(adcReading > highSample){
    highSample = adcReading;
  }
  if(adcReading < lowSample){
    lowSample = adcReading;
  }

  adcDone = true;

}

#endif
