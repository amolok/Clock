#ifndef __CLOCK__
#define __CLOCK__
// #include <Arduino.h>
#include <vector>
#include "font.h"
#include "display.h"
// #include "sensors.h"
// #include "OneButton.h"
#include "settings.h"


extern "C" {
  typedef void (*callbackFunction)(void);
}

void _incSM(uint8_t p, uint8_t x);

class Clockwork
{
private:
  uint8_t _D[4];
  // void _incHH(uint8_t b, uint8_t x);
  // void _SM(uint8_t p, uint8_t x);
public:
  void init(void);
  // void set(callbackFunction f);
  void BlankBlink(void);
  void MMSS(void);
  void HHMM(void);
  void _WD(uint8_t p, uint8_t wd);
  void DDWD(void);
  void DDMM(void);
  void Week(void);
  void YYYY(void);
  void Sunrise(void);
  void Sunset(void);
};

#endif