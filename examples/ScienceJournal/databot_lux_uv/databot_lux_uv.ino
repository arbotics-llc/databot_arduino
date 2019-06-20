#include <Wire.h>
#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

VEML6075 uv;
APDS9301 apds;

OpenLog myLog;
String myfile = "light.txt";

DynamicJsonDocument packet(200);

unsigned long loopStartTime = 0;
int loopDesiredPeriodMillis = 333;

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
  
  setupVEML6075(uv);
  setupApds(apds);

  Serial.flush();

  //open our log file and begin writing
  /*myLog.append(myfile);
  long sizeOfFile = myLog.size(myfile);
  //Serial.println(sizeOfFile);
  myLog.append(myfile);
  myLog.println(F(""));
  myLog.println(F("--------------------------------------------"));
  myLog.println(F("--------New Data Bot Logging Session--------"));
  myLog.println(F("--------------------------------------------"));
  myLog.syncFile();*/

  delay(1000);

}

// the loop function runs over and over again forever
void loop() {
    
    loopStartTime = millis();

    float lumflux = apds.readLuxLevel();
    float uva = uv.uva();
    float uvb = uv.uvb();
    float uvindex = uv.index( uva, uvb);

    //update our json packet with our new sensor values
    
    packet[F("lux")] = lumflux;
    packet[F("UV A")] = uva;
    packet[F("UV B")] = uvb;
    packet[F("UV Index")] = uvindex;
    packet[F("time")] = millis();
    
    sendPacket(packet);
    //logData(myLog, packet);

    //delay until we reach our desired loop time
    while(millis() < loopStartTime + loopDesiredPeriodMillis){
      delay(10);
    }
}
