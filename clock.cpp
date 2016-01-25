#include <vector>
#include "font.h"
#include "display.h"
// #include "sensors.h"
// #include "OneButton.h"

#ifndef FNT
FNT F;
#endif
#ifndef D
Display D;
#endif

class Clockwork
{
private:
  uint8_t _D[4];

  void _incSM(uint8_t p, uint8_t x){
    D._up(p+1,F.d[x%10],F.d[(x+1)%10]);
    if(x%10==9){
      D._up(p,F.d[x/10],F.d[((x+1)/10)%6]);
    }else{
      D._hold(p,F.d[x/10]);
    }
  }
  void _incHH(uint8_t b, uint8_t x){
    // if(__AMPM); 12 am/pm
    switch(x){
      case 23:
      // D._up(p,F.d[2],(__0H ? '0':F.blank));
      D._up(p  ,F.d[2],F.blank);
      D._up(p+1,F.d[3],F.d[0]);
      return;
      case 19:
      D._up(p,F.d[1],F.d[2]);
      case 9:
      // D._up(p,(__0H ? '0':F.blank,F.d[1]));
      D._up(p,F.blank,F.d[1]);
      break;
      default:
      if(x<10)
        // D._up(p,(__0H ? '0':F.blank,F.d[1]));
        D._hold(p,F.blank);
        // D._hold(p,F.d[x/10]);
      else
        D._hold(p,F.d[x/10]);
      break;
    }
    D._up(p+1,F.d[x%10],F.d[(x+1)%10]);
  }
  void _SM(uint8_t p, uint8_t x){
    D._hold(p,   F.d[x/10]);
    D._hold(p+1, F.d[x%10]);
  }
  // void _HHMM(){
    // _D[0]=Hour/10;
    // _D[1]=Hour%10;
    // _D[3]=Minute/10;
    // _D[4]=Minute%10;
    // D.B(_D);
  // }
public:
    // state = NULL;
    // void attachDisplay(callbackFunction newFunction) {
    //   _display = newFunction;
    // }
  // Display D;
  void init(){
    D.init();
    D.setRefresh(HHMM);
  };
  void set(callbackFunction f){
    D.setRefresh(f);
  };
  void MMSS(){
    _incSM(2, Second);
    if(Second==59){
      _incSM(0, Minute);
    }else{
      _SM(0, Minute);
    }
    D.blink(2, F.dot);
  };
  void HHMM(){
    if(Minute<59){
      D._hold(0, F.d[Hour/10]);
      D._hold(1, F.d[Hour%10]);
    }
    if(Second<59){
      D._hold(2, F.d[Minute/10]);
      D._hold(3, F.d[Minute%10]);
    }else{
      _incSM(2, Minute);
      if(Minute==59){
        _incHH(0,Hour);
      }
    }
  };
  void _WD(uint8_t p, uint8_t wd){
    const uint8_t rus[7][2]={{1,5}, {3,2}, {1,6}, {5,4}, {7,6}, {3,6}, {5,0}, };
    D._hold(p,   F.rusWeekDays[rus[wd][0]]);
    D._hold(p+1, F.rusWeekDays[rus[wd][1]]);
  };
  void DDWD(){
    _DD(0,Day);
    _WD(2,DayofWeek);
    D.on(1, F.dot);
  };
  void DDMM(){
    D._hold(0, F.d[Day/10]);
    D._hold(1, F.d[Day%10]);
    D._hold(2, F.d[(Month+1)/10])
    D._hold(3, F.d[(Month+1)%10])
    D.on(1, F.dot);
  };
  void Week(){
    uint8_t d[4];
    d[0]= (__A|__G|__D);
    d[1]= (__A|__G);
    d[2]= (__G|__D);
    d[3]= F.week;
    D.hold(d);
    switch(DayofWeek){
      case 1: D.blink(0,__A); break;
      case 2: D.blink(0,__G); break;
      case 3: D.blink(0,__D); break;
      case 4: D.blink(1,__A); break;
      case 5: D.blink(1,__G); break;
      case 6: D.blink(2,__G); break;
      case 0: D.blink(2,__D); break;
      default: D.blink2(3,F.minus);
    };
  };
  void YYYY(){
    D._DD(0, (Year+1900)/100);
    D._DD(2, Year%100);
    D.on(3, F.dot);
  };
  void Sunrise(){
    D._ab(  0, F.Sun.rise);
    D._hold(1, F.blank);
    D._hold(2, F.blank);
    D._hold(3, F.blank);
  };
  void Sunset(){
    D._ab(  0, F.Sun.set);
    D._hold(1, F.blank);
    D._hold(2, F.blank);
    D._hold(3, F.blank);
  };
};

struct tHHMM
{
  uint8_t Hour;
  uint8_t Minute;
};

struct settingsStruct
{
  tHHMM Day, Night; 
  // tHHMM Sunrise, Sunset;
  // tHHMM Wakeup;
  // tHHMM Alarm;
};
settingsStruct Settings;

/*
      MACHINE
*/

struct stateStruct
{
  callbackFunction fn;
  callbackFunction fx;
  word timer;
};

class Machine
{
private:
  callbackFunction _refreshFunction = NULL;
  stateStruct _state;
  std::deque<stateStruct> _states;
  callbackFunction _defaultState = NULL;
  callbackFunction _prevState = NULL;
  callbackFunction _onClick = NULL;
  callbackFunction _onDoubleClick = NULL;
  callbackFunction _onPress = NULL;
  word _c; // counter [s] in current state = 45.51 h
  // word _d; // delay [s], if 0 -- nonstop
  // callbackFunction _states[6];
  // word _delaysStack[6]; // [s] 45.51 = h
  bool _inHour(uint8_t begin, end, v){
    // 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22
    // 23 24 0 1 2 3 4 5 6 7
    bool f=false;
    // if(v.Hour>23) return false;
    if((begin.Hour>23)|| (end.Hour>23)|| (v.Hour>23) ) return false;
    for(uint8_t h=begin.Hour;h!=end.Hour;h++){
        h=h%24; if(v.Hour==h){f=true; break; } }
    return f;
  }
  bool _inTime(tHHMM begin, tHHMM end, tHHMM v){
    //  ... > Sunrise > Day > Sunset > Night > ... [begin,end)
    if(_inHour(begin.Hour, end.Hour, v.Hour)){
      if(v.Hour==begin.Hour){// 16:41 === 16:40
        if(v.Minute>=begin.Minute) return true;
        else return false;
      }
      if(v.Hour==end.Hour){ // 7:30 === 7:40
        if(v.Minute<end.Minute) return true;
        else return false;
      }
      return true; // [16:40.. [17:--,0:--,6:--] ..7:40)
    }else return false;
  };
  void _setDefaultState(){
    _defaultState=(_inTime(Settings.Day,Settings.Night,thm))? DaylightClock:NightClock ;
  };
  void _fallBack(){
    _setDefaultState();
    clearStates();
    addState(_defaultState,0,)
  }
public:
  void addState(callbackFunction state, word d, callbackFunction fx){
    stateStruct s;
    s.fn=state;
    s.timer=d;
    s.fx = fx;
    _states.push_front(s);
  };
  void nextState(){
    if(_states.size()>0){
      _prevState = _state.fn;
      if(_states.size()>0){
        _state = _states.back();
        _states.pop_back();
        _c = 0;
        D.drawToBuffer(_state.fn); // initializing 
        D.transition(_state.fx); // can be NULL
      }else{
        _state.fn=_defaultState;
        _state.fx=NULL;
        _state.t=0;
      }
    }
  };
  void clearStates(){
    _states.clear();
    _states.reserve(12);
  }
  void init(){
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
  void set(callbackFunction f){
    _refreshFunction=f;
  };
  void update(){ // 1s
    if(_refreshFunction)_refreshFunction();
    if(_state.fn) _state.fn();
    _c++;
    if(_state.t==0){ // 0 = last state or shortest (1s) show
      if(_states.size()>0) nextState();
    }else{
    if(_state.t-- >0) // all that >0 is a countdown
      if(_state.t==0) nextState();
  };

  void cycleOneClick(){
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
  void cycleTwoClick(){
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
  void DaylightClock(){
    if(_c==0) {
      clearStates();
      _defaultState = DaylightClock;
      _onClick = cycleOneClick;
      _onDoubleClick = cycleTwoClick;
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
  void NightClock(){
    if(_c==0){
      clearStates();
      _defaultState=NightClock;
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
};


#define MENU_EDIT   0 // [_234] [1234] _set [+234] _next [2_34]

/*

MENU_EDIT

[Settings]→[time→HH:MM]→[date→DD.MM]→[year→YYYY]→
[HH:MM]→[DD.MM]→[YYYY]

d[4]
editHHMM
      // [12:34]
      // {0-2}{0-9,0-9,0-3}
      // {0-5}{0-9}
dmin, dmax [4]

edit DDMM
      // [01.12] [31.01]
      // {0-3}{0-9,0-9,0-9,0-1}

edit YYYY
      // [2016]
      //    ^^{0-9}{0-9}

edit Digit

*/

struct EditableDigit
{
  uint8_t min, max, val;
};

struct menuStruct
{
  string title[12]; // menu label
  callbackFunction fn; // if select call this function
  uint8_t style; // MENU_EDIT || scroller fx
};


class Menu
{
private:
  // uint8_t _d[4];
  uint8_t _m; // current submenu
  EditableDigit _E[4]; // edit digit
  uint8_t _p; // edit pointer
  std::vector<menuStruct> _menuStack;
  menuStruct _menu;
  callbackFunction _back, _next, _set; // longPress, doubleClick, Click
  int _c; // counter [s]
  const uint8_t _timeout = 12; // [s]
public:
  bool exitMenu;
  callbackFunction _refresh;
  addMenu(string s, callbackFunction fn){
    menuStruct m;
    m.fn=fn;
    strcpy(m.title,s);
    _menuStack.push_front(m);
  };
  void init(){
    _m=0;
    _back=NULL;
    _next=nextMenu;
    _set=set;
    addMenu(Settings, 'Settings:', fx_short);
    // addMenu(Alarms,'Alarms:', fx_short);
    _refresh=titler;
  };
  void nextMenu(){
    if(_menuStack.size()>0){
      _prev = _menuStack.fn;
      _menu = _menuStack.back();
      _menuStack.pop_back();
      _c = 0;
      if(_menu.title) _refresh=titler;
      else
        _refresh=editor;
      if(_m==0)
        D.transition(D.down);
      else
        D.transition(D.right);
      // D.setRefresh
    }else{
        exitMenu=true;
      }
  };
  void back(){
    if(_back)_back();
    else exitMenu=true;
  };
  void set(){
    if(_set)_set();
    else if(_menu.fn);
  };
  void clearMenu(){
    _menuStack.clear();
    _menuStack.reserve(12);
  }

  void titler(){
    if(_c==0) D.scroll(_menu.s,_menu.fn,_menu.fx);
    if(_c>_timeout){
      exitMenu=true;
    }
  };
  void editor(){
    if(_c==0){
      D._hold(0,F.d[_E[0]]);
      D._hold(1,F.d[_E[1]]);
      D._hold(2,F.d[_E[2]]);
      D._hold(3,F.d[_E[3]]);
      D.blink(_p,F.d[_E[_p]]);
      _set=editSet;
      _back=editBack;
      _next=editNext;
    }else{
      D._hold(_p,F.d[_E[_p]]);
      D.blink(_p,F.d[_E[_p]]);
    }
  };
  void editNext(){
    _p++;
  };
  /* Settings */
  void Settings(){
    clearMenu();
    addMenu(editHHMM, NULL, NULL);
    addMenu(editDDMM, NULL, NULL);
    addMenu(editYYYY, NULL, NULL);
  };
  // void editD(uint8_t p, uint8_t* d, uint8_t min, uint8_t max){};
  /* Time */
  void editHHMM(){
    if(_m++==0){
      _p=0;
      _E[0].val=Hour/10;   _E[0].min=0; _E[0].max=2; // 24h
      _E[1].val=Hour%10;   _E[1].min=0; _E[1].max=9; // 24h
      _E[2].val=Minute/10; _E[2].min=0; _E[2].max=9; // 60m
      _E[3].val=Minute%10; _E[3].min=0; _E[3].max=9; // 60m
    }
    editor();
    if(_p>4)
  };
};