#include <Wire.h>
#include <databot.h>

#define I2Cclock 400000
#define I2Cport Wire

//OneWire on pin 4 of the databot
OneWire oneWire(4);
DallasTemperature tempsensor(&oneWire);

OpenLog myLog;
String myfile = "externaltemperature.txt";

DynamicJsonDocument packet(200);

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

  tempsensor.requestTemperatures();
  float temperature = tempsensor.getTempCByIndex(0);

  //update our json packet with our new sensor values

  packet[F("temperature")] = temperature;
  packet[F("time")] = millis();
  
  sendPacket(packet);
  //logData(myLog, packet);

  //Sleep 500 milliseconds second every loop
  LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);
}
