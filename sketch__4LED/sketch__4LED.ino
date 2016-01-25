#include <StandardCplusplus.h>
//#include <system_configuration.h>
//#include <unwind-cxx.h>
//#include <utility.h>

#include <Arduino.h>
// #include <vector>
//#include <TM1637Display.h>
#include "settings.h"
#include "font.h"
#ifndef FNT
FNT F;
#endif
#include "display.h" 
#ifndef D
Display4LED2 D;
#endif
// #include "clock.h" 
#include "machine.h"

uint8_t Hour=      15;
uint8_t Minute=    6;
uint8_t Second=    1;
uint8_t Day=       3;
uint8_t DayofWeek= 3;   // Sunday is day 0
uint8_t Month=     1;   // Jan is month 0
uint8_t Year=2016-1900; // the Year minus 1900

Settings.Day.Hour=9;
Settings.Day.Minute=0;
Settings.Night.Hour=21;
Settings.Night.Minute=0;

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   100


void setup()
{
  Serial.begin(9600);
  Serial.println(F("Internal Temperature Sensor"));
  int k;
  uint8_t _D[] = { 0xff, 0xff, 0xff, 0xff };
  // All segments on
  D.setSegments(_D);
  delay(TEST_DELAY);
  _D[0]=B00111001;
  _D[1]=B10001001;
  _D[2]=B00001001;
  _D[3]=B00001111;
  D.setSegments(_D);
//  D.update();
  
//  while(true) {
    for(k = 4; k < 16; k++) {
      D.setBrightness(k); 
      D.setSegments(_D);
      delay(TEST_DELAY); 
    }
    for(k = 15; k >= 4; k--) {
      D.setBrightness(k); 
      D.setSegments(_D);
      delay(TEST_DELAY); 
    }
//  }
  D.setBrightness(0x08);
}

int prevTemp=0;
int _c=0;

void showTemp(){
  // FNT F;
  int temp=(int)GetTemp();
  D._DD(0,temp);
  if(temp!=prevTemp){
    if(temp>prevTemp){
      D._ab(2,F.Sensor.Temp.rise);
    }else{
      D._ab(2,F.Sensor.Temp.fall);
    }
    prevTemp=temp;
  }else {
  D._hold(2, F.Sensor.Temp.sign);
  }
  
  D._hold(3, F.blank);

  for(uint8_t i=0;i<4;i++){
  D.update();
  delay(250);
  }  
}

void showTime(){
  // FNT F;
  D._DD(0,12);
  D._DD(2,34);
  D.blink(1,F.dot);
  
  for(uint8_t i=0;i<4;i++){
  D.update();
  delay(250);
  }  
}

void loop()
{
//  if(_c++ /3%2==0){
    showTemp();
//  }else{
//    showTime();
//  }
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

