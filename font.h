#ifndef __font__
#define __font__

// Fonts

#define __A B01000000
#define __B B00100000
#define __C B00010000
#define __D B00001000
#define __E B00000100
#define __F B00000010
#define __G B00000001
#define __p B10000000
#define FNT_blank B00000000
#define FNT_minus B01000000
#define FNT_dot __p
#define FNT_grad B01100011
#define FNT_week B00110000

struct FNT
{
  const byte blank = B00000000; // 
  const byte minus = B01000000; // -
  const byte week  = B00110000; //
  const byte grad  = B01100011; // °
  const byte celsius=B01000110; // C
  const byte dot   = __p; // .
  const byte trpile= __A|__D|__G;
  const byte d[10]=
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
  const byte AbC[26]=
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
  const byte rusWeekDays[8]=
  { // ПН ВТ СР ЧТ ПТ СБ ВС = БВНПРСТЧ
    B01111101, // Б
    B01111111, // В
    B01110110, // Н
    B00110111, // П
    B01110011, // Р
    B00111001, // С
    B00000111, // Т
    B01100110  // Ч
  };
  struct Sun
  {
    const byte rise[4]={
      B00001000,
      B01001000,
      B01011100,
      B01101011,
    };
    const byte set[4]={
      B01101011,      
      B01011100,
      B01001000,
      B00001000,
    };
  };
  struct Sensor
  {
    struct Temp
    {
      const byte ico = B11111000; // t.
      const byte sign= B01100011; // °
      const byte rise[4]={
        B00000000,
        B00001000,
        B01011100,
        B01100011 
      };
      const byte fall[4]={
        B01100011,
        B01011100,
        B00001000,
        B01100011 
      }
    };
    struct Humidity
    {
      const byte ico = B11110110; // H.
      const byte sign[2] = { B01100011, B01011100 }; // %
      const byte rise[2][4]={
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
      const byte fall[2][4]={
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
      const byte ico = B11110011; // P.
      const byte sign= B01001000; // m
      const byte rise[4] = {
        B00001000,
        B01001000,
        B01001001,
        B01001000,
      };
      const byte fall[4] = {
        B00000001,
        B01000000,
        B00001000,
        B01001000,
      };
      struct CO2
      {
        const byte ico[3]={celsius,d[0],d[2]}; // CO2
        const byte sign[3]={AbC['p'-'a'],AbC['p'-'a'],AbC['m'-'a']}; // ppm
      };
    };
    struct Light
    {
      const byte ico = B10111000; // L.
      // const byte sign= B10111000; // 
    };
  };
  struct level
  {
    const byte v3[3]={
      B00001000,
      B01001000,
      B01001001,
    };
    const byte l3[3]={
      B00001000,
      B01000000,
      B00000010,
    };
    const byte sound[4]={
      B01000000,
      B01000001,
      B01001000,
      B01001001,
    };
  };
};

#endif