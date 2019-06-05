#include <databot.h>
#define CURRENT_ALTITUDE 824

#define I2Cclock 400000
#define I2Cport Wire

MPL3115A2 barometer;

OpenLog myLog;
String myfile = "altitude.txt";

DynamicJsonDocument packet(200);

float oldaltitude = 0;
unsigned long loopStartTime = 0;
int loopDesiredPeriodMillis = 200;

// the setup function runs once when you press reset or power the board
void setup() {
  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);

  //setup sdcard
  myLog.begin();
  
  pinMode(A3, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(A3, LOW);
  digitalWrite(13, LOW);

  Serial.begin(9600);
  
  setupMPL3115A2(barometer, ALTITUDE);
  barometer.trimAltitudeMeters(CURRENT_ALTITUDE);
  
  Serial.flush();

  //open our log file and begin writing
  myLog.append(myfile);
  long sizeOfFile = myLog.size(myfile);
  myLog.append(myfile);
  myLog.println(F(""));
  myLog.println(F("--------------------------------------------"));
  myLog.println(F("--------New Data Bot Logging Session--------"));
  myLog.println(F("--------------------------------------------"));
  myLog.syncFile();

  delay(1000);

}

float smoothAltitude = 824;
#define LPF_beta 0.100

// the loop function runs over and over again forever
void loop() {
    
    loopStartTime = millis();

    float rawAltitude = barometer.readAltitude();
    smoothAltitude = smoothAltitude - (LPF_beta * (smoothAltitude - rawAltitude));
    
    //update our json packet with our new sensor values
    
    packet[F("altitude")] = smoothAltitude;
    packet[F("altraw")] = rawAltitude; 
    packet[F("time")] = millis();
    
    sendPacket(packet);
    logData(myLog, packet);

    //delay until we reach our desired loop time
    while(millis() < loopStartTime + loopDesiredPeriodMillis)
    {
      delay(10);
    }
}
