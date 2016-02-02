#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;
 
void setup () {
  //Initialize the serial port, wire library and RTC module
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();
  //We check if the RTC module is working, if not we sent Error
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    //If we remove the comment from the following line, we will set up the module time and date with the computer one
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}
 
void loop () {
    DateTime now = RTC.now();
    //We print the day
    Serial.print(now.day(), DEC);
    Serial.print('/');
    //We print the month
    Serial.print(now.month(), DEC);
    Serial.print('/');
    //We print the year
    Serial.print(now.year(), DEC);
    Serial.print(' ');
    //We print the hour
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    //We print the minutes
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    //We print the seconds
    Serial.print(now.second(), DEC);
    Serial.println();
    //We check the time and sent through the serial port every 3s
    delay(3000);
}
