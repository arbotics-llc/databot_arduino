#include <databot.h>

// Melody liberated from the toneMelody Arduino example sketch by Tom Igoe.
int melody[] = { 262, 196, 196, 220, 196, 0, 247, 262 };
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };

void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(A3, OUTPUT);
  pinMode(13, OUTPUT);

}

void loop() {
  digitalWrite(A3, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  for (unsigned long freq = 125; freq <= 3500; freq += 10) {  
    NewTone(9, freq, 5); // Play the frequency (125 Hz to 15 kHz sweep in 10 Hz steps) for 1ms.
    delay(5);
  }
  noNewTone();

  delay(500); // Wait half a second 
  digitalWrite(A3, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(500); // Wait half a second

  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000/noteDurations[thisNote];
    NewTone(9, melody[thisNote], noteDuration); 
    delay(noteDuration * 4 / 3); 
  }
  noNewTone();

  delay(500);

}
