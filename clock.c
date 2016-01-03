/*
Code for https://123d.circuits.io/circuits/1421761-fun-with-74hc595/edit
*/
// A#
#define lightPin 0
#define  tempPin 1
// D#
#define  dataPin 10 // DS
#define latchPin 11 // STCP
#define clockPin 12 // SHCP
#define ledG 13
#define ledR 6
// INT
#define timerPin 2
#define buttonPin 7
// #define clockPin 2
// #define buttonPin 3
volatile int button_state = LOW;
// sensors
#define normalize(a,min,max) (byte)((word)((a-min)*100)/(max-min))
#define LightSensorMin 2
#define LightSensorMax 381
#define normLight(l) normalize(l,LightSensorMin,LightSensorMax)
#define TempSensorMin    20 // -40
#define TempSensorMinT  -40
#define TempSensorMax   358 // 125
#define TempSensorMaxT  125
#define normTemp(t) (signed int)((float)(t-TempSensorMin)/(TempSensorMax-TempSensorMin)*(TempSensorMaxT-TempSensorMinT)+TempSensorMinT)
// Fonts
#define __A B01000000
#define __B B00100000
#define __C B00010000
#define __D B00001000
#define __E B00000100
#define __F B00000010
#define __G B00000001
#define __p B10000000
#define FNT_blank B11111111
#define FNT_minus B11111110
#define FNT_dot ~(__p)
#define FNT_celsius B11001110
#define FNT_grad B10011100
#define FNT_week ~B00000110
const byte FNT_Digit[10] =
{// ABCDEFG
  B10000001, //0
  B11001111, //1
  B10010010, //2
  B10000110, //3
  B11001100, //4
  B10100100, //5
  B10100000, //6
  B10001111, //7
  B10000000, //8
  B10000100  //9
};
const byte FNT_Alphabet[26]=
{
  ~B01110111, // A
  ~B00011111, // b
  ~B01001110, // C
  ~B00111101, // d
  ~B01001111, // E
  ~B01000111, // F
  ~B01011110, // G
  ~B00110111, // H
  ~B00110000, // I
  ~B00111000, // J
  ~B00000111, // k
  ~B00001110, // L
  ~B00001001, // m
  ~B00010101, // n
  ~B00011101, // o
  ~B01100111, // p
  ~B01110011, // q
  ~B00000101, // r
  ~B01011011, // s
  ~B00001111, // t
  ~B00011100, // u
  ~B00111110, // v
  ~B00111111, // w
  ~B00010011, // x
  ~B00110011, // Y
  ~B01101101  // z
};
const byte FNT_WeekDays_rus[8]=
{ // ом бр яп вр ор яа бя = абмопярв
  ~B01011111, // а
  ~B01111111, // б
  ~B00110111, // м
  ~B01110110, // о
  ~B01100111, // п
  ~B01001110, // я
  ~B01110000, // р
  ~B00110011  // в
};

#define Sensors 6
#define      TempSensor_id 0
#define  HumiditySensor_id 1
#define  PressureSensor_id 2
#define     LightSensor_id 3
#define     SoundSensor_id 4
#define ProximitySensor_id 5
const byte FNT_Sensor[Sensors]= // T, H, P, L, S, o // Temp, Humidity, Pressure, Light, Sound, Proximity
{
  B01110000, // t
  B01001000, // H
  B00011000, // P
  B01110001, // L
  B00100100, // S
  B01100010  // o
};
const byte FNT_LightSensorAnimation[3]=
{
  B11110111,
  B11110110,
  B10110110
};
/*
struct SEN
{
  char id,// THPLSo
  int cur, // current
  int pre, // previous
  int avg[10], // data array for avarage
  int frq, // frequency of requests
};
struct SENque
{
  int lastUpdate, // last update time
  int  timer[Sensors], // timer que
  // char sensor[Sensors], // sensors que
};
*/
// functions

// ANIMATION
// byte D[4];
struct Animation
{
  // char from; // state_A
  // char to; // state_B
  // byte A;
  // byte AA;
  // byte BB;
  // byte B;
  byte AB[4];
  byte pos; // A AA BB B
  byte Active; // 
};

Animation animate;

void init_animation(){
  // animate.from=0;
  // animate.to=0;
  animate.pos=0;
  animate.Active=0;
}

// .ABCDEFG

byte animation__shift_U(byte X){
  // C->B, E->F, G->A, D->G
  return (X & __C)<<1 | (X & __E)>>1 | (X & __G)<<6 | (X & __D)>>3 ;
}
byte animation__shift_D(byte X){
  // B->C, F->E, A->G, G->D
  return (X & __B)>>1 | (X & __F)<<1 | (X & __A)>>6 | (X & __G)<<3 ;
}
byte animation__shift_R(byte X){
  // E->C, F->B
  return (X & __E)<<2 | (X & __F)<<4 ;
}
byte animation__shift_L(byte X){
  // C->E, B->F
  return (X & __C)>>2 | (X & __B)>>4 ;
}

void animation_rotUp(byte A, byte B){
  // BA->D
  animate.AB[0]=A;
  animate.AB[1] = animation__shift_U(A) | (B & __A)>>3 ;
  // + BB->C, BF->E, BG->D
  animate.AB[2] = animation__shift_U(animate.AB[1]) | (B & __B)>>1 | (B & __F)<<1 | (B & __G)<<3;
  animate.AB[3]=B;
}
void animation_rotDown(byte A, byte B){
  // BD->A
  animate.AB[0]=A;
  animate.AB[1] = animation__shift_D(A) | (B & __D)<<3 ;
  // + BC->B, BE->F, BG->A
  animate.AB[2] = animation__shift_D(animate.AB[1]) | (B & __C)<<1 | (B & __E)>>1 | (B & __G)<<6;
  animate.AB[3]=B;  
}


// globals
unsigned long start;
// uint16_t start;
volatile byte Tic=0;

volatile byte Hour=      15;
volatile byte Minute=    6;
volatile byte Second=    1;
volatile byte Day=       3;
volatile byte DayofWeek= 3; // Sunday is day 0
volatile byte Month=     1;     // Jan is month 0
volatile byte Year=2016-1900;      // the Year minus 1900
// display
byte D[4]={0,0,0,0}; // Display global

void display(byte DISP[4]){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, DISP[3]);
  shiftOut(dataPin, clockPin, LSBFIRST, DISP[2]);
  shiftOut(dataPin, clockPin, LSBFIRST, DISP[1]);
  shiftOut(dataPin, clockPin, LSBFIRST, DISP[0]);
  digitalWrite(latchPin, HIGH);
}
void print_sens(byte id, signed int x){
  char string[5];
  byte disp[4];
  sprintf(string, "%4d", x);
  Serial.println(x);
  for(byte i=0;i<4;i++){
    if((string[i] >= '0')&&(string[i] <='9')) disp[i]=FNT_Digit[(byte)string[i]-'0'];
    else if(string[i]=='-') disp[i]=FNT_minus;
    else
      disp[i]=FNT_blank;
  }
  disp[0]=FNT_Sensor[id];
  display(disp);
}
void disp_light(int l){
  byte disp[4];
  byte t = normLight(l);
  Serial.print("L: ");
  Serial.print(t);
  Serial.println("%%");
/*  if(t<25){
    disp[1]=FNT_LightSensorAnimation[0];
    disp[2]=FNT_LightSensorAnimation[0];
    disp[3]=FNT_LightSensorAnimation[0];
  }
  if(t<5){
    disp[1]=FNT_dot;
  }
  if(t<10){
    disp[2]=FNT_dot;
  }
  if(t<15){
    disp[3]=FNT_dot;
  }
  if(t>=25){
    disp[1]=FNT_LightSensorAnimation[0];
    disp[2]=FNT_LightSensorAnimation[0];
    disp[3]=FNT_LightSensorAnimation[0];
  }
  if(t>30){
    disp[3]=FNT_LightSensorAnimation[1];
  }
  if(t>40){
    disp[2]=FNT_LightSensorAnimation[1];
  }
  if(t>50){
    disp[3]=FNT_LightSensorAnimation[2]; // 123
  }
  if(t>75){
    disp[2]=FNT_LightSensorAnimation[2];
  }
  if(t>90){
    disp[1]=FNT_LightSensorAnimation[2];
    disp[2]=FNT_LightSensorAnimation[2];
    disp[3]=FNT_LightSensorAnimation[2];
  }
*/
  disp[0]=FNT_Sensor[LightSensor_id];
  disp[1]=FNT_blank;
  disp[2]=FNT_blank;
  disp[3]=FNT_blank;
  if(t>5)disp[1]=FNT_LightSensorAnimation[0];
  if(t>33)disp[2]=FNT_LightSensorAnimation[1];
  if(t>66)disp[3]=FNT_LightSensorAnimation[2];
  if(t>95)disp[2]=FNT_LightSensorAnimation[2];
  display(disp);
}
/*
void disp_temp(int t){
  byte disp[4];
  byte t = normLight(l);
  Serial.print("T= ");
  Serial.print(t);
  disp[0]=FNT_Sensor[LightSensor_id];
}
*/

// demo
void demo_light(){
  int l=analogRead(lightPin);
  // Serial.print("L=");
  // Serial.println(l);
  analogWrite(ledR, l/4);
  // disp_light(l);
  print_sens(LightSensor_id, normLight(l));
  // print_sens(LightSensor_id, l);
}
void demo_temp(){
  signed int t=analogRead(tempPin);
  Serial.print("T=");
  Serial.print(analogRead(tempPin));
  Serial.print(" = ");
  Serial.println(normTemp(t));
  print_sens(TempSensor_id, normTemp(t));
}
void demo_time(){
  byte disp[4]={FNT_minus,FNT_minus&FNT_dot,FNT_minus,FNT_minus};
  char string[5];
  unsigned long t = millis()-start;
  // uint16_t t = start- (uint16_t)millis();
  // uint16_t t = now();
  Serial.println(t);
  sprintf(string, "%4d", t/1000);
  // if(t%100==0)tic();
  for(byte i=0;i<4;i++){
    if((string[i] >= '0')&&(string[i] <='9')) disp[i]=FNT_Digit[(byte)string[i]-'0'];
    else
      disp[i]=FNT_Digit[0];
  }
  display(disp);
}
void demo_alphabet(){
  for(byte i=0;i<sizeof(FNT_Alphabet);i++){
    D[0]=FNT_Alphabet[i];
    D[1]=((i>sizeof(FNT_Alphabet)+1)-1)&FNT_Alphabet[i+1];
    D[2]=((i>sizeof(FNT_Alphabet)+2)-1)&FNT_Alphabet[i+2];
    D[3]=((i>sizeof(FNT_Alphabet)+3)-1)&FNT_Alphabet[i+3];
    display(D);
    if(button_state==HIGH)return; // !!!: is it correct?
    delay(500);
  }
}
void demo_animation(){
  for(byte i=0;i<10;i++){
    // D[1]=FNT_Digit[i+1];
    D[2]=FNT_blank;
    animation_rotUp(~FNT_Digit[i],~FNT_Digit[(i+1)%10]);
    // animation_rotUp(~FNT_minus,~FNT_Digit[0]);
    // animation_rotUp(~FNT_Digit[i],~FNT_minus);
    D[0]=~animate.AB[0];
    // D[1]=~animation__shift_U(~D[0]);
    D[1]=~animate.AB[3];
    // D[1]=~animate.AB[1];
    // D[3]=~animate.AB[3];
    // D[2]=~animation__shift_D(~D[3]);
    // D[2]=~animate.AB[2];
    for (int i = 0; i < 4; ++i) {D[3]=~animate.AB[i]; display(D); delay(250);}
    display(D);
    delay(1000);
  }
}

// DISPLAY buffer
void fill_SS(){
  D[0]=FNT_blank;
  D[1]=FNT_blank;
  D[2]=FNT_Digit[Second/10];
  D[3]=FNT_Digit[Second%10];
}
void fill_HHMM(){
  D[0]=FNT_Digit[Hour/10];
  D[1]=FNT_Digit[Hour%10];
  D[2]=FNT_Digit[Minute/10];
  D[3]=FNT_Digit[Minute%10];
  if(Tic<5)D[1]=D[1]&FNT_dot;
}
void fill_MMSS(){
  D[0]=FNT_Digit[Minute/10];
  D[1]=FNT_Digit[Minute%10];
  D[2]=FNT_Digit[Second/10];
  D[3]=FNT_Digit[Second%10];
  D[1]=D[1]&FNT_dot;
}
void fill_ddmm(){
  D[0]=FNT_Digit[Day/10];
  D[1]=FNT_Digit[Day%10];
  D[2]=FNT_Digit[Month/10];
  D[3]=FNT_Digit[Month%10];
  D[1]=D[1]&FNT_dot;
}
void fill_mmYY(){
  D[0]=FNT_Digit[Month/10];
  D[1]=FNT_Digit[Month%10];
  D[2]=FNT_Digit[Year/10%10];
  D[3]=FNT_Digit[Year%10];
  D[1]=D[1]&FNT_dot;
}
void fill_YYYY(){
  D[0]=FNT_Digit[(Year+1900)/1000];
  D[1]=FNT_Digit[(Year+1900)/100%10];
  D[2]=FNT_Digit[(Year+1900)/10%10];
  D[3]=FNT_Digit[(Year+1900)%10]&FNT_dot;
}
void fill_W(){
  // DayofWeek 0-6 // 1 = Monday
  // ом бр яп, вр ор - , - яа бя
  if(Tic/4==0){
    D[0]= ~((((DayofWeek==1)-1)&__A)|(((DayofWeek==2)-1)&__G)|(((DayofWeek==3)-1)&__D));
    D[1]= ~((((DayofWeek==4)-1)&__A)|(((DayofWeek==5)-1)&__G));
    D[2]= ~((((DayofWeek==6)-1)&__G)|(((DayofWeek==0)-1)&__D));
    D[3]= FNT_week;
  }else{
    // D[0]= FNT_Digit[DayofWeek]|FNT_dot;
    D[0]= ~(__A|__G|__D);
    D[1]= ~(__A|__G);
    D[2]= ~(__G|__D);
    D[3]= FNT_week;
  }
}
void fill_weekday(){
  // абмопярв
  // 01234567
  D[0]=FNT_blank;
  D[1]=FNT_blank;
  switch(DayofWeek){
    case 1:
    D[2]=FNT_WeekDays_rus[3];
    D[3]=FNT_WeekDays_rus[2];
    break;
    case 2:
    D[2]=FNT_WeekDays_rus[1];
    D[3]=FNT_WeekDays_rus[6];
    break;
    case 3:
    D[2]=FNT_WeekDays_rus[5];
    D[3]=FNT_WeekDays_rus[4];
    break;
    case 4:
    D[2]=FNT_WeekDays_rus[7];
    D[3]=FNT_WeekDays_rus[6];
    break;
    case 5:
    D[2]=FNT_WeekDays_rus[3];
    D[3]=FNT_WeekDays_rus[6];
    break;
    case 6:
    D[2]=FNT_WeekDays_rus[5];
    D[3]=FNT_WeekDays_rus[0];
    break;
    case 0:
    D[2]=FNT_WeekDays_rus[1];
    D[3]=FNT_WeekDays_rus[5];
    break;
    default:
    D[2]=FNT_minus;
    D[3]=FNT_minus;
  }
}


void fill_currentTime(){
  char s[5];
  sprintf(s,"%2d",Second);
  D[0]=FNT_Digit[s[0]-'0'];
  D[1]=FNT_Digit[s[1]-'0'];
  D[2]=FNT_Digit[s[2]-'0'];
  D[3]=FNT_Digit[s[3]-'0'];
  // sprintf(s,"%2d",Minute);
  // D[2]=s[0];
  // D[3]=s[1];
  if(Tic/2==0){
    D[1]&FNT_dot;
  }
  Serial.println(Second);
}

// CLOCK
// #define ticHz 490 // 490Hz
#define ticHz 10 // 490Hz
volatile word tic_tac=0; // 49 = 10Hz

void tic(){
  // if(tic_tac/2==0){
  if(++Tic==10)Tic=0;
  // }
  if(++tic_tac>=ticHz){
    tic_tac=0;
    if(++Second>=60){Second=0;
      if(++Minute>=60){Minute=0;
        if(++Hour>=24){Hour=0;
          if(++DayofWeek>=7){DayofWeek=0;
          } if(++Day>31){Day=0;
            if(++Month>=12){Month=0;
              ++Year;
            } } } } } }
          }


// BUTTON

          void buttonINT() {
            button_state = !button_state;
          }

// states
          char state=0;
// setup
          void setup(){
            start = millis();
  Serial.begin(9600);  //Begin serial communcation
  Serial.print("SETUP: ");
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  digitalWrite(ledR, HIGH);
  digitalWrite(ledG, HIGH);
  delay(100);
  digitalWrite(ledR, LOW);
  Serial.print(".");
  pinMode(buttonPin, OUTPUT);
  // pinMode(buttonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonINT, CHANGE);
  attachInterrupt(digitalPinToInterrupt(timerPin), tic, CHANGE);
  Serial.print(".");
  delay(100);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  digitalWrite(latchPin,LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, B01111110);
  shiftOut(dataPin, clockPin, LSBFIRST, B01111110);
  shiftOut(dataPin, clockPin, LSBFIRST, B01111110);
  shiftOut(dataPin, clockPin, LSBFIRST, B01111110);
  digitalWrite(latchPin,HIGH);
  Serial.print(".");
  delay(100);
  {
    for(signed char i=9;i>=0;i--){
      // digitalWrite(ledR, HIGH);
      D[0]=FNT_Digit[i];
      D[1]=FNT_Digit[i];
      D[2]=FNT_Digit[i];
      D[3]=FNT_Digit[i];
      display(D);
      // delay(50);
      // digitalWrite(ledR, LOW);
      delay(50);
    }
  }
  Serial.print(".");
  // delay(1000);
  init_animation();
  Serial.print(".");
  digitalWrite(ledR, LOW);
  Serial.println(" COMPLETE");
  digitalWrite(ledG, LOW);
  state=0;
  // start = (uint16_t)millis();
  // Serial.print(start-millis());
  Serial.print(tic_tac);
  Serial.println(":");
}

// States
#define States 5
const char states[States]= {'s','a','w','W','Y'};
// const char states[States]= {'c','W','d','m','Y','s'};

// main loop

void fillState(char s){
  switch(states[s]){
    case 'L': demo_light(); break;
    case 'T': demo_temp(); break;
    case 't': demo_time(); break;
    case 'a': demo_animation(); break;
    // case 'a': demo_alphabet(); break;
    case 'c': fill_HHMM(); break;
    case 'M': fill_MMSS(); break;
    case 'd': fill_ddmm(); break;
    case 'W': fill_W(); break;
    case 'w': fill_weekday(); break;
    case 'Y': fill_YYYY(); break;
    case 's': fill_SS(); break;
    default: fill_SS();
  }
}

void loop(){
  tic();
  if(states[state]=='s'){
    if(!animate.Active){
      if(Tic==0){
            animate.Active=true;
            digitalWrite(ledR,LOW);
            fill_SS();
            animation_rotUp(~FNT_Digit[0],~FNT_Digit[1]);
            // animation_rotUp(~FNT_Digit[Second%10],~FNT_Digit[Second%10+1]);
            animate.pos=0;
          }
    }else{
      D[3]=~animate.AB[animate.pos];
      D[0]=FNT_Digit[animate.pos];
      if(Tic%3==1){ // 0 1 2 , 3 4 5, 7 8 9
        if(animate.pos++>4){animate.Active=false;}
      }
      // D[3]=~animate.AB[Tic/3];
    }
    display(D);
  }
  
  if(Tic==0){
    digitalWrite(ledG, HIGH);
    if(++DayofWeek>=7)DayofWeek=0;
    fillState(state);
    display(D); 
    delay(100);
  }
  // Serial.println("looping...");
  // Serial.println(tic_tac);

  if(button_state==HIGH){
    digitalWrite(ledR,LOW);
    if(++state>=States)state=0;
    while(button_state==HIGH){
      delay(10);
    }
  }else{
    // delay(10);
    digitalWrite(ledR, HIGH);
    digitalWrite(ledG, LOW);
  }
}