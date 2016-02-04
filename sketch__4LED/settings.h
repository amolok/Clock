#ifndef __settings__
#define __settings__

/* PIN */
#define CLK 2
#define DIO 3
#define BTN 8

/* Settings */

struct tHHMM
{
  uint8_t Hour, Minute;
};
struct tInterleave
{
  tHHMM start, end;
};

struct sSettings
{
  tHHMM Day, Night;
  tInterleave Sleep;
  // tHHMM Sunrise, Sunset;
  // tHHMM Wakeup;
  // tHHMM Alarm;
};

// sSettings Settings;

#endif