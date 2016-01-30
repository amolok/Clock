#ifndef __font__
#define __font__
#include <Arduino.h>

// Fonts

#define __A B00000001
#define __B B00000010
#define __C B00000100
#define __D B00001000
#define __E B00010000
#define __F B00100000
#define __G B01000000
#define __p B10000000
#define FNT_blank B00000000
#define FNT_minus B01000000
#define FNT_dot __p
#define FNT_grad B01100011
#define FNT_week B00110000

struct FNT
{
  const uint8_t blank = B00000000; // 
  const uint8_t minus = B01000000; // -
  const uint8_t week  = B00110000; //
  const uint8_t grad  = B01100011; // °
  const uint8_t celsius=B01000110; // C
  const uint8_t dot   = __p; // .
  const uint8_t trpile= __A|__D|__G;

  const uint8_t d[10]=
  {// GFEDCBA
    B00111111, //0
    B00000110, //1
    B01011011, //2
    B01001111, //3
    B01100110, //4
    B01101101, //5
    B01111101, //6
    B00000111, //7
    B01111111, //8
    B01101111  //9
  };

  const uint8_t AbC[26]=
  {
    B01110111, // A
    B01111100, // b
    B00111001, // C
    B01011110, // d
    B01111001, // E
    B01110001, // F
    B00111101, // G
    B01110110, // H
    B00000110, // I
    B00001110, // J
    B01110000, // k
    B00111000, // L
    B01001000, // m
    B01010100, // n
    B01011100, // o
    B01110011, // p
    B01100111, // q
    B01010000, // r
    B01101101, // s
    B01111000, // t
    B00011100, // u
    B00111110, // v
    B01111110, // w
    B01100100, // x
    B01100110, // Y
    B01011011  // z
  };

  const uint8_t rusWeekDays[8]=
  // ПН ВТ СР ЧТ ПТ СБ ВС = БВНПРСТЧ
  { 
    B01111101, // Б
    B01111111, // В
    B01110110, // Н
    B00110111, // П
    B01110011, // Р
    B00111001, // С
    B00000111, // Т
    B01100110  // Ч
  };
  struct sSun
  {
    const uint8_t rise[4]={
      B00001000,
      B01001000,
      B01011100,
      B01101011,
    };
    const uint8_t set[4]={
      B01101011,      
      B01011100,
      B01001000,
      B00001000,
    };
  };
  sSun Sun;
  struct sSensor
  {
    struct sTemp
    {
      const uint8_t ico = B11111000; // t.
      const uint8_t sign= B01100011; // °
      const uint8_t rise[4]={
        B00000000,
        B00001000,
        B01011100,
        B01100011 
      };
      const uint8_t fall[4]={
        B01100011,
        B01011100,
        B00001001,
        B01100011 
      };
    };
    sTemp Temp;
  };
  sSensor Sensor;
    /*
    struct Humidity
    {
      const uint8_t ico = B11110110; // H.
      const uint8_t sign[2] = { B01100011, B01011100 }; // %
      const uint8_t rise[2][4]={
        {
          B00000000,
          B00000000,
          B01100011,
          B01100011,
        },{
          B00000000,
          B01011100,
          B01011100,
          B01011100,
        }
      };
      const uint8_t fall[2][4]={
        {
          B01100011,
          B00000000,
          B00000000,
          B01100011,
        },{
          B01011100,
          B01011100,
          B00000000,
          B01011100,
        }
      };
      struct Pressure
      {
      const uint8_t ico = B11110011; // P.
      const uint8_t sign= B01001000; // m
      const uint8_t rise[4] = {
        B00001000,
        B01001000,
        B01001001,
        B01001000,
      };
      const uint8_t fall[4] = {
        B00000001,
        B01000000,
        B00001000,
        B01001000,
      };
      struct CO2
      {
        const uint8_t ico[3]={celsius,d[0],d[2]}; // CO2
        const uint8_t sign[3]={AbC['p'-'a'],AbC['p'-'a'],AbC['m'-'a']}; // ppm
      };
    };
    struct Light
    {
      const uint8_t ico = B10111000; // L.
      // const uint8_t sign= B10111000; // 
    };
  };
  struct level
  {
    const uint8_t v3[3]={
      B00001000,
      B01001000,
      B01001001,
    };
    const uint8_t l3[3]={
      B00001000,
      B01000000,
      B00000010,
    };
    const uint8_t sound[4]={
      B01000000,
      B01000001,
      B01001000,
      B01001001,
    };
  };
*/
};

extern FNT F;

#endif
