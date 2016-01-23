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

// SENSORS
// Temp.Sensor.pin

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
{ // �� �� �� �� �� �� �� = ��������
  ~B01011111, // �
  ~B01111111, // �
  ~B00110111, // �
  ~B01110110, // �
  ~B01100111, // �
  ~B01001110, // �
  ~B01110000, // �
  ~B00110011  // �
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

// display

struct DSP
{
  byte D[4]={0,0,0,0};
  byte AB[4][4];
  byte pos;
  byte brightness;
  boolean update;
};

DSP DISP;

byte D[4]={0,0,0,0}; // Display global

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
void DISP_transitionUp(byte cur, byte A, byte B){
  DISP.AB[0][cur]= A;
  DISP.AB[1][cur] = ~animation__shift_U(~A) | (~B & __A)>>3 ;
  DISP.AB[2][cur] = ~animation__shift_U(~DISP.AB[1][cur]) | (~B & __B)>>1 | (~B & __F)<<1 | (~B & __G)<<3;
  DISP.AB[3][cur]= B;
}
void DISP_hold(byte cur){
  DISP.AB[0][cur]=DISP.D[cur];
  DISP.AB[1][cur]=DISP.D[cur];
  DISP.AB[2][cur]=DISP.D[cur];
  DISP.AB[3][cur]=DISP.D[cur];
}
// globals
unsigned long start;
// uint16_t start;
volatile byte Tic=0;

volatile byte Hour=      15;
volatile byte Minute=    6;
volatile byte Second=    1;
volatile byte Day=       3;
volatile byte DayofWeek= 3;   // Sunday is day 0
volatile byte Month=     1;   // Jan is month 0
volatile byte Year=2016-1900; // the Year minus 1900

void display(byte D[4]){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, D[3]);
  shiftOut(dataPin, clockPin, LSBFIRST, D[2]);
  shiftOut(dataPin, clockPin, LSBFIRST, D[1]);
  shiftOut(dataPin, clockPin, LSBFIRST, D[0]);
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
  disp[0]=FNT_Sensor[LightSensor_id];
  disp[1]=FNT_blank;
  disp[2]=FNT_blank;
  disp[3]=FNT_blank;
  if(t>5) disp[1]=FNT_LightSensorAnimation[0];
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

// DISPLAY buffer
void fill_dot(){
  DISP.D[1]=DISP.D[1]&FNT_dot;
  DISP.AB[0][1]=DISP.AB[0][1]&FNT_dot;
  DISP.AB[1][1]=DISP.AB[1][1]&FNT_dot;
}
void fill_ss(){
  DISP.D[2]=FNT_Digit[Second/10];
  DISP.D[3]=FNT_Digit[Second%10];
  DISP_transitionUp(  3, FNT_Digit[Second%10], FNT_Digit[(Second+1)%10]);
  if(Second%10!=9)
    DISP_hold(2);
  else
    DISP_transitionUp(2, FNT_Digit[Second/10], FNT_Digit[(Second/10+1)%10]);
}
void fill_SS(){
  DISP.D[0]=FNT_blank;
  DISP.D[1]=FNT_blank;
  DISP_hold(0);
  DISP_hold(1);
  fill_ss();
  DISP.pos=1;
}
void fill_HHMM(){
  DISP.D[0]=FNT_Digit[Hour/10];
  DISP.D[1]=FNT_Digit[Hour%10];
  DISP.D[2]=FNT_Digit[Minute/10];
  DISP.D[3]=FNT_Digit[Minute%10];
  if(Tic<5)DISP.D[1]=DISP.D[1]&FNT_dot;
}
void fill_MMSS(){
  DISP.D[0]=FNT_Digit[Minute/10];
  DISP.D[1]=FNT_Digit[Minute%10];
  DISP_hold(0);
  DISP_hold(1);
  fill_ss();
  DISP.D[1]=DISP.D[1]&FNT_dot;
}
void fill_ddmm(){
  DISP.D[0]=FNT_Digit[Day/10];
  DISP.D[1]=FNT_Digit[Day%10];
  DISP.D[2]=FNT_Digit[Month/10];
  DISP.D[3]=FNT_Digit[Month%10];
  DISP.D[1]=DISP.D[1]&FNT_dot;
}
void fill_mmYY(){
  DISP.D[0]=FNT_Digit[Month/10];
  DISP.D[1]=FNT_Digit[Month%10];
  DISP.D[2]=FNT_Digit[Year/10%10];
  DISP.D[3]=FNT_Digit[Year%10];
  DISP.D[1]=DISP.D[1]&FNT_dot;
}
void fill_YYYY(){
  DISP.D[0]=FNT_Digit[(Year+1900)/1000];
  DISP.D[1]=FNT_Digit[(Year+1900)/100%10];
  DISP.D[2]=FNT_Digit[(Year+1900)/10%10];
  DISP.D[3]=FNT_Digit[(Year+1900)%10]&FNT_dot;
}
void fill_W(){
  // DayofWeek 0-6 // 1 = Monday
  // �� �� ��, �� �� - , - �� ��
  if(Tic/4==0){
    DISP.D[0]= ~((((DayofWeek==1)-1)&__A)|(((DayofWeek==2)-1)&__G)|(((DayofWeek==3)-1)&__D));
    DISP.D[1]= ~((((DayofWeek==4)-1)&__A)|(((DayofWeek==5)-1)&__G));
    DISP.D[2]= ~((((DayofWeek==6)-1)&__G)|(((DayofWeek==0)-1)&__D));
    DISP.D[3]= FNT_week;
  }else{
    // DISP.D[0]= FNT_Digit[DayofWeek]|FNT_dot;
    DISP.D[0]= ~(__A|__G|__D);
    DISP.D[1]= ~(__A|__G);
    DISP.D[2]= ~(__G|__D);
    DISP.D[3]= FNT_week;
  }
}
void fill_weekday(){
  // ��������
  // 01234567
  DISP.D[0]=FNT_blank;
  DISP.D[1]=FNT_blank;
  switch(DayofWeek){
    case 1:
    DISP.D[2]=FNT_WeekDays_rus[3];
    DISP.D[3]=FNT_WeekDays_rus[2];
    break;
    case 2:
    DISP.D[2]=FNT_WeekDays_rus[1];
    DISP.D[3]=FNT_WeekDays_rus[6];
    break;
    case 3:
    DISP.D[2]=FNT_WeekDays_rus[5];
    DISP.D[3]=FNT_WeekDays_rus[4];
    break;
    case 4:
    DISP.D[2]=FNT_WeekDays_rus[7];
    DISP.D[3]=FNT_WeekDays_rus[6];
    break;
    case 5:
    DISP.D[2]=FNT_WeekDays_rus[3];
    DISP.D[3]=FNT_WeekDays_rus[6];
    break;
    case 6:
    DISP.D[2]=FNT_WeekDays_rus[5];
    DISP.D[3]=FNT_WeekDays_rus[0];
    break;
    case 0:
    DISP.D[2]=FNT_WeekDays_rus[1];
    DISP.D[3]=FNT_WeekDays_rus[5];
    break;
    default:
    DISP.D[2]=FNT_minus;
    DISP.D[3]=FNT_minus;
  }
}

void DISP_update(){
  DISP.update=true;
}
void DISP_black(){
  byte T[4]={FNT_blank,FNT_blank,FNT_blank,FNT_blank};
  display(T);
}
// CLOCK
// #define ticHz 490 // 490Hz
// #define ticHz 10 // 490Hz
#define ticHz 1000
volatile word tic_tac=0;
volatile boolean toggle1 = 0;

void init_TimerINT(){
  cli();//stop interrupts
  //set timer1 interrupt at 1kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set timer count for 1khz increments
  // OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  OCR1A = 999;// = (8*10^6) / (1000*8) - 1
  // OCR1A = 1999;// = (16*10^6) / (1000*8) - 1
  //had to use 16 bit timer1 for this bc 1999>255, but could switch to timers 0 or 2 with larger prescaler
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}  

ISR(TIMER1_COMPA_vect){//timer1 interrupt 8kHz toggles pin timerPin
//generates pulse wave of frequency 8kHz/2 = 4kHz (takes two cycles for full wave- toggle high then toggle low)
  if (toggle1){
    digitalWrite(timerPin,HIGH);
    toggle1 = 0;
    // if(++Tic>=100)Tic=0;
    if(++tic_tac>=ticHz){       tic_tac=0;
      if(++Second>=60){         Second=0;
        if(++Minute>=60){       Minute=0;
          if(++Hour>=24){       Hour=0;
            if(++DayofWeek>=7){ DayofWeek=0;
            } if(++Day>31){     Day=0;
              if(++Month>=12){  Month=0;
                ++Year;
    } } } } } }
  }
  else{
    digitalWrite(timerPin,LOW);
    toggle1 = 1;
    Tic=tic_tac/100;
    if(Tic%3==0){
      DISP_update();
    }
  }
}


void timerINT(){
  Serial.println(tic_tac);
  digitalWrite(ledR,LOW);
  if(++Tic>10)Tic=0;

  if(++tic_tac>=ticHz){       tic_tac=0;
    if(++Second>=60){         Second=0;
      if(++Minute>=60){       Minute=0;
        if(++Hour>=24){       Hour=0;
          if(++DayofWeek>=7){ DayofWeek=0;
          } if(++Day>31){     Day=0;
            if(++Month>=12){  Month=0;
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
  // attachInterrupt(digitalPinToInterrupt(timerPin),   timerINT, CHANGE);
  init_TimerINT();
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
  // delay(100);
  DISP.pos=0;
  DISP.brightness=4;
  {
    for(signed char i=9;i>=0;i--){
      DISP.D[0]=FNT_Digit[i];
      DISP.D[1]=FNT_Digit[i];
      DISP.D[2]=FNT_Digit[i];
      DISP.D[3]=FNT_Digit[i];
      DISP_update();
      // delay(100);
    }
  }
/*  {
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
*/  Serial.print(".");
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
const char states[States]= {'s','c','w','W','Y'};
// const char states[States]= {'c','W','d','m','Y','s'};

// main loop

void fillState(char s){
  switch(states[s]){
    // case 'L': demo_light(); break;
    // case 'T': demo_temp(); break;
    // case 't': demo_time(); break;
    // case 'a': demo_animation(); break;
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
  if(Tic==0){
    digitalWrite(ledG, HIGH);
    fillState(state);
  }
  if(button_state==HIGH){
    digitalWrite(ledR,LOW);
    if(++state>=States)state=0;
    while(button_state==HIGH){
      DISP_black();
      Serial.println(tic_tac);
      delay(10);
    }
  }else{
    digitalWrite(ledR, HIGH);
    digitalWrite(ledG, LOW);
  }
  // simply update display
  if(DISP.update){
    if(++DayofWeek>=7)DayofWeek=0;// !!! TODO: ������
    DISP.update=false;
    if(DISP.pos==0){
      display(DISP.D);
    } else {
      display(DISP.AB[DISP.pos++-1]);
      if(DISP.pos>4){ // animate 1-4 AB[0..3]
        DISP.D[0]=DISP.AB[3][0];
        DISP.D[1]=DISP.AB[3][1];
        DISP.D[2]=DISP.AB[3][2];
        DISP.D[3]=DISP.AB[3][3];
        display(DISP.D);
        DISP.pos=0;
      }
    }
  }
  digitalWrite(ledR, HIGH);
  digitalWrite(ledG, LOW);
  delay(33);
}