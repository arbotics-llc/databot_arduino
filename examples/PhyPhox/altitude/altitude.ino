#include <databot.h>

// Start by finding your altitude from https://www.freemaptools.com/elevation-finder.htm or similar, then convert to meters
#define CURRENT_ALTITUDE_METERS 103.3

#define I2Cclock 400000
#define I2Cport Wire

MPL3115A2 barometer;

DynamicJsonDocument packet(200);

unsigned long timestamp;

void setup() {

  //setup i2c and set clock speed to 40khz
  Wire.begin();
  Wire.setClock(I2Cclock);

  pinMode(A3, OUTPUT);
  pinMode(13, OUTPUT);

  digitalWrite(A3, LOW);
  digitalWrite(13, LOW);

  Serial.begin(9600);
  
  setupMPL3115A2(barometer, ALTITUDE);

  barometer.trimAltitudeMeters(CURRENT_ALTITUDE_METERS);
  
  Serial.flush();

  delay(1000);  // pause while all sensors finish starting up

  timestamp = millis();

}

void loop() {
    
    // Place the altitude into field "A", and timestamp into field "b", as that's the
    // data exchange format by the packet sending code
    
    packet[F("a")] = barometer.readAltitude();

    packet[F("b")] = millis() - timestamp;  // relative to startup
    
    sendPacketFixedStringsFormat(packet);

    delay(500);

}

// Sample .phyphox file. Load this file into PhyPhox's editor (at https://phyphox.org/editor) to generate an online QR
// code & scan using your device's camera. Alternatively, scan the QR code at http://databot.us.com within the PhyPhox app
/*

<phyphox xmlns="http://phyphox.org/xml" xmlns:editor="http://phyphox.org/editor/xml" version="1.7" editor:version="1.0" locale="en">
    <title>databot</title>
    <category>aRbotics</category>
    <description>PhyPhox inspired altitude sample</description>
    <data-containers>
        <container size="0">AL</container>
        <container size="0">MS</container>
    </data-containers>
    <input>
        <bluetooth editor:uuid="97" id="HMSoftdatabot" name="HMSoftdatabot" uuid="0000ffe0-0000-1000-8000-00805f9b34fb" mode="notification" subscribeOnStart="false">
            <output char="0000ffe1-0000-1000-8000-00805f9b34fb" conversion="string" offset="0" length="13">AL</output>
            <output char="0000ffe1-0000-1000-8000-00805f9b34fb" conversion="string" offset="14" length="13">MS</output>
        </bluetooth>
    </input>
    <output>
    </output>
    <analysis sleep="0" onUserInput="false">
    </analysis>
    <views>
        <view label="Raw Data">
            <graph label="Pressure" labelX="Time (ms)" unitX="MS" labelY="Altitude (m)" unitY="AL" yPrecision="6" partialUpdate="true">
                <input axis="x">MS</input>
                <input axis="y">AL</input>
            </graph>
        </view>
    </views>
    <export>
        <set name="databot">
            <data name="Time (ms)">MS</data>
            <data name="Altitude (m)">AL</data>
        </set>
    </export>
</phyphox>

*/
