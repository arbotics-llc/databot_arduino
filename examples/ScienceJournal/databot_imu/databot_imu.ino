#include <databot.h>
#include <Wire.h>

#define I2Cclock 400000
#define I2Cport Wire

MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);

OpenLog myLog;
String myfile = "imusensordata.txt";

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

  setupIMU(myIMU);
  
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
  digitalWrite(A3, LOW);
  digitalWrite(13, LOW);

  updateImuAcceleration(myIMU);
  updateImuGyro(myIMU);
  updateImuMag(myIMU);
  
  //update our json packet with our new sensor values

  packet[F("a.x")] = myIMU.ax;
  packet[F("a.y")] = myIMU.ay;
  packet[F("a.z")] = myIMU.az;
  packet[F("g.x")] = myIMU.gx;
  packet[F("g.y")] = myIMU.gy;
  packet[F("g.z")] = myIMU.gz;
  packet[F("m.x")] = myIMU.mx;
  packet[F("m.y")] = myIMU.my;
  packet[F("m.z")] = myIMU.mz;
  packet[F("time")] = millis();
  
  sendPacket(packet);
  //logData(myLog, packet);

  //delay until we reach our desired loop time
  while(millis() < loopStartTime + loopDesiredPeriodMillis)
  {
    delay(10);
  }
}
