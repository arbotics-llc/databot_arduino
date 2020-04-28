String NAMED = String("Yellow");

#include <databot.h>

void setName() {

  String nameCmd = F("AT+NAMEdatabot");

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
