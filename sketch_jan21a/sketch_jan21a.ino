
// Internal Temperature Sensor
// Example sketch for ATmega328 types.
//
// April 2012, Arduino 1.0

#define ledG 13
#define ButtonPIN 7
#define BuzzerPIN 8
void setup()
{
  Serial.begin(9600);
  Serial.println(F("Internal Temperature Sensor"));
  pinMode(ledG, OUTPUT);
  pinMode(BuzzerPIN, OUTPUT);
  pinMode(ButtonPIN, INPUT);
  for(int i=0;i<100;i++){
  digitalWrite(ledG, HIGH);
  delay(10);
  digitalWrite(ledG, LOW);
  delay(1);
  }
  delay(1000);
  analogWrite(ledG, 32);
  delay(1000);
}

void loop()
{
  // Show the temperature in degrees Celcius.
  //  Serial.print("t=");
  int b;
  while(b=digitalRead(ButtonPIN) == HIGH){
  Serial.println(GetTemp(), 1);
  digitalWrite(ledG,b);
  for(int i=0;i<100;i++){
    digitalWrite(BuzzerPIN,HIGH);
    delay(1);
    digitalWrite(BuzzerPIN,LOW);
    delay(1);
  }
  delay(1000);
  }
  digitalWrite(ledG,b);
//  if(b==HIGH){}
//  Serial.println(GetTemp(), 1);
  //  Serial.println("°C");
//  digitalWrite(ledG, LOW);
  delay(100);
  //  digitalWrite(ledG,HIGH);
  //  delay(500);
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

