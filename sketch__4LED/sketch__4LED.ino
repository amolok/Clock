#define __DEBUG__
// #define __DEBUG_F__
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

uint8_t Hour =      0;
uint8_t Minute =    58;
uint8_t Second =    0;
uint8_t Day =       1;
uint8_t DayofWeek = 1;  // Sunday is day 0
uint8_t Month =     1;  // Jan is month 0
uint8_t Year = 2016 - 1900; // the Year minus 1900

unsigned long _time; // millis()
unsigned long _c; // 1/4s

sSettings Settings;

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   25
void printTime() {
  Serial.print("\n");
  Serial.print("> ");
  Serial.print(DayofWeek, DEC);
  Serial.print(" - ");
  Serial.print(Day, DEC);
  Serial.print('/');
  Serial.print(Month, DEC);
  Serial.print('/');
  Serial.print(Year+1900, DEC);
  Serial.print(' ');
  Serial.print(Hour, DEC);
  Serial.print(':');
  Serial.print(Minute, DEC);
  Serial.print(':');
  Serial.print(Second, DEC);
  Serial.print(' ');
}
void getTime() {
  DateTime now = RTC.now();
  Day =    now.day();         // The day now (1-31)
  Month =  now.month();       // The month now (1-12)
  Year =   now.year() - 1900;      // The full four digit year: (2009, 2010 etc)
  Hour =   now.hour();        // The hour now  (0-23)
  Minute = now.minute();      // The minute now (0-59)
  Second = now.second();      // The second now (0-59)
  DayofWeek = now.dayOfTheWeek(); // Day of the week, Sunday is day 1
  // DayofWeek=(now.dayOfTheWeek()+6)%7;  // Day of the week, Sunday is day 1
#ifdef __DEBUG__
  // printTime();
#endif
}

void setup()
{
  Serial.begin(9600);
#ifdef __DEBUG__
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
  Settings.Day.Hour = 9;
  Settings.Day.Minute = 0;
  Settings.Night.Hour = 17;
  Settings.Night.Minute = 0;
  Settings.Sleep.start={23,0};
  Settings.Sleep.end={7,30};
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
  if ((millis() - _time) > 249) {
    _time = millis();
    D.update();
  }
  delay(10);
}

bool _inHour(tHHMM begin, tHHMM end, tHHMM v)
{
  // 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22
  // 23 24 0 1 2 3 4 5 6 7
  bool f = false;
  // if(v.Hour>23) return false;
  if ((begin.Hour > 23) || (end.Hour > 23) || (v.Hour > 23) ) return false;
  for (uint8_t h = begin.Hour; h != end.Hour; h++) {
    h = h % 24; if (v.Hour == h) {
      f = true; break;
    }
  }
  return f;
}
bool _inTime(tHHMM begin, tHHMM end, tHHMM v)
{
  /*
#ifdef __DEBUG__
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
*/
  //  ... > Sunrise > Day > Sunset > Night > ... [begin,end)
  if (_inHour(begin, end, v)) {
    // 16:41 === 16:40
    if (v.Hour == begin.Hour) {
      if (v.Minute >= begin.Minute) return true;
      else return false;
    }
    // 7:30 === 7:40
    if (v.Hour == end.Hour) {
      if (v.Minute < end.Minute) return true;
      else return false;
    }
    return true; // [16:40.. [17:--,0:--,6:--] ..7:40)
  } else
    return false;
};

/* MACHINE */

struct stateStruct
{
  callbackFunction fn;
  transition_fx f;
  word timer;
};

// states
stateStruct _state;
std::vector<stateStruct> _states;
callbackFunction _refreshFunction;
callbackFunction _defaultState;
callbackFunction _prevState;
// control
callbackFunction _onClick;
callbackFunction _onDoubleClick;
callbackFunction _onPress;

void setDefaultState() {
#ifdef __DEBUG__
  Serial.print("[");
#endif
  tHHMM thm;
  thm.Hour = Hour;
  thm.Minute = Minute;
  if (_inTime(Settings.Day, Settings.Night, thm)) {
    _defaultState = DaylightClock;
  } else {
    if(_inTime(Settings.Sleep.start, Settings.Sleep.end, thm)){
      _defaultState = DeepNightClock;
    }else{
      _defaultState = NightClock;
    }
  }
// _defaultState = DeepNightClock;
#ifdef __DEBUG_F__
Serial.print((unsigned long)_defaultState);
Serial.println(" ]");
#endif
};

void _fallBack() {
#ifdef __DEBUG__
  Serial.print(F("<"));
#endif
  setDefaultState();
  clearStates();
  #ifdef __DEBUG_F__
  Serial.print("<<<");
  Serial.print((unsigned long)_defaultState);
  Serial.print("<<< ");
  #endif
  addState(_defaultState, 0, fxCut);
  // nextState();
  // if(_defaultState)_defaultState();
};

void addState(callbackFunction state , word d, transition_fx f) {
#ifdef __DEBUG__
  Serial.print(F("+"));
#endif
  stateStruct s;
  s.fn = state;
  s.timer = d;
  s.f = f;
  _states.insert(_states.begin(), s);
};

void _debug_time(){
  Serial.print(F("\n"));
  Serial.print(Hour);
  Serial.print(":");
  Serial.print(Minute);
  Serial.print(":");
  Serial.print(Second);
  Serial.print(F("  \t"));
}

void nextState()
{
  // const uint8_t _D[4]={F.blank,F.blank,F.blank,F.blank}; D.hold(_D);
  if (_states.size() > 0) {
    if(_state.fn != _defaultState)_prevState = _state.fn;
    _state = _states.back();
    _states.pop_back();
    // initializing
  } else {
#ifdef __DEBUG__
    Serial.print(F("<<<"));
    // Serial.println("<<<_defaultState");
#endif
    _state.fn = _defaultState;
    _state.f = fxCut;
    _state.timer = 0;
  }
#ifdef __DEBUG_F__
  Serial.print(F("nextState "));
  Serial.print((unsigned long)_state.fn);
#endif
#ifdef __DEBUG__
  Serial.print(F(" >>> "));
  _debug_time();
#endif
  _c = 0;
  D.drawToBuffer();
  _state.fn();
  D.transition(_state.f);
  // D.debug_print();
}

void clearStates() {
  _states.clear();
  _states.reserve(12);
  // _c=0;
}
void onClick() {
  #ifdef __DEBUG__
  Serial.print("\n<onClick>\n");
  #endif
  if (_onClick)_onClick();
}
void onDoubleClick() {
  #ifdef __DEBUG__
  Serial.print("\n<onDoubleClick>\n");
  #endif
  if (_onDoubleClick)_onDoubleClick();
  // nextState();
}
void onPress() {
  #ifdef __DEBUG__
  Serial.print("\n<onPress>\n");
  #endif
  if (_onPress)_onPress();
}

void MachineStart() {
#ifdef __DEBUG__
  Serial.print("\n\nMachine Starting... ");
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
  _c = 0;
  setDefaultState();
  ClockYYYY();
  addState(ClockDDMM,     1, fxCut);
  addState(ClockHHMM,     2, fxCut);
  addState(_defaultState, 0, fxCut);
  nextState();
  D.setRefresh(update);
#ifdef __DEBUG__
  Serial.println("... Machine started.");
#endif
};
// void set(callbackFunction f){
//   _refreshFunction=f;
// };

// 1s
void update() {
  #ifdef __DEBUG_F__
  Serial.print(F("<"));
  Serial.print((unsigned long)_state.fn);
  Serial.print(F(">"));
  #endif
  if (_state.fn)_state.fn();
  else {
    // #ifdef __DEBUG__
    Serial.println("!!! undef _state.fn !!!");
    // nextState();
    // _fallBack();
    // return;
    // if(_defaultState)_defaultState();
    // #endif
  }
  // 0 = last state or shortest (1s) show
  if (_state.timer == 0) {
#ifdef __DEBUG__
    // Serial.print(F("."));
#endif
    if (_states.size() > 0) {
      #ifdef __DEBUG__
      Serial.print(">");
      #endif
      nextState();
    } else {
      // #ifdef __DEBUG__
      // Serial.print(".");
      // #endif
      if(_c>3*60)_fallBack();
    }
  } else {
    // all that >0 is a countdown
    if (_state.timer-- > 0) {
#ifdef __DEBUG_F__
      Serial.print("<");
      Serial.print(_state.timer);
      Serial.print("> ");
#endif
      if (_state.timer == 0) {
#ifdef __DEBUG_F__
        Serial.print("<<timer>> ");
#endif
        nextState();
      }
    }
  };
  _c++;
  // tictac();
};

// void Scroller(){
//   if(_c==0){
//   }
// };


/* STATES */


void ClockNOPE() {
  // Clock.BlankBlink();
  D._hold(0, F.blank);
  D._hold(1, F.blank);
  D._hold(2, F.blank);
  D._hold(3, F.blank);
  if(Second%2==0)
    D._hold(1, F.dot);
  #ifdef __DEBUG__
  Serial.print(F("."));
  #endif
}

void ClockHHMM() {
#ifdef __DEBUG__
  // Serial.print(".");
  if (_c == 0)Serial.print("ClockHHMM ");
#endif
  Clock.HHMM();
}

void ClockMMSS() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ClockMMSS ");
#endif
  Clock.MMSS();
}
void ClockWeek() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ClockWeek ");
#endif
  Clock.Week();
}
void ClockDDWD() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ClockDDWD ");
#endif
  Clock.DDWD();
}
void ClockDDMM() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ClockDDMM ");
#endif
  Clock.DDMM();
}
void ClockYYYY() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ClockYYYY ");
#endif
  Clock.YYYY();
}
void ShowTemp() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ShowTemp ");
#endif
  S.showTemp();
}
void ShowTempBMP() {S.showTempBMP(); }
void ShowTempCPU() {S.showTempCPU(); }
void ShowHumidity() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ShowHumidity ");
#endif
  S.showHumidity();
}
void ShowPressure() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ShowPressure ");
#endif
  S.showPressure();
}
void ShowCO2() {
#ifdef __DEBUG__
  if (_c == 0)Serial.print("ShowCO2 ");
#endif
  S.showCO2();
}
void ClockSunset() {
  Clock.Sunset();
}
void ClockSunrise() {
  Clock.Sunrise();
}

void switchToClockFace(){
  clearStates();
  // if(_c==0){
    #ifdef __DEBUG__
      if (_c == 0)Serial.print("switchToClockFace ");
    #endif    
    _onClick=_fallBack;
    _onDoubleClick=_fallBack;
    _onPress=_fallBack;
  // }
  addState(ClockSS_Seconds, 0, fxFadeIn);
  nextState();
}

void ClockSS_Seconds(){
  D._hold(0, F.blank);
  D._hold(1, F.Clock.back[0]);
  D._hold(2, F.Clock.back[1]);
  D._hold(3, F.blank);
  D.blink(1, F.Clock.face[(Second/5)*2]);
  D.blink(2, F.Clock.face[(Second/5)*2+1]);
}
// extern void NightClock(void);

/* CYCLES */

void ClockDDWDMM(){
  if(_c==3){
    D.drawToBuffer();
    ClockDDMM();
    D.transition(fxDown);
    D._DD(0,Day);
    D.transition(fxCut);
    return;
  }
  if(_c<3)ClockDDWD();
  if(_c>3)ClockDDMM();
}

void cycleDate() {
  addState(ClockMMSS, 5, fxMixRight);
  addState(ClockWeek, 4, fxRight);
  addState(ClockDDWDMM, 6, fxRight);
  // addState(ClockDDWD, 3, fxRight);
  // addState(ClockDDMM, 3, fxDown);
  addState(ClockYYYY, 3, fxRight);
};

void cyclePopupInnerCondiotions() {
  addState(ShowCO2,      3, fxDown);
  addState(ShowTemp,     3, fxDown);
  addState(ShowTempCPU,  3, fxDown);
  addState(ShowTempBMP,  3, fxDown);
  addState(ShowHumidity, 3, fxDown);
  addState(ShowPressure, 3, fxDown);
  // addState(ShowTemp,     3, fxDown);
  // addState(ClockHHMM,    6, fxUp);
  // addState(ShowHumidity, 3, fxDown);
  // addState(ClockHHMM,   10, fxUp);
}


void cycleOneClick() {
  clearStates();
#ifdef __DEBUG__
  Serial.println("cycleOneClick");
#endif
  S.update();
  setDefaultState();
  cyclePopupInnerCondiotions();
  addState(ClockHHMM,    3, fxDown);
  addState(_fallBack,    0, fxCut);
  _onClick = nextState;
  _onDoubleClick=switchToClockFace;
  _onPress = _fallBack;
  nextState();
};

void cycleTwoClick() {
#ifdef __DEBUG__
  Serial.println(F("cycleTwoClick"));
#endif
  clearStates();
  // _onClick = nextState;
  _onClick = switchToClockFace;
  _onDoubleClick=switchToClockFace;
  // _onPress = switchToClockFace;
  _onPress = _fallBack;
  cycleDate();
  setDefaultState();
  addState(ClockHHMM, 3, fxLeft);
  addState(_fallBack, 0, fxCut);
  nextState();
};

void cycleSensors() {
  addState(ShowPressure, 3, fxRight);
  addState(ShowTemp,     3, fxRight);
  addState(ShowHumidity, 3, fxRight);
  // addState(ClockMMSS,0,fxUp);
}

void ShowSensors() {
#ifdef __DEBUG__
  if (_c == 0)Serial.println("ShowSensors");
#endif
  if (_c == 0) {
    clearStates();
    // _defaultState = ShowSensors;
    _onClick = cycleOneClick;
    // _onDoubleClick = cycleOneClick;
    _onDoubleClick = NightClock;
    _onPress = _fallBack;
    S.update();
    D.setBrightness(0x08);
    cycleSensors();
    addState(_defaultState, 0, fxCut);
  }
};

void NightClock() {
  // #ifdef __DEBUG__
  // Serial.println("NightClock");
  // #endif
  if (_c == 0) {
#ifdef __DEBUG__
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
  } else {
    ClockHHMM();
  }
  if (Second == 55) {
    // update sensors
    S.update();
    // every hour
    if (Minute == 59) {
      addState(ClockHHMM, 10, fxCut);
      cycleDate();
      addState(ClockHHMM,  6, fxLeft);
    } else {
      addState(ClockMMSS,  6, fxMixRight);
      addState(ClockHHMM,    6, fxMixLeft);
    }
    cyclePopupInnerCondiotions();
    addState(ClockHHMM,    1, fxUp);
    addState(_fallBack,    0, fxCut);
  }
  if (Second == 30) {
    cyclePopupInnerCondiotions();
    addState(ClockHHMM,    1, fxUp);
    addState(_fallBack,    0, fxCut);
  }
  if (Second == 40) {
    addState(ClockMMSS,    7, fxMixRight);
    addState(ClockHHMM,    3, fxMixLeft);
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
  if (((Hour == Settings.Day.Hour) && (Minute == Settings.Day.Minute)) && (Second == 30)) {
    clearStates();
    addState(ClockSunrise, 1, fxUp);
    addState(DaylightClock, 0, fxUp);
  }
};

void DaylightClock() {
  if (_c == 0) {
#ifdef __DEBUG__
    Serial.println(F("DaylightClock"));
#endif
    clearStates();
    D.setBrightness(0x0f);
    // _defaultState = DaylightClock;
    _onClick = cycleOneClick;
    _onDoubleClick = cycleTwoClick;
    _onPress = _fallBack;
    ClockHHMM();
    // addState(ClockHHMM, 0, fxCut);
    // nextState();
  };
  if (Second == 55) {
    S.update();
    clearStates();
    addState(ClockMMSS,    8, fxRight);
    // addState(ClockHHMM,    8, fxLeft);
    addState(ShowTemp,     4, fxRight);
    addState(ShowHumidity, 4, fxRight);
    addState(ShowCO2,      4, fxRight);
    if (Minute % 5 == 4) addState(ShowPressure, 4, fxRight);
    addState(ClockHHMM,    0, fxRight);
    addState(_defaultState, 0, fxCut);
    // addState(_defaultState,1, fxRight);
    nextState();
  } else {
    // if((Minute%15==14)&&(Second==0))
    // S.update();
    if (((Hour == Settings.Night.Hour) && (Minute == Settings.Night.Minute)) && (Second == 30)) {
      clearStates();
      addState(ClockSunset, 1, fxCut);
      addState(NightClock, 0, fxCut);
      setDefaultState();
    } else {
      // Serial.println("yep, here!");
      // addState(_defaultState,  0, fxCut);
      ClockHHMM();
    }
  }
};

void cycleNightClick() {
  clearStates();
  addState(ClockHHMM, 4, fxDown);
  addState(ClockDDWDMM, 6, fxDown);
  cyclePopupInnerCondiotions();
  addState(ClockHHMM, 4, fxUp);
  // addState(ClockMMSS, 9, fxMixLeft);
  addState(ClockNOPE, 5, fxFadeOut);
  // addState(ClockNOPE, 90, fxCut);
}

void cycleNightDoubleClick(){
  clearStates();
  addState(ClockHHMM, 3, fxRight);
  addState(ClockMMSS, 3, fxMixRight);
  addState(ClockDDWD, 3, fxMixRight);
  addState(ClockNOPE, 0, fxRight);
  addState(_fallBack, 0, fxCut);
}

void DeepNightClock() {
  // if(_c<12){
  if (_c == 0) {
    #ifdef __DEBUG__
    Serial.println(F("\nDeepNightClock "));
    #endif
    S.update();
      // clearStates();
    D.setBrightness(0x08);
      // _defaultState = DeepNightClock;
    _onClick = cycleNightClick;
    _onDoubleClick = cycleNightDoubleClick;
    _onPress = _fallBack;
  }
  // }else{
  // }

  ClockNOPE();

  if(Hour<3){
    if(Second==15)
    {
      if(Minute%10==1)addState(ShowTemp,     4, fxFadeIn);
      if(Minute%10==2)addState(ShowHumidity, 4, fxFadeIn);
      if(Minute%10==3)addState(ShowCO2,      4, fxFadeIn);
      if(Minute%10==4)addState(ShowPressure, 4, fxFadeIn);
      if(Minute%10==5)addState(ClockWeek,    4, fxFadeIn);
      if(Minute%10==6)addState(ClockDDWD,    4, fxFadeIn);
      if(Minute%10==7)addState(ClockDDMM,    4, fxFadeIn);
      if(Minute%10==8)addState(ShowHumidity, 4, fxFadeIn);
      if(Minute%10==9)addState(ShowTemp,     4, fxFadeIn);
      if(Minute%10==0)addState(ShowCO2,      4, fxFadeIn);
      addState(ClockNOPE, 1, fxFadeOut);
    }
    if(Second==30)
    {
      if(Minute%10==0){
        S.update();
        clearStates();
        addState(ClockHHMM,  4, fxFadeIn);
        if(Minute%10==0){
          cyclePopupInnerCondiotions();
          addState(ClockHHMM,4, fxDown);
        }
        addState(ClockNOPE, 1, fxFadeOut);
      }
      if(Minute%10==5){
        addState(ClockHHMM, 4, fxDown);
        addState(ClockNOPE, 1, fxDown);
        // addState(_fallBack, 0, fxCut);
      }
    }

    if(Second==45)
    {
      if(Minute%10==3){
        addState(ClockHHMM, 4, fxUp);
        addState(ClockNOPE, 1, fxDown);
        // addState(_fallBack, 0, fxCut);
      }
    }

  }

  // any hour
  if(Second == 55)
  {
    S.update();
    clearStates();
    if(Minute==59){
      addState(ClockHHMM,  6, fxFadeIn);
      cyclePopupInnerCondiotions();
      addState(ClockHHMM,  8, fxDown);
    } else {
      if(Minute%10==1){
        addState(ClockHHMM,  8, fxFadeIn);
        addState(ClockNOPE,  1, fxFadeOut);
      }
      if(Minute%10==2){
        addState(ClockHHMM,  8, fxLeft);
        addState(ClockDDMM,  4, fxLeft);
        addState(ClockNOPE,  1, fxLeft);
      }
      if(Minute%10==3){
        addState(ClockNOPE,  2, fxCut);
        addState(ClockHHMM,  8, fxRight);
        addState(ClockMMSS,  4, fxMixRight);
        addState(ClockNOPE,  1, fxRight);
      }
      if(Minute%10==4){
        addState(ClockNOPE,  4, fxCut);
        addState(ClockHHMM,  6, fxUp);
        addState(ClockNOPE,  1, fxUp);
      }
      if(Minute%10==5){
        addState(ClockHHMM,  8, fxLeft);
        addState(ClockNOPE,  1, fxRight);
      }
    }
    // addState(ClockNOPE,    0, fxFadeOut);
    // addState(_fallBack,    0, fxCut);
  }
  // Serial.print(Second);
  // Serial.print(F("@"));
}
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


  void tictac() {
    if (++Second >= 60) {
      Second = 0;
      if (++Minute >= 60) {
        Minute = 0;
        if (++Hour >= 24) {
          Hour = 0;
          if (++DayofWeek >= 7) {
            DayofWeek = 0;
          } if (++Day > 31) {
            Day = 0;
            if (++Month >= 12) {
              Month = 0;
              ++Year;
            }
          }
        }
      }
    }
  }
  char getInt(const char* string, int startIndex) {
    return int(string[startIndex] - '0') * 10 + int(string[startIndex + 1]) - '0';
  }
