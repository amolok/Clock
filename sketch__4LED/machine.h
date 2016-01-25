#ifndef __MACHINE_
#define __MACHINE_
#include <StandardCplusplus.h>
#include <vector>
// #include <functional>
#include "clock.h"

extern "C" {
  typedef void (*callbackFunction)(void);
}

struct stateStruct
{
  callbackFunction fn;
  callbackFunction fx;
  word timer;
};

class Machine
{
private:
  callbackFunction _refreshFunction;
  stateStruct _state;
  std::vector<stateStruct> _states;
  void (Machine::*_defaultState)(void);
  // std::function<void(void)> _defaultState;
  callbackFunction _prevState;
  callbackFunction _onClick;
  callbackFunction _onDoubleClick;
  callbackFunction _onPress;
  word _c; // counter [s] in current state = 45.51 h
  // bool _inHour(tHHMM begin, tHHMM end, tHHMM v);
  // bool _inTime(tHHMM begin, tHHMM end, tHHMM v);
  void _setDefaultState();
  void _fallBack();
public:
  void addState(void (Machine::*state)(void), word d, callbackFunction fx);
  void nextState();
  void clearStates();
  void init();
  void set(callbackFunction f);
  void update();
  void cycleOneClick();
  void cycleTwoClick();
  void DaylightClock();
  void NightClock();
};


#endif
