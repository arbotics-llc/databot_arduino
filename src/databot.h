/*    
Copyright aRbotics llc 2019
This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libs/ArduinoJson.h"
#include "libs/MPU9250.h"
#include "libs/SparkFun_VEML6075_Arduino_Library.h"
#include "libs/Sparkfun_APDS9301_Library.h"
#include "libs/SparkFunMPL3115A2.h"
#include "libs/SparkFun_Qwiic_OpenLog_Arduino_Library.h"
#include "libs/SparkFun_SHTC3.h"
#include "libs/SparkFun_SGP30_Arduino_Library.h"
#include "libs/arduinoFFT.h"
#include "libs/FlexiTimer2.h"
#include "libs/DallasTemperature.h"
#include "libs/LowPower.h"
#include "libs/NewTone.h"

#define MPU9250_ADDRESS MPU9250_ADDRESS_AD1

void updateImuAcceleration(MPU9250 &imu){
    if (imu.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
    {
      imu.readAccelData(imu.accelCount);  // Read the x/y/z adc values
  
      // Now we'll calculate the accleration value into actual g's
      // This depends on scale being set
      imu.ax = (float)imu.accelCount[0] * imu.aRes;// - imu.accelBias[0];
      imu.ay = (float)imu.accelCount[1] * imu.aRes;// - imu.accelBias[1];
      imu.az = (float)imu.accelCount[2] * imu.aRes;// - imu.accelBias[2];
    }
}

void updateImuGyro(MPU9250 &imu){

    imu.readGyroData(imu.gyroCount);
    
    imu.gx = (float)imu.gyroCount[0] * imu.gRes;
    imu.gy = (float)imu.gyroCount[1] * imu.gRes;
    imu.gz = (float)imu.gyroCount[2] * imu.gRes;
}

void updateImuMag(MPU9250 &imu){

    imu.readMagData(imu.magCount);

    imu.mx = (float)imu.magCount[0] * imu.mRes
               * imu.factoryMagCalibration[0] - imu.magBias[0];
    imu.my = (float)imu.magCount[1] * imu.mRes
               * imu.factoryMagCalibration[1] - imu.magBias[1];
    imu.mz = (float)imu.magCount[2] * imu.mRes
               * imu.factoryMagCalibration[2] - imu.magBias[2];
}

//sample_rate = 47 //anywhere from 0x00 to 0xFF, output rate = 1khz/(1+sample_rate), 47 should give us ~20hz, 0x00 gives 1khz, 0xFF gives ~4hz 
//afs_opt = 0 //AFS_2G
//gfs_opt = 0 //GFS_250DPS
//mfs_opt = 0 //MFS_14BITS
void setupIMU(MPU9250 &imu, uint8_t sample_rate = 47, uint8_t afs_opt = 0, uint8_t gfs_opt = 0, uint8_t mfs_opt = 0){
  byte c = imu.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);

  if (c == 0x71) // WHO_AM_I should always be 0x71
  {
    // Start by performing self test and reporting values
    imu.MPU9250SelfTest(imu.selfTest);
        Serial.print(F("x-axis self test: acceleration trim within : "));
    Serial.print(imu.selfTest[0],1); Serial.println("% of factory value");
    Serial.print(F("y-axis self test: acceleration trim within : "));
    Serial.print(imu.selfTest[1],1); Serial.println("% of factory value");
    Serial.print(F("z-axis self test: acceleration trim within : "));
    Serial.print(imu.selfTest[2],1); Serial.println("% of factory value");
    Serial.print(F("x-axis self test: gyration trim within : "));
    Serial.print(imu.selfTest[3],1); Serial.println("% of factory value");
    Serial.print(F("y-axis self test: gyration trim within : "));
    Serial.print(imu.selfTest[4],1); Serial.println("% of factory value");
    Serial.print(F("z-axis self test: gyration trim within : "));
    Serial.print(imu.selfTest[5],1); Serial.println("% of factory value");
    // Calibrate gyro and accelerometers, load biases in bias registers
    //imu.calibrateMPU9250(imu.gyroBias, imu.accelBias);
    
    imu.initMPU9250(afs_opt, gfs_opt, sample_rate);
    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = imu.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    if (d != 0x48)
    {
      // Communication failed, stop here
      Serial.println(F("Communication failed, abort!"));
      Serial.flush();
    }
    // Get magnetometer calibration from AK8963 ROM
    imu.initAK8963(imu.factoryMagCalibration, mfs_opt);
    // Initialize device for active mode read of magnetometer

    imu.getAres();
    imu.getGres();
    imu.getMres();
   
  }
}

#define ALTITUDE 0
#define PRESSURE 1

void setupMPL3115A2(MPL3115A2 &barometer, int option){
  barometer.begin();

  if(option == ALTITUDE){
    barometer.setModeAltimeter(); // Measure altitude above sea level in meters
  }else if(option == PRESSURE){
    barometer.setModeBarometer();
  }
  barometer.setOversampleRate(4); // Set Oversample to the recommended 128
  barometer.enableEventFlags(); // Enable all three pressure and temp event flags
  delay(5000);
}

void setupApds(APDS9301 &apds){
  apds.begin(0x39);
  apds.setGain(APDS9301::LOW_GAIN);
  apds.setIntegrationTime(APDS9301::INT_TIME_101_MS);
  apds.setLowThreshold(0); 
  apds.setHighThreshold(50);
  apds.setCyclesForInterrupt(1);
  apds.enableInterrupt(APDS9301::INT_OFF);
  apds.clearIntFlag();
}

VEML6075_error_t setupVEML6075(VEML6075 &uv){
  VEML6075_error_t ret;

  ret = uv.begin();
  if (ret != VEML6075_ERROR_SUCCESS) {
    Serial.println("no comms with VEML6075.");
    return ret;
  }

  ret = uv.setIntegrationTime(VEML6075::IT_200MS);
  if (ret != VEML6075_ERROR_SUCCESS) {
    Serial.println("failed to set integration time");
    return ret;
  }
}

double RHtoAbsolute(float relHumidity, float tempC) {
  double eSat = 6.11 * pow(10.0, (7.5 * tempC / (237.7 + tempC)));
  double vaporPressure = (relHumidity * eSat) / 100; //millibars
  double absHumidity = 1000 * vaporPressure * 100 / ((tempC + 273) * 461.5); //Ideal gas law with unit conversions
  return absHumidity;
}

uint16_t doubleToFixedPoint( double number) {
  int power = 1 << 8;
  double number2 = number * power;
  uint16_t value = floor(number2 + 0.5);
  return value;
}

void sendPacket(DynamicJsonDocument &packet){
  //msgpack is a binary serialized version of json, not very human readable but
  //a lot smaller and easier on the bluetooth. We send this data over serial
  //to the Bluetooth Low Energy module
  char packetstart[] = "msgstart";
  
  char msgpacketbuf[256];
  memset(msgpacketbuf,0,256*sizeof(char));
  size_t buf_size = serializeMsgPack(packet, msgpacketbuf);
  int i = 0;
  int j = 0;
  Serial.print(packetstart);
  delay(60);
  for(i = 0; i < buf_size; i++){
    Serial.print(msgpacketbuf[i]);
    j++;
    if(j == 20){
      j=0;
      delay(60);
    }
  }
}

void logData(OpenLog &myLog, DynamicJsonDocument &packet) {
  //json is a human readable data format, not as small as msgpack but this way
  //we can understand what is stored in the sd card easily. This data gets sent
  //over i2c to the sd card
  String logoutputjson = "";
  serializeJson(packet, logoutputjson);
  myLog.println(logoutputjson);
  myLog.syncFile();

  logoutputjson = "";
}
