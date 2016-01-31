/* Sensors */
#include "sensors.h"
// #include "settings.h"
#include "DHT.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

extern FNT F;
extern Display4LED2 D;
// extern sSettings Settings;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

#define _DEBUG_ false

/*
#define normalize(a,min,max) (byte)((word)((a-min)*100)/(max-min))
#define normTemp(t) (signed int)((float)(t-TempSensorMin)/(TempSensorMax-TempSensorMin)*(TempSensorMaxT-TempSensorMinT)+TempSensorMinT)

struct SensorParameters
{
  int minLevel, maxLevel, minValue, maxValue;
  int comfortZone[2];
  int alertZone[2];
};

// const SensorParameters tempSensor =     {1, 20,358, -40,125, 18,23, 16,27};
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

void displaySensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}



void Sensors::_readDHT(){
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    Serial.println("!!! Sensors: DHT reading failed !");
    return;
  }
  Temp.lastValue=Temp.value;
  Temp.value=t;
  Humidity.lastValue=Humidity.value;
  Humidity.value=h;
};

void Sensors::_readBMP(){
  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure)
  {
    float p = event.pressure * 0.75006375541921;
    // float p = event.pressure / 133.3223684;
    // {Serial.print(event.pressure); Serial.println("hPa");}
    // {Serial.print(p); Serial.println("mmHg");}
    /* Display atmospheric pressue in hPa */
    Pressure.lastValue=Pressure.value;
    Pressure.value=p;
  }
}

void Sensors::update(){
  _readDHT();
  _readBMP();
};
void Sensors::init(){
  dht.begin();
  if(!bmp.begin())
  {
    Serial.print("!!! BMP085 is not detected ... Check your wiring or I2C ADDR !");
    while(1);
  }
  // displaySensorDetails();
};
void Sensors::showTemp(){
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

void Sensors::showPressure(){
// [750m]
  if(_DEBUG_)Serial.print("Pressure: ");
  if(_DEBUG_)Serial.print(floor(Pressure.value+0.5));
  if(_DEBUG_)Serial.println("mmHg");
  D._DDD(0, (floor(Pressure.value+0.5)));
  D._hold(3, F.Sensor.Pressure.sign);
  if(Pressure.value>Pressure.lastValue) D._ab(3, F.Sensor.Pressure.rise);
  else if(Pressure.value<Pressure.lastValue) D._ab(3, F.Sensor.Pressure.fall);
};

void Sensors::showHumidity(){
  if(Humidity.value<10){
    D._hold(0, F.blank);
    D._D(1, Humidity.value);
  }else{
    if(Humidity.value<100)
      D._DD(0, Humidity.value);
    else{
      D._DD(0, 99);
      D.blink2(1,F.dot);
    }
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
/*
void Sensors::showCO2(){
  if(CO2.value>=1000)
    D._DDDD(CO2.value);
  else D._DDD(1, CO2.value);
};
*/