#include <databot.h>

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(A3, OUTPUT);
  pinMode(13, OUTPUT);

  for(int i = 0; i < 3; i ++){

    digitalWrite(A3, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    for (unsigned long freq = 125; freq <= 15000; freq += 10) {  
      NewTone(9, freq, 1); // Play the frequency (125 Hz to 15 kHz sweep in 10 Hz steps) for 1ms.
      delay(1);
    }
    noNewTone();

    //sleep for 500 milliseconds
    //note: a call to delay(500); would accomplish the same thing here
    //but sleeping saves battery. If timing isn't critical then we can
    //use sleep mode instead
    LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);

    digitalWrite(A3, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW

    //sleep for 500 milliseconds
    LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);
  }

}

void loop() {
  digitalWrite(A3, HIGH); 
  digitalWrite(13, HIGH);

  //sleep for one second
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

  digitalWrite(A3, LOW);
  digitalWrite(13, LOW);

  //sleep for one second
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

}
