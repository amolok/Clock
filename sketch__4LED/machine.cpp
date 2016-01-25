#include <Arduino.h>
#include "font.h"
#include "display.h"
#include "clock.h"
// #include <vector>
// #include <QueueArray.h>
#include "settings.h"
#include "machine.h"

extern FNT F;
extern Display4LED2 D;
extern sSettings Settings;
extern uint8_t Year, Month, Day, DayofWeek, Hour, Minute, Second;

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
void Machine::_setDefaultState(){
  tHHMM thm;
  thm.Hour=Hour;
  thm.Minute=Minute;
  if(_inTime(Settings.Day,Settings.Night,thm))
    _defaultState = &Machine::DaylightClock;
  else
    _defaultState = &Machine::NightClock;
};
void Machine::_fallBack(){
  _setDefaultState();
  clearStates();
  // addState(_defaultState,0);
};


void Machine::addState(void (Machine::*state)(void) , word d, callbackFunction fx){
  stateStruct s;
  s.fn=state;
  s.timer=d;
  s.fx = fx;
  _states.push_front(s);
};

void Machine::nextState()
{
  if(_states.size()>0){
    _prevState = _state.fn;
    if(_states.size()>0){
      _state = _states.back();
      _states.pop_back();
      _c = 0;
// initializing 
      D.drawToBuffer(_state.fn);
// can be NULL
      D.transition(_state.fx); 
    }else{
      _state.fn=_defaultState;
      _state.fx=NULL;
      _state.t=0;
    }
  }
};

void Machine::clearStates(){
  _states.clear();
  _states.reserve(12);
}
void Machine::Machine(){
// Sensor Sensors;
  clearStates();
  Clock.init();
  M.init();
  tHHMM thm;
  thm.Hour = Hour; thm.Minute = Minute;
  _setDefaultState();
  addState(_defaultState, 0);
  Clock.set(update);
};
void Machine::set(callbackFunction f){
  _refreshFunction=f;
};
void Machine::update(){ // 1s
  if(_refreshFunction)_refreshFunction();
  if(_state.fn) _state.fn();
  _c++;
if(_state.t==0){ // 0 = last state or shortest (1s) show
  if(_states.size()>0) nextState();
}else{
if(_state.t-- >0) // all that >0 is a countdown
  if(_state.t==0) nextState();
};
};

void Machine::cycleOneClick(){
// clearStates();
  Sensors.update();
  addState(Sensors.showTemp, 1, D.down);
  addState(Sensors.showPressure, 1, D.down);
  addState(Sensors.showHumidity, 1, D.down);
  addState(Sensors.showCO2, 1, D.down);
  addState(_defaultState, 0, D.down);
  _onClick=nextState;
  _onDoubleClick=NULL;
  _onPress=_defaultState;
// nextState();
};
void Machine::cycleTwoClick(){
// clearStates();
  addState(Clock.MMSS, 3, D.right);
  addState(Clock.Week, 3, D.right);
// addState(DDWDMM, 1, D.right);
  addState(Clock.DDWD, 1, D.right);
  addState(Clock.DDMM, 1, D.down);
  addState(Clock.YYYY, 1, D.left);
  addState(_defaultState, 0, D.left);
  _onClick=nextState;
  _onDoubleClick=NULL;
  _onPress=ClockMenu;
// nextState();
};
// void Scroller(){
//   if(_c==0){
//   }
// };
void Machine::DaylightClock(){
  if(_c==0) {
    clearStates();
    _defaultState = &Machine::DaylightClock;
    _onClick = &Machine::cycleOneClick;
    _onDoubleClick = &Machine::cycleTwoClick;
    _onPress = function(){
      clearStates();
      addState(_defaultState, 0, NULL);
    };
  };
  if(Second==58){
    clearStates();
    addState(Clock.MMSS, 2, D.right);
    addState(Clock.HHMM, 2, D.left);
// Sensors.update();
    addState(Sensors.showTemp, 1, D.up);
    addState(Sensors.showPressure, 1, D.up);
    addState(Sensors.showHumidity, 1, D.up);
    addState(Sensors.showCO2, 1, D.up);
    addState(_defaultState, 0, D.up);      
  }
  if((Minute%15==14)&&(Second==0))
    Sensors.update();
  if(((Hour==Settings.Night.Hour)&&(Minute==Settings.Night.Minute))&&(Second==30)){
    clearStates();
    addState(Clock.Sunset,1,D.down);
    addState(NightClock,0,D.down);
  }
};
void Machine::NightClock(){
  if(_c==0){
    clearStates();
    _defaultState = &Machine::NightClock;
    addState(Clock.HHMM, 5, D.left);
    addState(Sensors.showTemp, 5, D.down);
    addState(_defaultState, 0, D.up);      
  }
  if(((Hour==Settings.Day.Hour)&&(Minute==Settings.Day.Minute))&&(Second==30)){
    clearStates();
    addState(Clock.Sunrise,1,D.up);
    addState(DaylightClock,0,D.up);
  }    
};
/*
void ClockMenu(){
if(_c==0){
clearStates();
Menu.init();
addState(Menu.show, 0, D.left);
_onClick=Menu.nextMenu;
_onDoubleClick=Menu.set;
_onPress=Menu.back;
}
if(Menu.update)Menu.update();
if(Menu.exitMenu){
addState(_defaultState, 0, D.left); // go back to [HH:MM]
}
}
*/