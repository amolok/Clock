#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   300

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

TM1637Display display(CLK, DIO);

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Internal Temperature Sensor"));
  int k;
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  display.setBrightness(0x0f);
  // All segments on
  display.setSegments(data);
  delay(TEST_DELAY);
  data[0]=B00111001;
  data[1]=B10001001;
  data[2]=B00001001;
  data[3]=B00001111;
  display.setSegments(data);
  
//  while(true) {
    for(k = 4; k < 16; k++) {
      display.setBrightness(k); 
      display.setSegments(data);
      delay(TEST_DELAY); 
    }
    for(k = 15; k >= 4; k--) {
      display.setBrightness(k); 
      display.setSegments(data);
      delay(TEST_DELAY); 
    }
//  }
}

void loop()
{
  uint8_t D[] = {0xff,0xff,0xff,0xff};
  int k;
  display.setBrightness(0x0f);
  // Serial.println(GetTemp(), 1);
  display.showNumberDec((int)GetTemp(),false,2,0);
  D[0]=B01100011;
  D[1]=B00000000;
  display.setSegments(D,2,2);
  delay(100);
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



