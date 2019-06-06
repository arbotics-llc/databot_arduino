#include <databot.h>
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(A3, OUTPUT);
  pinMode(13, OUTPUT);
  } // just start playing!


void loop() {
  digitalWrite(A3, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  for (unsigned long freq = 125; freq <= 15000; freq += 10) {  
    NewTone(9, freq, 1); // Play the frequency (125 Hz to 15 kHz sweep in 10 Hz steps) for 1ms.
    delay(1);
  }
  noNewTone();

  delay(500); // Wait a second.
  digitalWrite(A3, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(500); // Wait a second.
}