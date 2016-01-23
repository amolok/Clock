/* Sensors */

#define normalize(a,min,max) (byte)((word)((a-min)*100)/(max-min))
#define normTemp(t) (signed int)((float)(t-TempSensorMin)/(TempSensorMax-TempSensorMin)*(TempSensorMaxT-TempSensorMinT)+TempSensorMinT)

struct SensorParameters
{
  byte pin;
  int minLevel, maxLevel, minValue, maxValue;
  int comfortZone[2];
  int alertZone[2];
};

const SensorParameters tempSensor =     {1, 20,358, -40,125, 18,23, 16,27};
// const SensorParameters humiditySensor = {2, 0,100,    0,100, 60,70, 50,80};
const SensorParameters lightSensor =    {0, 2,381,    0,100, 60,80,  0,100};

Sensors Temp     = {tempSensor, 0, 60*15};
Sensors Humidity = {humiditySensor,  50, 60*15};

/*
struct SENque
{
  int lastUpdate, // last update time
  int  timer[Sensors], // timer que
  // char sensor[Sensors], // sensors que
};
*/


struct history
{
  int value;
  int lastValue;
  int MinToday, MaxToday;
  // int dayly[7];
  // int hourly[24];
  // int quarter[4];
  // int everyFive[3];
  // int everyMinute[5];
}; // 2*4 = 8b
  // 7+24+4+3+5+2 = 45*2 = 90b
  // SensorParameters Sensor;
  // int current; // current
  // int frq; // frequency of requests [s]
  // int avarage[12]; // data array for avarage
  // int lastCheck
// 90+(12+1+1)*2+3 = 121b

class Sensors
{
private:
public:
  history Temp, Pressure, Humidity, CO2, Light;
  void update(){
  };
  void init(){
  };
  void showTemp(){
    // [-12°] [-1°C] [ 0°C] [ 1°C] [12°C] [23°C]
    byte p;
    if(Temp.value<0) D._hold(0,F.minus);
    else D._hold(0,F.blank);
    if(Temp.value>-10){
      if(Temp.value<10) {
        D._D(1, abs(Temp.value));
      }else{
        D._DD(0, Temp.value);
      }
      D._hold(2,F.grad); p=2;
      D._hold(3,F.celsius);
    }else{
      D._DD(1, abs(Temp.value));
      D._hold(3,F.grad); p=3;
    }
    if(Temp.value>Temp.lastValue) D._ab(p, F.Sensor.Temp.rise);
      else if(Temp.value<Temp.lastValue) D._ab(p, F.Sensor.Temp.fall);
  };
  void showPressure(){
    // [750m]
    D._DDD(0, Pressure.value);
    D._hold(3, F.Sensor.Pressure.sign);
    if(Pressure.value>Pressure.lastValue) D._ab(3, F.Sensor.Pressure.rise);
    else if(Pressure.value<Pressure.lastValue) D._ab(3, F.Sensor.Pressure.fall);
  };
  void showHumidity(){
    if(Humidity.value<10){
      D._hold(0, F.blank);
      D._D(1, Humidity.value);
    }else{
      if(Humidity.value<100)
        D._DD(0, Humidity.value);
      else
        D._DD(0, 99);
        D.blink2(1,F.dot);
    }
    if(Humidity.value>Humidity.lastValue) {
      D._ab(2, F.Sensor.Humidity.rise[0]);
      D._ab(3, F.Sensor.Humidity.rise[1]);
    }
    else if(Humidity.value<Humidity.lastValue){
      D._ab(2, F.Sensor.Humidity.fall[0]);
      D._ab(3, F.Sensor.Humidity.fall[1]);
    } else {
      D._hold(2, F.Sensor.Humidity.sign[0]);
      D._hold(3, F.Sensor.Humidity.sign[1]);
    }
  };
  void showCO2(){
    if(CO2.value>=1000)
      D._DDDD(CO2.value);
    else D._DDD(1, CO2.value);
  };
};

