#ifndef __MACHINE_
#define __MACHINE_
#include <StandardCplusplus.h>
#include <vector>
// #include <functional>
#include "clock.h"

extern "C" {
  typedef void (*callbackFunction)(void);
}


class Machine
{
private:
  struct stateStruct
  {
    void (Machine::*fn)(void);
    void (Machine::*fx)(void);
    word timer;
  };
  void (Machine::*_refreshFunction)(void);
  stateStruct _state;
  std::vector<stateStruct> _states;
  void (Machine::*_defaultState)(void);
  // std::function<void(void)> _defaultState;
  void (Machine::*_prevState)(void);
  void (Machine::*_onClick)(void);
  void (Machine::*_onDoubleClick)(void);
  void (Machine::*_onPress)(void);
  word _c; // counter [s] in current state = 45.51 h
  // bool _inHour(tHHMM begin, tHHMM end, tHHMM v);
  // bool _inTime(tHHMM begin, tHHMM end, tHHMM v);
  void _setDefaultState();
  void _fallBack();
public:
  void addState(void (Machine::*state)(void), word d, void (Machine::*fx)(void));
  void nextState();
  void clearStates();
  void init();
  void set(void (Machine::*f)(void));
  void update();
  void cycleOneClick();
  void cycleTwoClick();
  void DaylightClock();
  void NightClock();
};


#endif
