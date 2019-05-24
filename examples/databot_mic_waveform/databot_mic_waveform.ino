#include <databot.h>

volatile unsigned int adcReading = 0;
volatile boolean adcDone;

#define samplingFrequency 8000

void setup() {
  // put your setup code here, to run once:

  enableADCINT();

  Serial.begin(115200);

  FlexiTimer2::set(1, 1.0/samplingFrequency, startADCRead); // *samplingFrequency* hz
  FlexiTimer2::start();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(adcDone == true){
    adcDone = false;

    Serial.println(adcReading * 3.3 / 100); //1/10's of a volt looks better on plotter
    
  }
    
}

void enableADCINT() {
  cli(); //stops any interrupts
  
  ADMUX = 0;            // measure from pin 0
  ADMUX |= bit(REFS0);  // set AVCC as voltage reference
  ADMUX |= bit(ADLAR);  // use 8 bit resolution

  ADCSRA |= bit(ADPS2) | bit(ADPS1) | bit(ADPS0); // 128 prescale, 16mhz / 128 = 104us reads, slowest option
  ADCSRA |= bit(ADEN);  // turn on ADC
  ADCSRA |= bit(ADIE) | bit(ADSC);  // turn on Interrupts and start one ADC conversion

  sei();  //enables interrupts again
}

void startADCRead() {
  ADCSRA |= bit(ADSC); 
}

ISR(ADC_vect) {
  byte low;
  byte high;

  // must read low bytes first, this keeps low and high bytes synced
  low = ADCL >> 6;
  high = ADCH;

  adcReading = (high << 2) | low;

  adcDone = true;

}
