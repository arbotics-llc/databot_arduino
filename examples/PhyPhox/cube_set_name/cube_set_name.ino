String NAMED = String("databot");
//String NAMED = String("Midnight");
//String NAMED = String("Grape");
//String NAMED = String("Royal");
//String NAMED = String("Teal");
//String NAMED = String("Plum");
//String NAMED = String("Lime");
//String NAMED = String("Lemon");
//String NAMED = String("Orange");
//String NAMED = String("Grass");
//String NAMED = String("Apple");

#include <databot.h>

void setName() {

  String nameCmd = F("AT+NAMEDB_");

  nameCmd += NAMED;

  Serial.println(nameCmd);

  delay(80);

  Serial.println("AT+RESET");

  delay(125);

  Serial.flush();

  delay(125);

}

void setup() {

  Serial.begin(9600);

  delay(125);

  Serial.flush();

  delay(200);

  setName();

  Serial.println("Naming as \"" + NAMED + "\"");

}

void loop() {}
