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

// Module connection pins (Digital Pins)

// Time

uint8_t Hour=      15;
uint8_t Minute=    6;
uint8_t Second=    1;
uint8_t Day=       3;
uint8_t DayofWeek= 3;   // Sunday is day 0
uint8_t Month=     1;   // Jan is month 0
uint8_t Year=2016-1900; // the Year minus 1900

sSettings Settings;

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   100


void setup()
{
  Settings.Day.Hour=9;
  Settings.Day.Minute=0;
  Settings.Night.Hour=21;
  Settings.Night.Minute=0;

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
    MachineInit();
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
void _setDefaultState(){
  tHHMM thm;
  thm.Hour=Hour;
  thm.Minute=Minute;
  if(_inTime(Settings.Day,Settings.Night,thm))
    _defaultState = &DaylightClock;
  else
    _defaultState = &NightClock;
};
void _fallBack(){
  _setDefaultState();
  clearStates();
  // addState(_defaultState,0);
};

void addState(callbackFunction state , word d, transition_fx f){
  stateStruct s;
  s.fn=state;
  s.timer=d;
  s.f = f;
  _states.insert(_states.begin(), s);
};

void nextState()
{
  if(_states.size()>0){
    _prevState = _state.fn;
    if(_states.size()>0){
      _state = _states.back();
      _states.pop_back();
      _c = 0;
// initializing 
      D.drawToBuffer();
      _state.fn();
// can be NULL
      D.transition(_state.f); 
    }else{
      _state.fn=_defaultState;
      _state.f=fxCut;
      _state.timer=0;
    }
  }
};

void clearStates(){
  _states.clear();
  _states.reserve(12);
}
void MachineInit(){
// Sensor Sensors;
  clearStates();
  Clock.init();
  tHHMM thm;
  thm.Hour = Hour; thm.Minute = Minute;
  _setDefaultState();
  addState(_defaultState, 0, fxCut);
  D.setRefresh(update);
};
// void set(callbackFunction f){
//   _refreshFunction=f;
// };
void update(){ // 1s
  // if(_refreshFunction)_refreshFunction();
  // if(_state.fn) 
  // must use '.*' or '->*' to call pointer-to-member function in '((Machine*)this)->_state.stateStruct::fn (...)', e.g. '(... ->* ((Machine*)this)->_state.stateStruct::fn) (...)'
  _state.fn();
  _c++;
if(_state.timer==0){ // 0 = last state or shortest (1s) show
  if(_states.size()>0) nextState();
}else{
if(_state.timer-- >0) // all that >0 is a countdown
  if(_state.timer==0) nextState();
};
};

/*
void cycleOneClick(){
// clearStates();
  Sensors.update();
  addState(Sensors.showTemp, 1, fxDown);
  addState(Sensors.showPressure, 1, fxDown);
  addState(Sensors.showHumidity, 1, fxDown);
  addState(Sensors.showCO2, 1, fxDown);
  addState(_defaultState, 0, fxDown);
  _onClick=nextState;
  _onDoubleClick=NULL;
  _onPress=_defaultState;
// nextState();
};
void cycleTwoClick(){
// clearStates();
  addState(Clock.MMSS, 3, fxRight);
  addState(Clock.Week, 3, fxRight);
// addState(DDWDMM, 1, fxRight);
  addState(Clock.DDWD, 1, fxRight);
  addState(Clock.DDMM, 1, fxDown);
  addState(Clock.YYYY, 1, fxLeft);
  addState(_defaultState, 0, fxLeft);
  _onClick=nextState;
  _onDoubleClick=NULL;
  _onPress=ClockMenu;
// nextState();
};
// void Scroller(){
//   if(_c==0){
//   }
// };
*/

// States

void ClockHHMM(){Clock.HHMM();}
void ClockMMSS(){Clock.MMSS();}
void ClockSunset(){Clock.Sunset();}
void ClockSunrise(){Clock.Sunrise();}

void DaylightClock(){
  if(_c==0) {
    clearStates();
    _defaultState = &DaylightClock;
    // _onClick = &cycleOneClick;
    // _onDoubleClick = &cycleTwoClick;
    _onPress = clearStates;
  };
  if(Second==58){
    clearStates();
    addState(ClockMMSS, 2, fxRight);
    addState(ClockHHMM, 2, fxLeft);
// Sensors.update();
// addState(Sensors.showTemp, 1, fxUp);
// addState(Sensors.showPressure, 1, fxUp);
// addState(Sensors.showHumidity, 1, fxUp);
// addState(Sensors.showCO2, 1, fxUp);
    addState(_defaultState, 0, fxUp);      
  }
  if((Minute%15==14)&&(Second==0))
    // Sensors.update();
  if(((Hour==Settings.Night.Hour)&&(Minute==Settings.Night.Minute))&&(Second==30)){
    clearStates();
    addState(ClockSunset,1,fxDown);
    addState(NightClock,0,fxDown);
  }
};
void NightClock(){
  if(_c==0){
    clearStates();
    _defaultState = &NightClock;
    addState(ClockHHMM, 5, fxLeft);
    // addState(Sensors.showTemp, 5, fxDown);
    addState(_defaultState, 0, fxUp);      
  }
  if(((Hour==Settings.Day.Hour)&&(Minute==Settings.Day.Minute))&&(Second==30)){
    clearStates();
    addState(ClockSunrise,1,fxUp);
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