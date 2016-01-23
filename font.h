#ifndef __font_
#define __font_

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
#define FNT_minus B00000001
#define FNT_dot __p
#define FNT_grad B01100011
#define FNT_week B00000110

struct FNT
{
  const byte blank = B00000000; // 
  const byte minus = B00000001; // -
  const byte week  = B00000110; //
  const byte grad  = B01100011; // °
  const byte celsius=B00110001; // C
  const byte dot   = __p; // .
  const byte trpile= __A|__D|__G;
  const byte d[10]=
  {// ABCDEFG
    B01111110, //0
    B00110000, //1
    B01101101, //2
    B01111001, //3
    B00110011, //4
    B01011011, //5
    B01011111, //6
    B01110000, //7
    B01111111, //8
    B01111011  //9
  };
  const byte AbC[26]=
  {
    B01110111, // A
    B00011111, // b
    B01001110, // C
    B00111101, // d
    B01001111, // E
    B01000111, // F
    B01011110, // G
    B00110111, // H
    B00110000, // I
    B00111000, // J
    B00000111, // k
    B00001110, // L
    B00001001, // m
    B00010101, // n
    B00011101, // o
    B01100111, // p
    B01110011, // q
    B00000101, // r
    B01011011, // s
    B00001111, // t
    B00011100, // u
    B00111110, // v
    B00111111, // w
    B00010011, // x
    B00110011, // Y
    B01101101  // z
  };
  const byte rusWeekDays[8]=
  { // ПН ВТ СР ЧТ ПТ СБ ВС = БВНПРСТЧ
    B01011111, // Б
    B01111111, // В
    B00110111, // Н
    B01110110, // П
    B01100111, // Р
    B01001110, // С
    B01110000, // Т
    B00110011  // Ч
  };
  struct Sun
  {
    const byte rise[4]={
      B00001000,
      B00001001,
      B00011101,
      B01101011,
    };
    const byte set[4]={
      B01101011,      
      B00011101,
      B00001001,
      B00001000,
    };
  };
  struct Sensor
  {
    struct Temp
    {
      const byte ico = B10001111; // t.
      const byte sign= B01100011; // °
      const byte rise[4]={
        B00000000,
        B00001000,
        B00011101,
        B01100011 
      };
      const byte fall[4]={
        B01100011,
        B00011101,
        B00001000,
        B01100011 
      }
    };
    struct Humidity
    {
      const byte ico = B10110111; // H.
      const byte sign[2] = { B01100011, B00011101 }; // %
      const byte rise[2][4]={
        {
          B00000000,
          B00000000,
          B01100011,
          B01100011,
        },{
          B00000000,
          B00011101,
          B00011101,
          B00011101,
        }
      };
      const byte fall[2][4]={
        {
          B01100011,
          B00000000,
          B00000000,
          B01100011,
        },{
          B00011101,
          B00011101,
          B00000000,
          B00011101,
        }
      };
      struct Pressure
      {
      const byte ico = B11100111; // P.
      const byte sign= B00001001; // m
      const byte rise[4] = {
        B00001000,
        B00001001,
        B01001001,
        B00001001,
      };
      const byte fall[4] = {
        B01000000,
        B00000001,
        B00001000,
        B00001001,
      };
      struct CO2
      {
        const byte ico[3]={celsius,d[0],d[2]}; // CO2
        const byte sign[3]={AbC['p'-'a'],AbC['p'-'a'],AbC['m'-'a']}; // ppm
      };
    };
    struct Light
    {
      const byte ico = B10001110; // L.
      // const byte sign= B10001110; // 
    };
  };
  struct level
  {
    const byte v3[3]={
      B00001000,
      B00001001,
      B01001001,
    };
    const byte l3[3]={
      B00001000,
      B00000001,
      B00100000,
    };
    const byte sound[4]={
      B00000001,
      B01000001,
      B00001001,
      B01001001,
    };
  };
};

#endif