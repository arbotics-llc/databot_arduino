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

#ifndef DATABOT_H
#define DATABOT_H

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
//MPU9250 accelerometer/gyroscope/magnetometer helper functions
bool isImuReady(MPU9250 &imu);
void updateImuAcceleration(MPU9250 &imu);
void updateImuGyro(MPU9250 &imu);
void updateImuMag(MPU9250 &imu);
bool setupIMU(MPU9250 &imu, uint8_t sample_rate, uint8_t afs_opt, uint8_t gfs_opt, uint8_t mfs_opt);

#define ALTITUDE 0
#define PRESSURE 1
//MPL3115A2 altitude/pressure sensor helper function
void setupMPL3115A2(MPL3115A2 &barometer, int option);

//APDS9301 ambient light sensor helper function
bool setupApds(APDS9301 &apds);

//VEML6075 uv sensor helper function
bool setupVEML6075(VEML6075 &uv);

//SHTC3 humidity sensor helper functions
double RHtoAbsolute(float relHumidity, float tempC);
uint16_t doubleToFixedPoint( double number);

//bluetooth low energy and logging communication helper functions
void sendPacket(DynamicJsonDocument &packet);
void logData(OpenLog &myLog, DynamicJsonDocument &packet);

#endif
