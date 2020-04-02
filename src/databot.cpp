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

#include "databot.h"

/*
* Function: isImuReady
* --------------------
* checks to see if a new reading from the MPU9250 is available
*
* arguments
* imu: pointer to the MPU9250 imu object
*
* returns true if a reading is available, false if no reading is available
*/
bool isImuReady(MPU9250 &imu){

    if (imu.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01){
      return true;
    }else{
      return false;
    }

}

/*
* Function updateImuAcceleration
* ------------------------------
* grabs new raw acceleromater data from MPU9250, calculates proper range and 
ast delay* updates associated member variables
*
* usage note: only call this after isImuReady has returned true or you may get stale data
*
* arguments
* imu: pointer to the MPU9250 imu object
* 
* returns void
*/
void updateImuAcceleration(MPU9250 &imu){

    imu.readAccelData(imu.accelCount);  // Read the x/y/z adc values
  
    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    imu.ax = (float)imu.accelCount[0] * imu.aRes;// - imu.accelBias[0];
    imu.ay = (float)imu.accelCount[1] * imu.aRes;// - imu.accelBias[1];
    imu.az = (float)imu.accelCount[2] * imu.aRes;// - imu.accelBias[2];

}

/*
* Function updateImuGyro
* ------------------------------
* grabs new raw gyroscope data from MPU9250, calculates proper range and 
* updates associated member variables
*
* usage note: only call this after isImuReady has returned true or you may get stale data
*
* arguments
* imu: pointer to the MPU9250 imu object
* 
* returns void
*/
void updateImuGyro(MPU9250 &imu){

    imu.readGyroData(imu.gyroCount);
    
    imu.gx = (float)imu.gyroCount[0] * imu.gRes;
    imu.gy = (float)imu.gyroCount[1] * imu.gRes;
    imu.gz = (float)imu.gyroCount[2] * imu.gRes;
}

/*
* Function updateImuMag
* ------------------------------
* grabs new raw magnetometer data from MPU9250, calculates proper range and 
* updates associated member variables
*
* usage note: the update rate of the Magnetometer can be much slower than the gyro and accelerometer
* you may experience some aliasing artifacts 
*
* arguments
* imu: pointer to the MPU9250 imu object
* 
* returns void
*/
void updateImuMag(MPU9250 &imu){

    imu.readMagData(imu.magCount);

    imu.mx = (float)imu.magCount[0] * imu.mRes
               * imu.factoryMagCalibration[0] - imu.magBias[0];
    imu.my = (float)imu.magCount[1] * imu.mRes
               * imu.factoryMagCalibration[1] - imu.magBias[1];
    imu.mz = (float)imu.magCount[2] * imu.mRes
               * imu.factoryMagCalibration[2] - imu.magBias[2];
}

/*
* Function: setupIMU
* ------------------
* helper function to set up the MPU9250
*
* arguments
* imu: pointer to the MPU9250 object
* sample_rate_divider: default = 47 //anywhere from 0x00 to 0xFF, output rate = 
*   1khz/(1+sample_rate), 47 should give us ~20hz, 0x00 gives 1khz, 0xFF gives ~4hz 
*
* afs_opt: default = 0 //AFS_2G
*   AFS_2G = accelerometer reads from -2 to 2 g-forces, AFS_4G reads from -4to 4
*   AFS_8G from -8 to 8, AFS_16G from -16 to 16, loss in precision as you go up
*
* gfs_opt: default = 0 //GFS_250DPS
*   GFS_250DPS = gyro reads from -250 to 250 degrees per second, GFS_500DPS from -500 to 500
*   GFS_1000DPS from -1000 to 1000, GFS_2000DPS from -2000 to 2000
*   Again, loss in precision as you go up
*
* mfs_opt: default = 0 //MFS_14BITS
*   MFS_14BITS = magnetometer reads a 14 bit value, MFS_16BITS magnetometer reads a 16 bit value
*
* returns: true on succesful setup, false on communication failure
*/

bool setupIMU(MPU9250 &imu, uint8_t sample_rate_divider = 47, uint8_t afs_opt = 0, uint8_t gfs_opt = 0, uint8_t mfs_opt = 0){
  byte c = imu.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);

  if (c == 0x71) // WHO_AM_I should always be 0x71
  {
    imu.initMPU9250(afs_opt, gfs_opt, sample_rate_divider);
    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = imu.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    if (d != 0x48)
    {
      // Communication failed, stop here
      Serial.println(F("Communication failed, abort!"));
      Serial.flush();
      return false;
    }
    // Get magnetometer calibration from AK8963 ROM
    imu.initAK8963(imu.factoryMagCalibration, mfs_opt);
    // Initialize device for active mode read of magnetometer

    imu.getAres();
    imu.getGres();
    imu.getMres();

    return true;

  }else{
    //return false if communication fails
    return false;
  }

}

/*
* Function setupMPL3115A2
* -----------------------
* configures the MPL3115A2 for altitude or pressure mode
*
* arguments
* barometer: pointer to the MPL3115A2 object
*
* option: option to begin in altitude or pressure mode
*   0 = altitude mode, ALTITUDE
*   1 = pressure mode, PRESSURE
*
* returns void
*/
void setupMPL3115A2(MPL3115A2 &barometer, int option){
  barometer.begin();
  barometer.setModeStandby();
  barometer.reset();
  delay(1000);

  if(option == ALTITUDE){
    barometer.setModeAltimeter(); // Measure altitude above sea level in meters
  }else if(option == PRESSURE){
    barometer.setModeBarometer();
  }
  barometer.setOversampleRate(7); // Set Oversample to the 2 samples, read takes 10ms
  barometer.enableEventFlags(); // Enable all three pressure and temp event flags
  barometer.setModeActive();
}

/*
* Function setupApds
* ------------------
* setup and configure the APDS9301 for light sensing
*
* arguments
* apds: pointer to APDS9301 object
*
* returns true on succesful setup, false on i2c failure
*/
bool setupApds(APDS9301 &apds){
  if(apds.begin(0x39) == APDS9301::I2C_FAILURE){
    return false;
  }
  apds.setGain(APDS9301::LOW_GAIN);
  apds.setIntegrationTime(APDS9301::INT_TIME_101_MS);
  apds.setLowThreshold(0); 
  apds.setHighThreshold(50);
  apds.setCyclesForInterrupt(1);
  apds.enableInterrupt(APDS9301::INT_OFF);
  apds.clearIntFlag();

  return true;
}

/*
* Function setupVEML6075
* ----------------------
* setup and configure the VEML6075 for UV sensing
*
* arguments
* uv: pointer to VEML6075 object
*
* returns true on succesful setup, false on i2c failure
*/
bool setupVEML6075(VEML6075 &uv){
  VEML6075_error_t ret;

  ret = uv.begin();
  if (ret != VEML6075_ERROR_SUCCESS) {
    Serial.println("no comms with VEML6075.");
    return false;
  }

  ret = uv.setIntegrationTime(VEML6075::IT_100MS);
  if (ret != VEML6075_ERROR_SUCCESS) {
    Serial.println("failed to set integration time");
    return false;
  }

  return true;
}

/*
* Function RHtoAbsolute
* ---------------------
* calculates the absolute humidity from relative humidity and temperature
*
* arguments:
* relHumidity: relative humidity in percent
* tempC: temperature in celsius
*
* returns: absolute humidty as grams per cubic meter of air
*/
double RHtoAbsolute(float relHumidity, float tempC) {
  double eSat = 6.11 * pow(10.0, (7.5 * tempC / (237.7 + tempC)));
  double vaporPressure = (relHumidity * eSat) / 100; //millibars
  double absHumidity = 1000 * vaporPressure * 100 / ((tempC + 273) * 461.5); //Ideal gas law with unit conversions
  return absHumidity;
}

/*
* Function doubleToFixedPoint
* ---------------------------
* converts double (Floating point) to a fixed point decimal point value
*
* usage note: float and double are the same thing our processor the ATmega 328p
*
* arguments:
* number: floating point number you want converted to fixed point
*
* returns: fixed point value
*/
uint16_t doubleToFixedPoint( double number) {
  int power = 1 << 8;
  double number2 = number * power;
  uint16_t value = floor(number2 + 0.5);
  return value;
}

/*
* Function getExternalTemperature
* -------------------------------
* reads temperature in celsius from the DS18B20 temperature probe
*
* usage note: will retry on first three failures
*
* arguments:
* tempsensor: pointer to DallasTemperature object
*
* returns: on success returns temperature in Celsius, on failure returns
* -127 
*/
float getExternalTemperature(DallasTemperature &tempsensor) {
  float temperature; 


  for( int i = 0; i < 3; i++ ) {

   tempsensor.requestTemperatures();
   temperature = tempsensor.getTempCByIndex(0);

   if( temperature != -127 ) {

     return temperature;

   }

  }
  
  return temperature;

}

/*
* Function sendPacket
* converts JSON packet to Message pack and sends over serial to the HM19 bluetooth chip
*
* usage note: this function takes a fair amount of stack space, if your program starts acting
* crazy this function or logData are likely culprits
*
* usage note: for now the buffer to hold your serialized packet is only 256 characters long,
* if your packet is too large it will be cut off and won't be parsed correctly on the other end
*
* arguments
* packet: pointer to the packet (DynamicJsonDocument) you wish to send
*
* returns void
*/
void sendPacket(DynamicJsonDocument &packet){
  //msgpack is a binary serialized version of json, not very human readable but
  //a lot smaller and easier on the bluetooth. We send this data over serial
  //to the Bluetooth Low Energy module
  char packetstart[] = "msgstart";
  char msgpacketbuf[180];
  memset(msgpacketbuf,0,180*sizeof(char));
  size_t buf_size = serializeMsgPack(packet, msgpacketbuf, 180*sizeof(char));
  int i = 0;
  int j = 0;
  Serial.print(packetstart);
  for(i = 0; i < buf_size; i++){
    Serial.print(msgpacketbuf[i]);
    j++;
    if(j == 100){
      j=0;
      delay(60);
    }
  }
  delay(60);
  //a note on the delays, if we don't add the 60ms delay inbetween each
  //packet the hm19 tends to overwrite individual packets. 
  //We go with a 100 byte packet assumption because that is a safe size 
  //most phones should be able to safely negotiate
}

void sendPacketEx(const char *field_ordering, DynamicJsonDocument &packet) {

  String broadcasted = "";

  JsonObject object = packet.as<JsonObject>();

  for (unsigned int i = 0; i < strlen(field_ordering); i++) {

    String ky = String(field_ordering[i]);

    if (object.containsKey(ky)) {

      JsonVariant jv = object.getMember(ky);

      broadcasted.concat(ky);

      broadcasted.concat((String(jv.as<float>(), SPKEX_DECIMAL_DIGITS_ACCY)));

      broadcasted.concat(SPKEX_ENDMARK);

    }

  }

  Serial.println(broadcasted);

}

/*
* Function logData
* converts JSON packet and sends to the SD card over i2c to be stored 
*
* usage note: this function takes a fair amount of stack space, if your program
* starts acting crazy this function or sendPacket are likely culprits.
*
* arguments
* myLog: pointer to OpenLog object
* packet: pointer to the packet (DynamicJsonDocument) you wish log 
*
* returns void
*/
void logData(OpenLog &myLog, DynamicJsonDocument &packet) {
  //json is a human readable data format, not as small as msgpack but this way
  //we can understand what is stored in the sd card easily. This data gets sent
  //over i2c to the sd card
  unsigned long old_timeout = Wire.getTimeout();
  Wire.setTimeout(100000);
  String logoutputjson = "";
  serializeJson(packet, logoutputjson);
  myLog.println(logoutputjson);
  myLog.syncFile();
  Wire.setTimeout(old_timeout);
}

/*
* Function logData
* ----------------
* writes a string to the SD card 
*
* arguments
* myLog: pointer to OpenLog object
* string: pointer to the string you wish log 
*
* returns void
*/
void logData(OpenLog &myLog, const String &string) {
  unsigned long old_timeout = Wire.getTimeout();
  Wire.setTimeout(100000);
  myLog.println(string);
  myLog.syncFile();
  Wire.setTimeout(old_timeout);
}


int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

String ReadCubeName() {
  String n = "";
  for (int i = 0; i < 20; i++) n.concat((char)EEPROM.read(i));
  n.trim();
  return n;
}
