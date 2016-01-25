#ifndef __settings__
#define __settings__

/* Settings */

struct tHHMM
{
  uint8_t Hour;
  uint8_t Minute;
};

struct sSettings
{
  tHHMM Day, Night;
  // tHHMM Sunrise, Sunset;
  // tHHMM Wakeup;
  // tHHMM Alarm;
};

sSettings Settings;

#endif