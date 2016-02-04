#define _DEBUG_ 
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;

char compileTime[] = __TIME__;

#include <StandardCplusplus.h>
//#include <system_configuration.h>
//#include <unwind-cxx.h>
//#include <utility.h>
#include <Arduino.h>

// #include <vector>
//#include <TM1637Display.h>
#include "font.h"
#ifndef FNT
FNT F;
#endif
#include "display.h" 
#ifndef D
Display4LED2 D = Display4LED2();
#endif
#include "clock.h" 
#include "settings.h"
Clockwork Clock;
#include "sensors.h"
#include "OneButton.h"
#define BTN 8
OneButton Button(BTN, false);

Sensors S;

// Time

uint8_t Hour=      0;
uint8_t Minute=    58;
uint8_t Second=    0;
uint8_t Day=       1;
uint8_t DayofWeek= 1;   // Sunday is day 0
uint8_t Month=     1;   // Jan is month 0
uint8_t Year=2016-1900; // the Year minus 1900

unsigned long _time; // millis()
unsigned long _c; // 1/4s

sSettings Settings;

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   25
void printTime(){
  Serial.print('\n> ');
  Serial.print(DayofWeek, DEC);
  Serial.print(', ');
  Serial.print(Day, DEC);
  Serial.print('/');
  Serial.print(Month, DEC);
  Serial.print('/');
  Serial.print(Year, DEC);
  Serial.print(' ');
  Serial.print(Hour, DEC);
  Serial.print(':');
  Serial.print(Minute, DEC);
  Serial.print(':');
  Serial.print(Second, DEC);
  Serial.print(' ');
}
void getTime(){
  DateTime now = RTC.now();
  Day=    now.day();          // The day now (1-31)
  Month=  now.month();        // The month now (1-12)
  Year=   now.year()-1900;         // The full four digit year: (2009, 2010 etc)
  Hour=   now.hour();         // The hour now  (0-23)
  Minute= now.minute();       // The minute now (0-59)
  Second= now.second();       // The second now (0-59)
  DayofWeek=now.dayOfTheWeek();  // Day of the week, Sunday is day 1
  // DayofWeek=(now.dayOfTheWeek()+6)%7;  // Day of the week, Sunday is day 1
  #ifdef _DEBUG_
  // printTime();
  #endif
}

void setup()
{
  Serial.begin(9600);
  #ifdef _DEBUG_
  Serial.println(F("Setup..."));
  #endif
  Serial.println( "Compiled: " __DATE__ ", " __TIME__ ", " __VERSION__);
  RTC.begin();
  // Compilation time correction
  // RTC.adjust(DateTime(__DATE__, __TIME__));
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  getTime();
  printTime();
  Settings.Day.Hour=9;
  Settings.Day.Minute=0;
  Settings.Night.Hour=17;
  Settings.Night.Minute=0;

  /*
  int k;
  uint8_t _D[] = { 0xff, 0xff, 0xff, 0xff };
  // All segments on
  D.setSegments(_D);
  delay(TEST_DELAY);
  _D[0]=B00111001;
  _D[1]=B00001001;
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
  */
  S.init();
  /*
  for(k = 15; k >= 4; k--) {
    D.setBrightness(k); 
    D.setSegments(_D);
    delay(TEST_DELAY); 
  }
//  }
  */
  D.setBrightness(0x08);
  _time = millis();
  MachineStart();
}

void loop()
{
  getTime(); // !!! no time sync yet
  Button.tick();
  if((millis()-_time)>250){
    _time=millis();
    D.update();
  }
  delay(10);
}
/*
int prevTemp=0;

void showTemp(){
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
*/

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


bool _inHour(tHHMM begin, tHHMM end, tHHMM v)
{
// 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22
// 23 24 0 1 2 3 4 5 6 7
  bool f=false;
// if(v.Hour>23) return false;
  if((begin.Hour>23)|| (end.Hour>23)|| (v.Hour>23) ) return false;
  for(uint8_t h=begin.Hour;h!=end.Hour;h++){
    h=h%24; if(v.Hour==h){f=true; break;
    }
  }
  return f;
}
bool _inTime(tHHMM begin, tHHMM end, tHHMM v)
{
  #ifdef _DEBUG_
  Serial.print(v.Hour);
  Serial.print(":");
  Serial.print(v.Minute);
  Serial.print(" in ");
  Serial.print(begin.Hour);
  Serial.print(":");
  Serial.print(begin.Minute);
  Serial.print("-");
  Serial.print(end.Hour);
  Serial.print(":");
  Serial.print(end.Minute);
  #endif
//  ... > Sunrise > Day > Sunset > Night > ... [begin,end)
  if(_inHour(begin, end, v)) {
// 16:41 === 16:40
    if(v.Hour==begin.Hour){
      if(v.Minute>=begin.Minute) return true;
      else return false;
    }
// 7:30 === 7:40
    if(v.Hour==end.Hour){ 
      if(v.Minute<end.Minute) return true;
      else return false;
    }
  return true; // [16:40.. [17:--,0:--,6:--] ..7:40)
  }else
  return false;
};

/* MACHINE */

struct stateStruct
{
  callbackFunction fn;
  transition_fx f;
  word timer;
};
stateStruct _state;
std::vector<stateStruct> _states;
callbackFunction _refreshFunction;
callbackFunction _defaultState;
// std::function<void(void)> _defaultState;
callbackFunction _prevState;
callbackFunction _onClick;
callbackFunction _onDoubleClick;
callbackFunction _onPress;
// word _c; // counter [s] in current state = 45.51 h
void setDefaultState(){
  #ifdef _DEBUG_
  Serial.print("[setDefaultState ");
  #endif
  tHHMM thm;
  thm.Hour=Hour;
  thm.Minute=Minute;
  if(_inTime(Settings.Day,Settings.Night,thm)){
    _defaultState = DaylightClock;
    #ifdef _DEBUG_
    Serial.print(" DaylightClock]");
    #endif
  }
  else{
    _defaultState = NightClock;
    #ifdef _DEBUG_
    Serial.print(" NightClock]");
    #endif
  }
  // _defaultState = ShowSensors;
};

void _fallBack(){
  #ifdef _DEBUG_
  Serial.print(F("<<<_fallBack "));
  #endif
  setDefaultState();
  clearStates();
  addState(_defaultState,0,fxCut);
  nextState();
};

void addState(callbackFunction state , word d, transition_fx f){
  #ifdef _DEBUG_
  Serial.print(F("<<<addState "));
  #endif
  stateStruct s;
  s.fn=state;
  s.timer=d;
  s.f = f;
  _states.insert(_states.begin(), s);
};

void nextState()
{
  #ifdef _DEBUG_
  Serial.println(F("nextState>>>"));
  #endif
  // const uint8_t _D[4]={F.blank,F.blank,F.blank,F.blank}; D.hold(_D);
  if(_states.size()>0){
    _prevState = _state.fn;
    _state = _states.back();
    _states.pop_back();
  // initializing 
  }else{
  #ifdef _DEBUG_
    Serial.println("<<<_defaultState");
  #endif
    _state.fn=_defaultState;
    _state.f=fxCut;
    _state.timer=0;
  }
  #ifdef _DEBUG_
  Serial.print(Hour);
  Serial.print(":");
  Serial.print(Minute);
  Serial.print(":");
  Serial.print(Second);
  Serial.print(F("  \t"));
  #endif
  _c = 0;
  D.drawToBuffer();
  _state.fn();
  D.transition(_state.f);
  // D.debug_print();
}

void clearStates(){
  _states.clear();
  _states.reserve(12);
}
void onClick(){
  Serial.print("<onClick>");
  if(_onClick)_onClick();
    else Serial.print(" nope!");
  Serial.print("\n");
}
void onDoubleClick(){
  Serial.print("onDoubleClick !");
  if(_onDoubleClick)_onDoubleClick();
    else Serial.print(" nope!");
  Serial.print("\n");
  // nextState();
}
void onPress(){
  Serial.print("onPress !");
  if(_onPress)_onPress();
    else Serial.print(" nope!");
  Serial.print("\n");
}

void MachineStart(){
  #ifdef _DEBUG_
  Serial.print("\n\nMachineStart... ");
  #endif
  clearStates();
  // set # millisec after single click is assumed.
  // Button.setClickTicks(600);
  // set # millisec after press is assumed.
  // Button.setPressTicks(2000);
  // attach Button functions
  Button.attachClick(onClick);
  Button.attachDoubleClick(onDoubleClick);
  // Button.attachLongPressStop(onPress);
  Button.attachLongPressStart(onPress);
  Clock.init();
  _c=0;
  setDefaultState();
  ClockYYYY();
  addState(ClockDDMM,     1, fxCut);
  addState(ClockHHMM,     1, fxCut);
  addState(_defaultState, 0, fxCut);
  nextState();
  D.setRefresh(update);
  #ifdef _DEBUG_
  Serial.println(" --- complete.");
  #endif
};
// void set(callbackFunction f){
//   _refreshFunction=f;
// };

 // 1s
void update(){
  // #ifdef _DEBUG_
  // Serial.println(F("<update>"));
  // #endif
  if(_state.fn)_state.fn();
  else{
    // #ifdef _DEBUG_
    Serial.println("!!! undef _state.fn !!!");
    // nextState();
    // _fallBack();
    // return;
    // if(_defaultState)_defaultState();
    // #endif
  }
// 0 = last state or shortest (1s) show
  if(_state.timer==0){
    #ifdef _DEBUG_
    Serial.print(F("."));
    #endif
    if(_states.size()>0){
      // #ifdef _DEBUG_
      // Serial.println("<states>");
      // #endif
      nextState();
    }else{
      // #ifdef _DEBUG_
      // Serial.println("(states) ");
      // #endif  
      _fallBack();    
    }
  }else{
// all that >0 is a countdown
    if(_state.timer-- >0){
      #ifdef _DEBUG_
      Serial.print("<");
      Serial.print(_state.timer);
      Serial.print("> ");
      #endif
      if(_state.timer==0) {
      #ifdef _DEBUG_
      Serial.print("<<timer>> ");
      #endif
        nextState();
      }
    }
  };
  _c++;
  // tictac();
};

void cycleOneClick(){
  clearStates();
  #ifdef _DEBUG_
  Serial.println("cycleOneClick");
  #endif
  S.update();
  addState(ShowCO2,      3, fxDown);
  addState(ShowTemp,     3, fxDown);
  addState(ShowHumidity, 3, fxDown);
  addState(ShowPressure, 3, fxDown);
  setDefaultState();
  addState(ClockHHMM,    3, fxDown);
  addState(_fallBack,    0, fxCut);
  _onClick=nextState;
  // _onDoubleClick=NULL;
  _onPress=_fallBack;
// nextState();
};

void cycleTwoClick(){
  clearStates();
  #ifdef _DEBUG_
  Serial.println(F("cycleTwoClick"));
  #endif
  addState(ClockMMSS, 5, fxFadeRight);
  addState(ClockWeek, 5, fxRight);
// addState(DDWDMM, 1, fxRight);
  addState(ClockDDWD, 3, fxRight);
  addState(ClockDDMM, 3, fxDown);
  addState(ClockYYYY, 3, fxRight);
  // addState(ClockSunset,  3, fxDown);
  // addState(ClockSunrise, 3, fxDown);
  setDefaultState();
  addState(ClockHHMM, 3, fxLeft);
  addState(_fallBack, 0, fxCut);
  _onClick=nextState;
  // _onDoubleClick=NULL;
  _onPress=_fallBack;
// nextState();
};
// void Scroller(){
//   if(_c==0){
//   }
// };


// States

void ClockHHMM(){
  #ifdef _DEBUG_
  // Serial.print(".");
  if(_c==0)Serial.print("ClockHHMM ");
  #endif
  Clock.HHMM();
}
void ClockMMSS(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ClockMMSS ");
  #endif
  Clock.MMSS();
}
void ClockWeek(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ClockWeek ");
  #endif
  Clock.Week();
}
void ClockDDWD(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ClockDDWD ");
  #endif
  Clock.DDWD();
}
void ClockDDMM(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ClockDDMM ");
  #endif
  Clock.DDMM();
}
void ClockYYYY(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ClockYYYY ");
  #endif
  Clock.YYYY();
}
void ShowTemp(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ShowTemp ");
  #endif
  S.showTemp();
}
void ShowHumidity(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ShowHumidity ");
  #endif
  S.showHumidity();
}
void ShowPressure(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ShowPressure ");
  #endif
  S.showPressure();
}
void ShowCO2(){
  #ifdef _DEBUG_
  if(_c==0)Serial.print("ShowCO2 ");
  #endif
  S.showCO2();  
}
void ClockSunset(){Clock.Sunset();}
void ClockSunrise(){Clock.Sunrise();}

// extern void NightClock(void);

void ShowSensors(){
  #ifdef _DEBUG_
  if(_c==0)Serial.println("ShowSensors");
  #endif
  if(_c==0){
    clearStates();
    // _defaultState = ShowSensors;
    _onClick = cycleOneClick;
    // _onDoubleClick = cycleOneClick;
    _onDoubleClick = NightClock;
    _onPress = _fallBack;
    S.update();
    D.setBrightness(0x08);
    addState(ShowPressure, 3, fxRight);
    addState(ShowTemp,     3, fxRight);
    addState(ShowHumidity, 3, fxRight);
    // addState(ClockMMSS,0,fxUp);
    addState(_defaultState,1, fxCut);      
  }  
};

void NightClock(){
  // #ifdef _DEBUG_
  // Serial.println("NightClock");
  // #endif
  if(_c==0){
    #ifdef _DEBUG_
    Serial.println("NightClock -- init");
    #endif
    clearStates();
    S.update();
    D.setBrightness(0x08);
    ClockHHMM();
    // _defaultState = NightClock;
    _onClick = cycleOneClick;
    _onDoubleClick = cycleTwoClick;
    // _onDoubleClick = DaylightClock;
    _onPress = _fallBack;
    // addState(ClockHHMM, 1, fxCut);
    // addState(ClockMMSS, 3, fxRight);
  }else{
    ClockHHMM();
  }
  if(Second==57){
    if(Minute%10==9){
      addState(ClockHHMM, 7, fxCut);
      addState(ClockMMSS, 3, fxFadeRight);
      addState(ClockDDMM, 3, fxRight);
      // addState(ClockHHMM, 5, fxLeft);
    }
    addState(ShowTemp,     3, fxDown);
    addState(ClockHHMM,    6, fxUp);
    addState(ShowHumidity, 3, fxDown);
    addState(ClockHHMM,    6, fxUp);
    addState(ShowCO2,      3, fxDown);
    addState(ClockHHMM,   10, fxUp);
    addState(ShowTemp,     3, fxDown);
    addState(ClockHHMM,    6, fxUp);
    addState(ShowHumidity, 3, fxDown);
    addState(ClockHHMM,    6, fxUp);
    addState(_defaultState,0, fxCut);
  }
  /*
  if(Second==5){
    addState(ShowTemp,     5, fxDown);
    addState(ShowHumidity, 5, fxDown);
    addState(ShowPressure, 5, fxDown);
    addState(ClockMMSS,    1, fxUp);
    addState(ClockHHMM,    1, fxLeft);
    addState(_defaultState,0, fxCut);
  }
  if(Second==30){
    addState(ShowTemp,     5, fxDown);
    addState(ClockHHMM,    5, fxUp);
    addState(ShowHumidity, 5, fxDown);
    addState(ClockHHMM,    5, fxUp);
    addState(_defaultState,0, fxCut);
  }
  */
  if(((Hour==Settings.Day.Hour)&&(Minute==Settings.Day.Minute))&&(Second==30)){
    clearStates();
    addState(ClockSunrise, 1,fxUp);
    addState(DaylightClock,0,fxUp);
  }    
};
/*
void ClockMenu(){
if(_c==0){
clearStates();
Menu.init();
addState(Menu.show, 0, fxLeft);
_onClick=Menu.nextMenu;
_onDoubleClick=Menu.set;
_onPress=Menu.back;
}
if(Menu.update)Menu.update();
if(Menu.exitMenu){
addState(_defaultState, 0, fxLeft); // go back to [HH:MM]
}
}
*/
void DaylightClock(){
  if(_c==0) {
    #ifdef _DEBUG_
    Serial.println(F("DaylightClock"));
    #endif
    clearStates();
    D.setBrightness(0xff);
    // _defaultState = DaylightClock;
    _onClick = cycleOneClick;
    _onDoubleClick = cycleTwoClick;
    _onPress = _fallBack;
    addState(ClockHHMM, 0, fxCut);
    // nextState();
  };
  if(Second==55){
    S.update();
    clearStates();
    addState(ClockMMSS,    8, fxRight);
    // addState(ClockHHMM,    8, fxLeft);
    addState(ShowTemp,     4, fxRight);
    addState(ShowHumidity, 4, fxRight);
    addState(ShowCO2,      4, fxRight);
    if(Minute%5==4) addState(ShowPressure, 4, fxRight);
    addState(ClockHHMM,    0, fxRight);      
    addState(_defaultState,0, fxCut);      
    // addState(_defaultState,1, fxRight);
    nextState();
  }else{
    // if((Minute%15==14)&&(Second==0))
    // S.update();
    if(((Hour==Settings.Night.Hour)&&(Minute==Settings.Night.Minute))&&(Second==30)){
      clearStates();
      addState(ClockSunset,1,fxCut);
      addState(NightClock, 0,fxCut);
      setDefaultState();
    }else {
      // Serial.println("yep, here!");
      addState(_defaultState,  0, fxCut);
    }
  }
};

void tictac(){
  if(++Second>=60){         Second=0;
    if(++Minute>=60){       Minute=0;
      if(++Hour>=24){       Hour=0;
        if(++DayofWeek>=7){ DayofWeek=0;
        } if(++Day>31){     Day=0;
          if(++Month>=12){  Month=0;
            ++Year;
} } } } }
}
char getInt(const char* string, int startIndex) {
  return int(string[startIndex] - '0') * 10 + int(string[startIndex+1]) - '0';
}
