#include <databot.h>

#define UINTMAX 653535

const byte adcPin = 0;
volatile unsigned int adcReading = 0;
volatile boolean adcDone;
volatile unsigned int lowSample = UINTMAX;
volatile unsigned int highSample = 0;

#define samplingFrequency 8000
#define samples 128

int fftindex = 0;
float vReal[samples];
float vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

DynamicJsonDocument packet(100);

arduinoFFT FFT = arduinoFFT();

void setup() {
  // put your setup code here, to run once:
  
  enableADCINT();
  
  Serial.begin(9600);

  FlexiTimer2::set(1, 1.0/samplingFrequency, startADCRead); // *samplingFrequency* hz
  FlexiTimer2::start();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(adcDone == true){
    adcDone = false;
    
    if(fftindex<samples){
      vReal[fftindex] = adcReading;
      vImag[fftindex] = 0;
      fftindex++;
    }else{
      cli();
      FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
      FFT.ComplexToMagnitude(vReal, vImag, samples);
      float biggest = 0;
      int biggest_i = 0;
      for(int i = 2; i < (samples >> 1); i++){
        if(vReal[i] > biggest){
          biggest = vReal[i];
          biggest_i = i;
        }
      }

      float peakToPeak = (highSample - lowSample)* 3.3 / 1024;
      float fundamentalFreq = (biggest_i * 1.0 * samplingFrequency) / samples;

      if(peakToPeak > 0.13){
        packet[F("mic freq")] = fundamentalFreq;
      }else{
        packet[F("mic freq")] = (float)0;
      }

      packet[F("mic amp")] = peakToPeak;
      packet[F("time")] = millis();
      sendPacket(packet);
      
      fftindex=0;
      lowSample = UINTMAX;
      highSample = 0;
      sei();
    }
  
  }
    
}

void enableADCINT(){
  cli(); //stops any interrupts
  
  ADMUX = 0;            // measure from pin 0
  ADMUX |= bit(REFS0);  // set AVCC as voltage reference
  ADMUX |= bit(ADLAR);  // use 8 bit resolution

  ADCSRA |= bit(ADPS2) | bit(ADPS1) | bit(ADPS0); // 128 prescale, 16mhz / 128 = 104us reads, slowest option
  ADCSRA |= bit(ADEN);  // turn on ADC
  ADCSRA |= bit(ADIE) | bit(ADSC);  // turn on Interrupts and start one ADC conversion

  sei();  //enables interrupts again
}

void startADCRead()
{
  ADCSRA |= bit(ADSC);
}

ISR(ADC_vect) {
  byte low;
  byte high;

  // must read low bytes first, this keeps low and high bytes synced
  low = ADCL >> 6;
  high = ADCH;

  adcReading = (high << 2) | low;

  if(adcReading > highSample){
    highSample = adcReading;
  }
  if(adcReading < lowSample){
    lowSample = adcReading;
  }

  adcDone = true;

}
