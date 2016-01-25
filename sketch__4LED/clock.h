#ifndef __CLOCK__
#define __CLOCK__
extern "C" {
  typedef void (*callbackFunction)(void);
}

class Clockwork
{
private:
  uint8_t _D[4];
  // void _incSM(uint8_t p, uint8_t x);
  // void _incHH(uint8_t b, uint8_t x);
  // void _SM(uint8_t p, uint8_t x);
public:
  void init();
  void set(callbackFunction f);
  void MMSS();
  void HHMM();
  void _WD(uint8_t p, uint8_t wd);
  void DDWD();
  void DDMM();
  void Week();
  void YYYY();
  void Sunrise();
  void Sunset();
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

// settingsStruct Settings;

#endif