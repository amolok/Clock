#ifndef __SENSORS__
#define __SENSORS__
/* Sensors */
#include "clock.h"
// #include "font.h"
// #include "display.h"
// #include "settings.h"

#define DHTPIN 7     // what digital pin we're connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)


#include "DHT.h"
/*
#define normalize(a,min,max) (byte)((word)((a-min)*100)/(max-min))
#define normTemp(t) (signed int)((float)(t-TempSensorMin)/(TempSensorMax-TempSensorMin)*(TempSensorMaxT-TempSensorMinT)+TempSensorMinT)

struct SensorParameters
{
  int minLevel, maxLevel, minValue, maxValue;
  int comfortZone[2];
  int alertZone[2];
};

const SensorParameters tempSensor =     {1, 20,358, -40,125, 18,23, 16,27};
// const SensorParameters humiditySensor = {2, 0,100,    0,100, 60,70, 50,80};
// const SensorParameters lightSensor =    {0, 2,381,    0,100, 60,80,  0,100};

Sensors Temp     = {tempSensor, 0, 60*15};
Sensors Humidity = {humiditySensor,  50, 60*15};

struct SENque
{
  int lastUpdate, // last update time
  int  timer[Sensors], // timer que
  // char sensor[Sensors], // sensors que
};
*/

struct sHistory
{
  float value;
  float lastValue;
};
/*
  int MinToday, MaxToday;
  int dayly[7];
  int hourly[24];
  int quarter[4];
  int everyFive[3];
  int everyMinute[5];
  7+24+4+3+5+2 = 45*2 = 90b
  SensorParameters Sensor;
  int current; // current
  int frq; // frequency of requests [s]
  int avarage[12]; // data array for avarage
  int lastCheck
*/
// 90+(12+1+1)*2+3 = 121b

class Sensors
{
private:
  void _readDHT();
  void _readBMP();
public:
  sHistory Temp, Humidity, Pressure; // CO2, Light;
  void update();
  void init();
  // [-12°] [-1°C] [ 0°C] [ 1°C] [12°C] [23°C]
  void showTemp();
  // [40%]
  void showHumidity();
  // [750m]
  void showPressure();
/*
  // [1200]
  void showCO2();
*/
};

#endif
