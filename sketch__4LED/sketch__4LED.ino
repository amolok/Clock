#include <StandardCplusplus.h>
//#include <system_configuration.h>
//#include <unwind-cxx.h>
//#include <utility.h>

#include <Arduino.h>
#include <vector>
//#include <TM1637Display.h>
#include "display.h"
//#include "font.h"

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   100

Display4LED2 disp;

//FNT F;

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Internal Temperature Sensor"));
  int k;
  uint8_t D[] = { 0xff, 0xff, 0xff, 0xff };
  // All segments on
  disp.setSegments(D);
  delay(TEST_DELAY);
  D[0]=B00111001;
  D[1]=B10001001;
  D[2]=B00001001;
  D[3]=B00001111;
  disp.setSegments(D);
//  disp.update();
  
//  while(true) {
    for(k = 4; k < 16; k++) {
      disp.setBrightness(k); 
      disp.setSegments(D);
      delay(TEST_DELAY); 
    }
    for(k = 15; k >= 4; k--) {
      disp.setBrightness(k); 
      disp.setSegments(D);
      delay(TEST_DELAY); 
    }
//  }
  disp.setBrightness(0x08);
}

void loop()
{
  disp._DD(0,(int)GetTemp());
  disp._hold(2, B01100011);
  disp._hold(3, B00000000);
  disp.update();
  delay(250);
}


double GetTemp(void)
{
  unsigned int wADC;
  double t;

//  digitalWrite(ledG, HIGH);

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA, ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celcius.
  return (t);
}

