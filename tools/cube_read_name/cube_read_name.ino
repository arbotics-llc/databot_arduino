#include <databot.h>

void setup() {

  Serial.begin(9600);

  delay(125);

  Serial.flush();

}

static bool displayed_once = false;

void loop() {

  if (!displayed_once) Serial.println(ReadCubeName());

  displayed_once = true;

}
